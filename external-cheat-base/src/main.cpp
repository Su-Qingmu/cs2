#include "features/esp.hpp"
#include "core/renderer/sdl_renderer.h"
#include "features/menu.hpp"
#include "features/web_radar/web_radar_service.hpp"
#include "features/local_radar/local_fixed_radar.hpp"
#include "core/game/web_radar_json.hpp"
#include "core/diagnostics.hpp"
#include "core/performance_metrics.hpp"
#include "core/runtime_timing.hpp"
#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <thread>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <Windows.h>
#include "imgui.h"

// #define SHOW_CONSOLE

uint32_t WIDTH;
uint32_t HEIGHT;
uint32_t WINDOW_W;
uint32_t WINDOW_H;
int32_t VIEWPORT_X;
int32_t VIEWPORT_Y;
uint32_t VIEWPORT_W;
uint32_t VIEWPORT_H;

// Render waiting screen with ImGui
void renderWaitingScreen(int dotCount)
{
    const float dpiScale = sdl_renderer::getDpiScale();
    const float margin = std::max(8.0f, 16.0f * dpiScale);
    const float windowWidth = std::min(
        480.0f * dpiScale,
        std::max(1.0f, static_cast<float>(WIDTH) - margin * 2.0f));
    const float windowHeight = std::min(
        270.0f * dpiScale,
        std::max(1.0f, static_cast<float>(HEIGHT) - margin * 2.0f));
    ImGui::SetNextWindowPos(
        ImVec2(
            WIDTH / 2.0f - windowWidth / 2.0f,
            HEIGHT / 2.0f - windowHeight / 2.0f
        ),
        ImGuiCond_Always
    );
    ImGui::SetNextWindowSize(
        ImVec2(windowWidth, windowHeight),
        ImGuiCond_Always
    );

    ImGui::Begin("Aegis // Connection", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    {
        const ImVec2 position = ImGui::GetWindowPos();
        const ImVec2 size = ImGui::GetWindowSize();
        sdl_renderer::setInteractiveRect(
            position.x,
            position.y,
            size.x,
            size.y);
    }

    char dots[4] = {0};
    for (int i = 0; i < dotCount; i++) dots[i] = '.';

    ImGui::TextColored(
        ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
        "AEGIS");
    ImGui::SameLine();
    ImGui::TextColored(
        ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
        "CS2 OVERLAY");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 12.0f * dpiScale));
    ImGui::TextColored(
        ImVec4(0.930f, 0.960f, 1.000f, 1.0f),
        "Waiting for Counter-Strike 2%s",
        dots);
    ImGui::TextColored(
        ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
        "The client and its active monitor will be detected automatically.");

    ImGui::Dummy(ImVec2(0.0f, 14.0f * dpiScale));
    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        ImVec4(0.070f, 0.090f, 0.125f, 1.0f));
    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(0.145f, 0.185f, 0.240f, 1.0f));
    ImGui::BeginChild(
        "##WaitingHint",
        ImVec2(0.0f, 90.0f * dpiScale),
        true);
    ImGui::TextColored(
        ImVec4(0.930f, 0.650f, 0.260f, 1.0f),
        "DISPLAY MODE");
    ImGui::TextWrapped(
        "Use Fullscreen Windowed in CS2. The overlay will then map "
        "the game viewport to the correct monitor and aspect ratio.");
    ImGui::EndChild();
    ImGui::PopStyleColor(2);

    ImGui::Dummy(ImVec2(0.0f, 8.0f * dpiScale));
    ImGui::TextColored(
        ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
        "Checking every 3 seconds  |  Press F9 to exit");

    ImGui::End();
}

namespace
{
    class ScopedHandle
    {
    public:
        explicit ScopedHandle(HANDLE handle = nullptr)
            : handle_(handle)
        {
        }

        ~ScopedHandle()
        {
            if (handle_) {
                CloseHandle(handle_);
            }
        }

        ScopedHandle(const ScopedHandle&) = delete;
        ScopedHandle& operator=(const ScopedHandle&) = delete;

        HANDLE get() const
        {
            return handle_;
        }

    private:
        HANDLE handle_ = nullptr;
    };

    void showFatalError(const wchar_t* message)
    {
        diagnostics::log(message);
        MessageBoxW(
            nullptr,
            message,
            L"CS2 ESP",
            MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    }

    std::string makeViewerToken()
    {
        static constexpr char alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-_";
        static_assert(sizeof(alphabet) - 1 == 64);

        std::array<unsigned char, 32> entropy{};
        const NTSTATUS result = BCryptGenRandom(
            nullptr,
            entropy.data(),
            static_cast<ULONG>(entropy.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (result != 0) {
            throw std::runtime_error(
                "Windows could not generate a Web Radar access token");
        }

        std::string token(32, 'A');
        for (std::size_t index = 0; index < token.size(); ++index) {
            token[index] = alphabet[entropy[index] & 63U];
        }
        return token;
    }

    std::string webRadarDocumentRoot()
    {
        std::array<wchar_t, 32768> executablePath{};
        const DWORD length = GetModuleFileNameW(
            nullptr,
            executablePath.data(),
            static_cast<DWORD>(executablePath.size()));
        if (length == 0 || length >= executablePath.size()) {
            return "web-radar/dist";
        }
        return (
            std::filesystem::path(
                std::wstring_view(executablePath.data(), length))
                .parent_path() /
            L"web-radar" /
            L"dist").string();
    }

    game::web_radar_json::TeamViewPolicy teamViewPolicy(
        const int value) noexcept
    {
        switch (value) {
        case 1:
            return game::web_radar_json::TeamViewPolicy::localTeamOnly;
        case 2:
            return game::web_radar_json::TeamViewPolicy::opponentsOnly;
        default:
            return game::web_radar_json::TeamViewPolicy::all;
        }
    }

    std::shared_ptr<const std::string> serializeSnapshotCached(
        const esp::GameSnapshot& snapshot,
        const game::web_radar_json::SerializationOptions& options)
    {
        if (!snapshot) {
            return {};
        }
        struct CacheEntry
        {
            esp::GameSnapshot snapshot;
            game::web_radar_json::SerializationOptions options;
            std::shared_ptr<const std::string> payload;
        };
        thread_local std::array<CacheEntry, 4> entries;
        thread_local std::size_t nextEntry = 0;

        for (const CacheEntry& entry : entries) {
            if (entry.snapshot == snapshot && entry.options == options &&
                entry.payload) {
                return entry.payload;
            }
        }

        const auto serializationStarted =
            std::chrono::steady_clock::now();
        auto payload = std::make_shared<const std::string>(
            game::web_radar_json::serializeSnapshotV1(*snapshot, options));
        performance_metrics::serializationDuration.record(
            std::chrono::steady_clock::now() - serializationStarted);
        entries[nextEntry] = CacheEntry{snapshot, options, payload};
        nextEntry = (nextEntry + 1) % entries.size();
        return payload;
    }

    class WebRadarRuntime final
    {
    public:
        WebRadarRuntime()
            : documentRoot_(webRadarDocumentRoot())
        {
        }

        ~WebRadarRuntime()
        {
            stop();
        }

        void sync(const menu::RuntimeConfig& config)
        {
            const Settings desired{
                config.webRadarEnabled,
                config.webRadarLanAccess,
                config.webRadarPort
            };
            if (settings_ && *settings_ == desired) {
                publishUiStatus();
                return;
            }

            const bool rotateToken =
                desired.enabled &&
                (!settings_ || !settings_->enabled);
            settings_ = desired;
            detachAndStop();
            lastError_.clear();

            if (!desired.enabled) {
                publishUiStatus(true);
                return;
            }

            try {
                if (rotateToken || token_.empty()) {
                    token_ = makeViewerToken();
                }
                web_radar::WebRadarConfig serviceConfig;
                serviceConfig.bindAddress = desired.lanAccess
                    ? "0.0.0.0"
                    : "127.0.0.1";
                serviceConfig.port = desired.port;
                serviceConfig.documentRoot = documentRoot_;
                serviceConfig.token = token_;

                auto service = std::make_shared<
                    web_radar::WebRadarService>(
                        std::move(serviceConfig));
                if (!service->start()) {
                    lastError_ = service->status().lastError;
                    service->stop();
                    publishUiStatus(true);
                    return;
                }

                std::lock_guard<std::mutex> lock(serviceMutex_);
                service_ = std::move(service);
            } catch (const std::exception& error) {
                lastError_ = error.what();
            }
            publishUiStatus(true);
        }

        void publish(
            const esp::GameSnapshot& snapshot,
            const bool includePlayerNames,
            const bool includeSteamIds,
            const int teamPolicy)
        {
            if (!snapshot) {
                return;
            }

            std::shared_ptr<web_radar::WebRadarService> service;
            {
                std::lock_guard<std::mutex> lock(serviceMutex_);
                service = service_;
            }
            if (!service || !service->isRunning()) {
                return;
            }

            game::web_radar_json::SerializationOptions options;
            options.includePlayerNames = includePlayerNames;
            options.includeSteamIds = includeSteamIds;
            options.teamViewPolicy = teamViewPolicy(teamPolicy);
            service->publish(serializeSnapshotCached(snapshot, options));
        }

        void stop()
        {
            settings_.reset();
            detachAndStop();
            publishUiStatus(true);
        }

        [[nodiscard]] std::size_t viewerCount() const noexcept
        {
            return viewerCount_.load(std::memory_order_relaxed);
        }

    private:
        struct Settings
        {
            bool enabled = false;
            bool lanAccess = false;
            uint16_t port = 22006;

            bool operator==(const Settings&) const = default;
        };

        void detachAndStop()
        {
            std::shared_ptr<web_radar::WebRadarService> service;
            {
                std::lock_guard<std::mutex> lock(serviceMutex_);
                service.swap(service_);
            }
            if (service) {
                service->stop();
            }
        }

        void publishUiStatus(bool force = false)
        {
            const auto now = std::chrono::steady_clock::now();
            if (!force && now < nextUiStatusPublish_) {
                return;
            }
            nextUiStatusPublish_ = now + std::chrono::milliseconds(250);

            menu::WebRadarUiStatus ui;
            ui.bindAddress = settings_ && settings_->lanAccess
                ? "0.0.0.0"
                : "127.0.0.1";
            ui.error = lastError_;

            std::shared_ptr<web_radar::WebRadarService> service;
            {
                std::lock_guard<std::mutex> lock(serviceMutex_);
                service = service_;
            }
            if (service) {
                const web_radar::WebRadarStatus status = service->status();
                ui.running = status.isRunning();
                ui.viewers = status.viewerCount;
                ui.publishedFrames = status.publishedFrames;
                ui.sentFrames = status.sentFrames;
                ui.replacedFrames = status.replacedFrames;
                ui.publishedBytes = status.publishedBytes;
                ui.maximumSendLatencyMilliseconds =
                    status.maximumSendLatencyMilliseconds;
                ui.bindAddress = status.bindAddress;
                if (!status.lastError.empty()) {
                    ui.error = status.lastError;
                }
                if (ui.running) {
                    ui.viewerUrl =
                        "http://127.0.0.1:" +
                        std::to_string(status.port) +
                        "/?token=" + token_;
                }
            }
            viewerCount_.store(ui.viewers, std::memory_order_relaxed);
            menu::setWebRadarStatus(std::move(ui));
        }

        std::mutex serviceMutex_;
        std::shared_ptr<web_radar::WebRadarService> service_;
        std::optional<Settings> settings_;
        std::string token_;
        std::string documentRoot_;
        std::string lastError_;
        std::chrono::steady_clock::time_point nextUiStatusPublish_{};
        std::atomic<std::size_t> viewerCount_{0};
    };
}

int main(int argc, char* argv[])
{
    SetLastError(ERROR_SUCCESS);
    ScopedHandle instanceMutex(CreateMutexW(
        nullptr,
        FALSE,
        L"Local\\CS2ExternalOverlay-76F5E6C2-235A-4AA0"));
    const DWORD mutexError = GetLastError();
    if (!instanceMutex.get()) {
        showFatalError(
            L"Unable to create the single-instance guard.");
        return -1;
    }
    if (mutexError == ERROR_ALREADY_EXISTS) {
        MessageBoxW(
            nullptr,
            L"CS2 ESP is already running.",
            L"CS2 ESP",
            MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
        return 0;
    }

    memory::SetWritesAllowed(true);
    menu::loadPersistentSettings();

#ifndef SHOW_CONSOLE
    FreeConsole();
#endif

    WebRadarRuntime webRadar;
    menu::setStartupReport(diagnostics::inspectInstallation(
        std::filesystem::path(webRadarDocumentRoot())));

    while (sdl_renderer::running)
    {
        sdl_renderer::menuVisible = true;
        if (!sdl_renderer::initWaiting()) {
            showFatalError(
                L"Unable to initialize the overlay. Windows 10/11 with "
                L"per-monitor DPI awareness and a working SDL2.dll are required.");
            memory::Close();
            return -1;
        }
        if (!sdl_renderer::initImGui()) {
            showFatalError(L"Unable to initialize ImGui.");
            local_fixed_radar::reset();
            sdl_renderer::destroy();
            memory::Close();
            return -1;
        }

        int dotCount = 0;
        DWORD lastCheckTime = 0;
        bool gameFound = false;
        while (sdl_renderer::running && !gameFound)
        {
            sdl_renderer::pollEvents();

            const DWORD currentTime = GetTickCount();
            if (currentTime - lastCheckTime >= 3000 ||
                lastCheckTime == 0) {
                lastCheckTime = currentTime;
                dotCount = (dotCount % 3) + 1;
                if (esp::init()) {
                    gameFound = true;
                    break;
                }
            }

            if (!sdl_renderer::beginFrame()) {
                break;
            }
            sdl_renderer::newFrameImGui();
            renderWaitingScreen(dotCount);
            sdl_renderer::renderImGui();
            sdl_renderer::endFrame();
            Sleep(16);
        }

        sdl_renderer::shutdownImGui();
        local_fixed_radar::reset();
        sdl_renderer::destroy();
        if (!sdl_renderer::running) {
            break;
        }
        if (!gameFound ||
            !sdl_renderer::init(
                L"Counter-Strike 2",
                static_cast<DWORD>(esp::pID)) ||
            !sdl_renderer::initImGui()) {
            diagnostics::log(
                L"Game attach failed or raced with shutdown; retrying.");
            sdl_renderer::shutdownImGui();
            local_fixed_radar::reset();
            sdl_renderer::destroy();
            esp::clearRuntimeState();
            memory::Close();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500));
            continue;
        }

        // Visual sampling follows the display refresh rate. Latency-sensitive
        // input features retain 240 Hz, while Radar-only work runs at its actual
        // 20 Hz publication rate instead of consuming a core without visible
        // benefit.
        std::atomic<bool> dataRunning{true};
        performance_metrics::resetSession();
        memory::ResetReadMetrics();
        std::thread dataThread(
            [&dataRunning, &webRadar]() {
            auto nextDataTime = std::chrono::steady_clock::now();
            auto nextRadarTime = nextDataTime;
            int appliedSamplingRate = 0;
            int appliedThreadPriority = 0x7FFFFFFF;
            bool stateClearedWhileInactive = false;
            while (dataRunning.load(std::memory_order_relaxed)) {
                const menu::RuntimeConfig config =
                    menu::getRuntimeConfig();
                const bool gameForeground =
                    sdl_renderer::isGameForeground();
                const int desiredThreadPriority =
                    THREAD_PRIORITY_BELOW_NORMAL;
                if (desiredThreadPriority != appliedThreadPriority &&
                    SetThreadPriority(
                        GetCurrentThread(),
                        desiredThreadPriority)) {
                    appliedThreadPriority = desiredThreadPriority;
                }
                const bool radarSamplingEnabled =
                    config.radarSnapshotEnabled();
                const int radarRate = radarSamplingEnabled
                    ? runtime_timing::radarSamplingRate(
                        runtime_timing::RadarDemand{
                            config.localRadarEnabled,
                            config.webRadarEnabled,
                            webRadar.viewerCount() > 0,
                            gameForeground
                        })
                    : 0;
                performance_metrics::radarRateHz.store(
                    radarRate,
                    std::memory_order_relaxed);
                const bool backgroundRadarSampling =
                    config.webRadarEnabled &&
                    !config.webRadarPauseWhenUnfocused;
                if (!gameForeground && !backgroundRadarSampling) {
                    if (!stateClearedWhileInactive) {
                        esp::clearRuntimeState();
                        stateClearedWhileInactive = true;
                        if (config.webRadarEnabled) {
                            webRadar.publish(
                                esp::getGameSnapshot(),
                                config.webRadarIncludePlayerNames,
                                config.webRadarIncludeSteamIds,
                                config.webRadarTeamViewPolicy);
                        }
                    }
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(25));
                    nextDataTime = std::chrono::steady_clock::now();
                    nextRadarTime = nextDataTime;
                    continue;
                }
                stateClearedWhileInactive = false;

                const runtime_timing::SamplingDemand samplingDemand{
                    gameForeground &&
                        (config.effectiveEspEnabled ||
                         config.grenadeESP ||
                         config.droppedWeaponESP),
                    gameForeground &&
                        (config.antiFlash || config.bombTimer),
                    radarRate > 0 ? radarRate : 20
                };
                const int samplingRate =
                    runtime_timing::dataSamplingRate(
                        sdl_renderer::getTargetRefreshRate(),
                        samplingDemand);
                if (samplingRate != appliedSamplingRate) {
                    appliedSamplingRate = samplingRate;
                    performance_metrics::samplingRateHz.store(
                        samplingRate,
                        std::memory_order_relaxed);
                    nextDataTime = std::chrono::steady_clock::now();
                }
                const auto dataInterval =
                    runtime_timing::intervalForRate(samplingRate);

                menu::RuntimeConfig samplingConfig = config;
                samplingConfig.radarRefreshRateHz = std::max(1, radarRate);
                if (!gameForeground) {
                    samplingConfig.antiFlash = false;
                    samplingConfig.espEnabled = false;
                }
                const auto samplingStarted =
                    std::chrono::steady_clock::now();
                esp::updateEntities(samplingConfig);

                const auto sampleComplete =
                    std::chrono::steady_clock::now();
                performance_metrics::samplingDuration.record(
                    sampleComplete - samplingStarted);
                if (radarSamplingEnabled &&
                    sampleComplete >= nextRadarTime) {
                    const esp::GameSnapshot snapshot =
                        esp::getGameSnapshot();
                    if (config.webRadarEnabled) {
                        webRadar.publish(
                            snapshot,
                            config.webRadarIncludePlayerNames,
                            config.webRadarIncludeSteamIds,
                            config.webRadarTeamViewPolicy);
                    }
                    nextRadarTime =
                        sampleComplete +
                        runtime_timing::intervalForRate(
                            std::max(1, radarRate));
                } else if (!radarSamplingEnabled) {
                    nextRadarTime = sampleComplete;
                }

                nextDataTime += dataInterval;
                const auto now = std::chrono::steady_clock::now();
                if (nextDataTime > now) {
                    std::this_thread::sleep_until(nextDataTime);
                } else {
                    // Never spin continuously when RPM work exceeds its current
                    // budget. A short backoff keeps one slow map/update from
                    // monopolizing a CPU core.
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1));
                    performance_metrics::missedSamplingDeadlines.fetch_add(
                        1,
                        std::memory_order_relaxed);
                    nextDataTime = now + dataInterval;
                }
            }
        });

        auto nextRenderTime = std::chrono::steady_clock::now();
        while (sdl_renderer::running &&
               !sdl_renderer::isGameDisconnected())
        {
            const auto renderStarted = std::chrono::steady_clock::now();
            sdl_renderer::pollEvents();
            sdl_renderer::updateWindowPosition();
            if (!sdl_renderer::isGameVisible()) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(25));
                nextRenderTime = std::chrono::steady_clock::now();
                continue;
            }

            if (!sdl_renderer::beginFrame()) {
                break;
            }
            sdl_renderer::newFrameImGui();

            if (menu::effectiveEspEnabled ||
                menu::grenadeESP ||
                menu::droppedWeaponESP) {
                esp::render();
            }
            const menu::RuntimeConfig renderConfig =
                menu::getRuntimeConfig();
            if (renderConfig.localRadarEnabled) {
                const esp::GameSnapshot snapshot =
                    esp::getGameSnapshot();
                if (snapshot) {
                    local_fixed_radar::render(
                        *snapshot,
                        local_fixed_radar::RenderConfig{
                            renderConfig.localRadarAnchorX,
                            renderConfig.localRadarAnchorY,
                            renderConfig.localRadarSize,
                            renderConfig.localRadarMarkerSize,
                            renderConfig.localRadarShowNames
                        });
                }
            } else {
                local_fixed_radar::reset();
            }
            esp::renderBombTimer();
            menu::render();
            {
                const menu::RuntimeConfig config =
                    menu::getRuntimeConfig();
                webRadar.sync(config);
            }

            sdl_renderer::renderImGui();
            sdl_renderer::endFrame();
            performance_metrics::renderCpuDuration.record(
                std::chrono::steady_clock::now() - renderStarted);

            const int refreshRate = std::max(
                60,
                sdl_renderer::getTargetRefreshRate());
            const auto renderInterval =
                std::chrono::nanoseconds(
                    1000000000LL / refreshRate);
            nextRenderTime += renderInterval;
            const auto now = std::chrono::steady_clock::now();
            if (nextRenderTime > now) {
                std::this_thread::sleep_until(nextRenderTime);
            } else {
                performance_metrics::missedRenderDeadlines.fetch_add(
                    1,
                    std::memory_order_relaxed);
                nextRenderTime = now;
            }
        }

        dataRunning.store(false, std::memory_order_relaxed);
        dataThread.join();
        sdl_renderer::shutdownImGui();
        local_fixed_radar::reset();
        sdl_renderer::destroy();
        esp::clearRuntimeState();
        {
            const menu::RuntimeConfig config =
                menu::getRuntimeConfig();
            if (config.webRadarEnabled) {
                webRadar.publish(
                    esp::getGameSnapshot(),
                    config.webRadarIncludePlayerNames,
                    config.webRadarIncludeSteamIds,
                    config.webRadarTeamViewPolicy);
            }
        }
        memory::Close();

        if (sdl_renderer::running) {
            diagnostics::log(
                L"CS2 disconnected; returning to the waiting screen.");
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500));
        }
    }

    webRadar.stop();
    menu::savePersistentSettings();
    memory::Close();
    return 0;
}
