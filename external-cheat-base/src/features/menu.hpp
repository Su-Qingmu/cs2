#pragma once

#include "imgui.h"
#include "core/renderer/sdl_renderer.h"
#include "core/memory/memory.hpp"
#include "core/diagnostics.hpp"
#include "core/performance_metrics.hpp"
#include "features/web_radar/public_relay_config.hpp"
#include "features/web_radar/snapshot_recorder.hpp"
#include <Windows.h>
#include <shellapi.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace menu
{
    // RuntimeConfig is copied by the 240 Hz sampling loop. Keep the complete
    // connection configuration in one immutable allocation instead of
    // repeatedly allocating URL/room/token strings, and wipe the token before
    // that allocation is released.
    class PublicRelayConnectionSettings final
    {
    public:
        PublicRelayConnectionSettings(
            const std::string_view endpointUrl,
            const std::string_view room,
            const std::string_view token)
            : endpointUrl_(endpointUrl),
              room_(room),
              token_(token)
        {
        }

        ~PublicRelayConnectionSettings()
        {
            if (!token_.empty()) {
                SecureZeroMemory(token_.data(), token_.size());
            }
        }

        PublicRelayConnectionSettings(
            const PublicRelayConnectionSettings&) = delete;
        PublicRelayConnectionSettings& operator=(
            const PublicRelayConnectionSettings&) = delete;
        PublicRelayConnectionSettings(
            PublicRelayConnectionSettings&&) = delete;
        PublicRelayConnectionSettings& operator=(
            PublicRelayConnectionSettings&&) = delete;

        [[nodiscard]] std::string_view endpointUrl() const noexcept
        {
            return endpointUrl_;
        }

        [[nodiscard]] std::string_view room() const noexcept
        {
            return room_;
        }

        [[nodiscard]] std::string_view token() const noexcept
        {
            return token_;
        }

    private:
        std::string endpointUrl_;
        std::string room_;
        std::string token_;
    };

    // ImGui edits these values on the render thread. The worker copies one
    // coherent snapshot under this mutex at the start of each update pass.
    inline std::mutex configMutex;
    inline diagnostics::StartupReport startupReport;

    inline void setStartupReport(diagnostics::StartupReport report)
    {
        startupReport = std::move(report);
    }

    struct RuntimeConfig
    {
        bool espEnabled = true;
        bool espWeapon = true;
        bool espFlashIndicator = false;
        bool antiFlash = false;
        bool espViewAngle = true;
        bool localRadarEnabled = false;
        bool localRadarShowNames = true;
        float localRadarAnchorX = 0.02f;
        float localRadarAnchorY = 0.08f;
        float localRadarSize = 0.32f;
        float localRadarMarkerSize = 12.0f;
        bool webRadarEnabled = false;
        bool webRadarLanAccess = false;
        bool webRadar暂停WhenUnfocused = true;
        bool webRadarIncludePlayerNames = true;
        bool webRadarIncludeSteamIds = false;
        int webRadarTeamViewPolicy = 0;
        uint16_t webRadarPort = 22006;
        bool radarRecordingEnabled = false;
        int radarRefreshRateHz = 20;
        bool publicRelayEnabled = false;
        bool publicRelayIncludePlayerNames = true;
        bool publicRelayIncludeSteamIds = false;
        int publicRelayTeamViewPolicy = 0;
        std::shared_ptr<const PublicRelayConnectionSettings>
            publicRelayConnection;
        bool espWallCheck = true;
        bool espSkeleton = true;
        bool grenadeESP = false;
        bool droppedWeaponESP = false;
        bool bombTimer = true;

        bool headOffsetEnabled = true;
        float headOffsetAmount = 5.0f;
        float headOffsetAngleMin = 45.0f;
        float headOffsetAngleMax = 135.0f;
        bool aimbotEnabled = false;
        float aimbotFOV = 10.0f;
        float aimbotSmoothing = 5.0f;
        int aimbotBone = 0;
        bool aimbotVisibleOnly = true;
        int aimbotKey = VK_SHIFT;
        bool smartAimEnabled = false;
        int smartAim优先级 = 0;
        float mouseSensitivity = 1.0f;

        bool triggerbotEnabled = false;
        int triggerbotDelay = 50;
        int triggerbotKey = 0x46;
        bool inputSuppressed = false;

        [[nodiscard]] bool radarSnapshotEnabled() const noexcept
        {
            return localRadarEnabled || webRadarEnabled ||
                publicRelayEnabled || radarRecordingEnabled;
        }
    };

    // Current tab index
    inline int currentTab = 0;

    // ESP Settings
    inline bool espEnabled = true;
    inline bool espBox = true;
    inline bool espHealth = true;
    inline bool espDistance = true;  // Default ON
    inline bool espWeapon = true;    // 武器 display - Default ON
    inline bool espViewAngle = true; // View angle indicator - Default ON
    inline bool espViewAngleText = false; // Show angle degree text
    inline bool espFlashIndicator = false; // Flashbang eye indicator - Default OFF
    inline bool espWallCheck = true; // CS2 spotted-state indicator
    inline bool espSnaplines = false;

    // 骨骼 ESP
    inline bool espSkeleton = true;
    inline float espSkeletonColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // White

    // Colors
    inline float espBoxColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };          // Red - spotted state
    inline float espWallColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };         // Green - not spotted or unknown
    inline float espDistanceColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    inline float espWeaponColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f };       // Cyan color for weapon
    inline float espFlashNormalColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };  // Red - normal eye state
    inline float espFlashColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };        // Yellow - flashed eye state
    inline float espSnaplinesColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

    // Visual Settings
    inline int snaplines起点 = 0; // 0=底部, 1=中心, 2=顶部

    // 自瞄 Settings
    inline bool aimbotEnabled = false;     // 自瞄 enabled
    inline float aimbotFOV = 10.0f;        // Field of view for aimbot (degrees)
    inline float aimbotSmoothing = 5.0f;   // Smoothing factor (1.0 = instant, higher = smoother)
    inline int aimbotBone = 0;             // 0=Head, 1=Neck, 2=Chest
    inline bool aimbotVisibleOnly = true;  // Only aim at enemies flagged as spotted
    inline int aimbotKey = VK_SHIFT;       // 自瞄 activation key (default: Shift key)
    inline bool aimbotShowFOV = true;      // Show FOV circle on screen
    inline float aimbotFOVColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };  // Color-key overlays use opaque primitives

    // 头部偏移 Settings (for side-facing enemies)
    inline bool headOffsetEnabled = true;      // Enable head offset compensation
    inline float headOffsetAmount = 5.0f;      // Offset amount in game units (0-15)
    inline float headOffsetAngleMin = 45.0f;   // Minimum angle for offset (degrees)
    inline float headOffsetAngleMax = 135.0f;  // Maximum angle for offset (degrees)

    // 智能瞄准 Settings (auto-lock spotted enemies by priority)
    inline bool smartAimEnabled = false;      // Smart aim mode (ignores FOV, auto-selects best target)
    inline int smartAim优先级 = 0;          // 0=距离 first, 1=Health first

    // 自动扳机 Settings
    inline bool triggerbotEnabled = false; // 自动扳机 enabled
    inline int triggerbotDelay = 50;       // 开火前延迟(毫秒)
    inline int triggerbotKey = 0x46;       // 自动扳机 activation key (default: F key, 0x46 = 'F')

    // 输入 Settings
    inline float mouseSensitivity = 1.0f;  // In-game mouse sensitivity (for aim/trigger mouse conversion)

    // Viewport mapping: 0=auto black-bar detection, 1=full client,
    // 2=force 4:3 black bars, 3=force 16:10 black bars.
    inline int viewportMode = 0;

    // The local overlay and browser use the same fixed north-up map catalogue
    // and full GameSnapshot. Position values are normalized to the available
    // viewport so resolution and black-bar changes do not move the panel out
    // of bounds.
    inline bool localRadarEnabled = false;
    inline bool localRadarShowNames = true;
    inline float localRadarAnchorX = 0.02f;
    inline float localRadarAnchorY = 0.08f;
    inline float localRadarSize = 0.32f;
    inline float localRadarMarkerSize = 12.0f;

    // Fixed-map browser Radar settings. The HTTP service is local-only unless
    // the user explicitly enables LAN access; stream URLs always carry a token.
    inline bool webRadarEnabled = false;
    inline bool webRadarLanAccess = false;
    inline bool webRadar暂停WhenUnfocused = true;
    inline bool webRadarIncludePlayerNames = true;
    inline bool webRadarIncludeSteamIds = false;
    inline int webRadarTeamViewPolicy = 0;
    inline int webRadarPort = 22006;
    inline bool radarRecordingEnabled = false;

    // Public Relay credentials intentionally live only in process memory and
    // are never included in any settings persistence path. The producer token
    // is rendered with ImGui's password mode and is never copied to status.
    inline bool publicRelayEnabled = false;
    inline bool publicRelayIncludePlayerNames = true;
    inline bool publicRelayIncludeSteamIds = false;
    inline int publicRelayTeamViewPolicy = 0;
    inline std::array<char, 512> publicRelayUrl{};
    inline std::array<char, 65> publicRelayRoom{};
    inline std::array<char, 513> publicRelayToken{};
    inline std::shared_ptr<const PublicRelayConnectionSettings>
        publicRelayConnectionSnapshot;

    struct WebRadarUiStatus
    {
        bool running = false;
        size_t viewers = 0;
        std::uint64_t publishedFrames = 0;
        std::uint64_t sentFrames = 0;
        std::uint64_t replacedFrames = 0;
        std::uint64_t publishedBytes = 0;
        double maximumSendLatencyMilliseconds = 0.0;
        std::string viewerUrl;
        std::string bindAddress = "127.0.0.1";
        std::string error;
    };

    inline std::mutex webRadarStatusMutex;
    inline WebRadarUiStatus webRadarStatus;

    inline void setWebRadarStatus(WebRadarUiStatus status)
    {
        std::lock_guard<std::mutex> lock(webRadarStatusMutex);
        webRadarStatus = std::move(status);
    }

    inline WebRadarUiStatus getWebRadarStatus()
    {
        std::lock_guard<std::mutex> lock(webRadarStatusMutex);
        return webRadarStatus;
    }

    inline std::mutex recorderStatusMutex;
    inline web_radar::SnapshotRecorderStatus recorderStatus;

    inline void setRecorderStatus(
        web_radar::SnapshotRecorderStatus status)
    {
        std::lock_guard<std::mutex> lock(recorderStatusMutex);
        recorderStatus = std::move(status);
    }

    inline web_radar::SnapshotRecorderStatus getRecorderStatus()
    {
        std::lock_guard<std::mutex> lock(recorderStatusMutex);
        return recorderStatus;
    }

    struct PublicRelayUiStatus
    {
        web_radar::PublicRelayState state =
            web_radar::PublicRelayState::disabled;
        std::uint64_t framesSent = 0;
        std::uint64_t replacedFrames = 0;
        std::uint64_t droppedFrames = 0;
        std::uint64_t reconnects = 0;
        std::string error;
    };

    inline std::mutex publicRelayStatusMutex;
    inline PublicRelayUiStatus publicRelayStatus;

    inline void setPublicRelayStatus(PublicRelayUiStatus status)
    {
        std::lock_guard<std::mutex> lock(publicRelayStatusMutex);
        publicRelayStatus = std::move(status);
    }

    inline PublicRelayUiStatus getPublicRelayStatus()
    {
        std::lock_guard<std::mutex> lock(publicRelayStatusMutex);
        return publicRelayStatus;
    }

    // Misc Settings
    inline bool antiFlash = false;          // Memory writes are opt-in
    inline bool bombTimer = true;            // Show bomb timer on screen
    inline bool grenadeESP = false;          // Show grenade positions
    inline bool droppedWeaponESP = false;    // Show dropped weapon positions

    // 切换菜单 Key
    inline int menuToggleKey = VK_F4;        // Menu toggle key (default: F4)
    inline int exitKey = VK_F9;              // Exit key (default: F9)

    // Hotkey binding state
    inline bool isBindingKey = false;
    inline int* bindingKeyTarget = nullptr;
    inline const char* bindingKeyName = nullptr;
    inline bool bindingWaitingForRelease = false;
    inline std::string bindingError;
    inline bool suppressHotkeysUntilRelease = false;

    inline RuntimeConfig buildRuntimeConfig()
    {
        RuntimeConfig config{};
        config.espEnabled = espEnabled;
        config.espWeapon = espWeapon;
        config.espFlashIndicator = espFlashIndicator;
        config.antiFlash = antiFlash;
        config.espViewAngle = espViewAngle;
        config.localRadarEnabled = localRadarEnabled;
        config.localRadarShowNames = localRadarShowNames;
        config.localRadarAnchorX = std::clamp(
            localRadarAnchorX,
            0.0f,
            1.0f);
        config.localRadarAnchorY = std::clamp(
            localRadarAnchorY,
            0.0f,
            1.0f);
        config.localRadarSize = std::clamp(
            localRadarSize,
            0.18f,
            0.65f);
        config.localRadarMarkerSize = std::clamp(
            localRadarMarkerSize,
            6.0f,
            24.0f);
        config.webRadarEnabled = webRadarEnabled;
        config.webRadarLanAccess = webRadarLanAccess;
        config.webRadar暂停WhenUnfocused =
            webRadar暂停WhenUnfocused;
        config.webRadarIncludePlayerNames =
            webRadarIncludePlayerNames;
        config.webRadarIncludeSteamIds =
            webRadarIncludeSteamIds;
        config.webRadarTeamViewPolicy = std::clamp(
            webRadarTeamViewPolicy,
            0,
            2);
        config.webRadarPort = static_cast<uint16_t>(
            std::clamp(webRadarPort, 1024, 65535));
        config.radarRecordingEnabled = radarRecordingEnabled;
        config.publicRelayEnabled = publicRelayEnabled;
        config.publicRelayIncludePlayerNames =
            publicRelayIncludePlayerNames;
        config.publicRelayIncludeSteamIds =
            publicRelayIncludeSteamIds;
        config.publicRelayTeamViewPolicy = std::clamp(
            publicRelayTeamViewPolicy,
            0,
            2);
        config.publicRelayConnection = publicRelayConnectionSnapshot;
        config.espWallCheck = espWallCheck;
        config.espSkeleton = espSkeleton;
        config.grenadeESP = grenadeESP;
        config.droppedWeaponESP = droppedWeaponESP;
        config.bombTimer = bombTimer;

        config.headOffsetEnabled = headOffsetEnabled;
        config.headOffsetAmount = headOffsetAmount;
        config.headOffsetAngleMin = headOffsetAngleMin;
        config.headOffsetAngleMax = headOffsetAngleMax;
        config.aimbotEnabled = aimbotEnabled;
        config.aimbotFOV = aimbotFOV;
        config.aimbotSmoothing = aimbotSmoothing;
        config.aimbotBone = aimbotBone;
        config.aimbotVisibleOnly = aimbotVisibleOnly;
        config.aimbotKey = aimbotKey;
        config.smartAimEnabled = smartAimEnabled;
        config.smartAim优先级 = smartAim优先级;
        config.mouseSensitivity = mouseSensitivity;

        config.triggerbotEnabled = triggerbotEnabled;
        config.triggerbotDelay = triggerbotDelay;
        config.triggerbotKey = triggerbotKey;
        config.inputSuppressed =
            isBindingKey || suppressHotkeysUntilRelease;
        return config;
    }

    inline RuntimeConfig runtimeConfigSnapshot = buildRuntimeConfig();

    inline RuntimeConfig getRuntimeConfig()
    {
        std::lock_guard<std::mutex> lock(configMutex);
        return runtimeConfigSnapshot;
    }

    inline void publishRuntimeConfig()
    {
        const std::string_view currentUrl(publicRelayUrl.data());
        const std::string_view currentRoom(publicRelayRoom.data());
        const std::string_view currentToken(publicRelayToken.data());
        if (currentUrl.empty() && currentRoom.empty() && currentToken.empty()) {
            publicRelayConnectionSnapshot.reset();
        } else if (!publicRelayConnectionSnapshot ||
                   publicRelayConnectionSnapshot->endpointUrl() != currentUrl ||
                   publicRelayConnectionSnapshot->room() != currentRoom ||
                   publicRelayConnectionSnapshot->token() != currentToken) {
            publicRelayConnectionSnapshot =
                std::make_shared<const PublicRelayConnectionSettings>(
                    currentUrl,
                    currentRoom,
                    currentToken);
        }
        const RuntimeConfig updated = buildRuntimeConfig();
        std::lock_guard<std::mutex> lock(configMutex);
        runtimeConfigSnapshot = updated;
    }

    inline std::filesystem::path persistentSettingsPath()
    {
        std::array<wchar_t, 32768> localAppData{};
        const DWORD length = GetEnvironmentVariableW(
            L"LOCALAPPDATA",
            localAppData.data(),
            static_cast<DWORD>(localAppData.size()));
        std::filesystem::path directory =
            length > 0 && length < localAppData.size()
                ? std::filesystem::path(
                    std::wstring_view(localAppData.data(), length)) /
                    L"AegisCS2"
                : std::filesystem::temp_directory_path() / L"AegisCS2";
        std::error_code error;
        std::filesystem::create_directories(directory, error);
        return directory / L"settings-v1.ini";
    }

    inline int readPersistentInt(
        const wchar_t* key,
        const int fallback,
        const std::filesystem::path& path)
    {
        return static_cast<int>(GetPrivateProfileIntW(
            L"settings",
            key,
            fallback,
            path.c_str()));
    }

    inline float readPersistentFloat(
        const wchar_t* key,
        const float fallback,
        const std::filesystem::path& path)
    {
        std::array<wchar_t, 64> fallbackText{};
        std::array<wchar_t, 64> value{};
        swprintf_s(fallbackText.data(), fallbackText.size(), L"%.6f", fallback);
        GetPrivateProfileStringW(
            L"settings",
            key,
            fallbackText.data(),
            value.data(),
            static_cast<DWORD>(value.size()),
            path.c_str());
        wchar_t* end = nullptr;
        const float parsed = std::wcstof(value.data(), &end);
        return end != value.data() && std::isfinite(parsed)
            ? parsed
            : fallback;
    }

    inline void loadPersistentSettings()
    {
        const std::filesystem::path path = persistentSettingsPath();
        if (readPersistentInt(L"schema", 0, path) != 1) {
            publishRuntimeConfig();
            return;
        }

        viewportMode = std::clamp(
            readPersistentInt(L"viewport_mode", viewportMode, path),
            0,
            3);
        localRadarShowNames = readPersistentInt(
            L"local_radar_names",
            localRadarShowNames ? 1 : 0,
            path) != 0;
        localRadarAnchorX = std::clamp(
            readPersistentFloat(
                L"local_radar_anchor_x",
                localRadarAnchorX,
                path),
            0.0f,
            1.0f);
        localRadarAnchorY = std::clamp(
            readPersistentFloat(
                L"local_radar_anchor_y",
                localRadarAnchorY,
                path),
            0.0f,
            1.0f);
        localRadarSize = std::clamp(
            readPersistentFloat(
                L"local_radar_size",
                localRadarSize,
                path),
            0.18f,
            0.65f);
        localRadarMarkerSize = std::clamp(
            readPersistentFloat(
                L"local_radar_marker_size",
                localRadarMarkerSize,
                path),
            6.0f,
            24.0f);
        webRadarPort = std::clamp(
            readPersistentInt(L"web_radar_port", webRadarPort, path),
            1024,
            65535);
        webRadar暂停WhenUnfocused = readPersistentInt(
            L"pause_shared_radar_unfocused",
            webRadar暂停WhenUnfocused ? 1 : 0,
            path) != 0;
        webRadarIncludePlayerNames = readPersistentInt(
            L"web_radar_player_names",
            webRadarIncludePlayerNames ? 1 : 0,
            path) != 0;
        webRadarTeamViewPolicy = std::clamp(
            readPersistentInt(
                L"web_radar_team_policy",
                webRadarTeamViewPolicy,
                path),
            0,
            2);
        publishRuntimeConfig();
    }

    inline void writePersistentValue(
        const wchar_t* key,
        const std::wstring_view value,
        const std::filesystem::path& path)
    {
        const std::wstring owned(value);
        WritePrivateProfileStringW(
            L"settings",
            key,
            owned.c_str(),
            path.c_str());
    }

    inline void savePersistentSettings()
    {
        const std::filesystem::path path = persistentSettingsPath();
        writePersistentValue(L"schema", L"1", path);
        writePersistentValue(
            L"viewport_mode",
            std::to_wstring(std::clamp(viewportMode, 0, 3)),
            path);
        writePersistentValue(
            L"local_radar_names",
            localRadarShowNames ? L"1" : L"0",
            path);

        const auto writeFloat = [&path](
            const wchar_t* key,
            const float value) {
            std::array<wchar_t, 64> text{};
            swprintf_s(text.data(), text.size(), L"%.6f", value);
            writePersistentValue(key, text.data(), path);
        };
        writeFloat(L"local_radar_anchor_x", localRadarAnchorX);
        writeFloat(L"local_radar_anchor_y", localRadarAnchorY);
        writeFloat(L"local_radar_size", localRadarSize);
        writeFloat(L"local_radar_marker_size", localRadarMarkerSize);
        writePersistentValue(
            L"web_radar_port",
            std::to_wstring(std::clamp(webRadarPort, 1024, 65535)),
            path);
        writePersistentValue(
            L"pause_shared_radar_unfocused",
            webRadar暂停WhenUnfocused ? L"1" : L"0",
            path);
        writePersistentValue(
            L"web_radar_player_names",
            webRadarIncludePlayerNames ? L"1" : L"0",
            path);
        writePersistentValue(
            L"web_radar_team_policy",
            std::to_wstring(std::clamp(webRadarTeamViewPolicy, 0, 2)),
            path);
    }

    // Convert virtual key code to key name
    inline const char* GetKeyName(int vkCode)
    {
        static char keyName[32];

        switch (vkCode)
        {
        // Special keys
        case VK_LBUTTON: return "鼠标1";
        case VK_RBUTTON: return "鼠标2";
        case VK_MBUTTON: return "鼠标3";
        case VK_XBUTTON1: return "鼠标4";
        case VK_XBUTTON2: return "鼠标5";
        case VK_BACK: return "退格";
        case VK_TAB: return "Tab";
        case VK_RETURN: return "回车";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_PAUSE: return "暂停";
        case VK_CAPITAL: return "大写锁定";
        case VK_ESCAPE: return "Esc";
        case VK_SPACE: return "空格";
        case VK_PRIOR: return "上页";
        case VK_NEXT: return "下页";
        case VK_END: return "End";
        case VK_HOME: return "Home";
        case VK_LEFT: return "左";
        case VK_UP: return "上";
        case VK_RIGHT: return "右";
        case VK_DOWN: return "下";
        case VK_INSERT: return "插入";
        case VK_DELETE: return "删除";
        case VK_LSHIFT: return "左Shift";
        case VK_RSHIFT: return "右Shift";
        case VK_LCONTROL: return "左Ctrl";
        case VK_RCONTROL: return "右Ctrl";
        case VK_LMENU: return "左Alt";
        case VK_RMENU: return "右Alt";

        // Function keys
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";

        // Numpad
        case VK_NUMPAD0: return "数字0";
        case VK_NUMPAD1: return "数字1";
        case VK_NUMPAD2: return "数字2";
        case VK_NUMPAD3: return "数字3";
        case VK_NUMPAD4: return "数字4";
        case VK_NUMPAD5: return "数字5";
        case VK_NUMPAD6: return "数字6";
        case VK_NUMPAD7: return "数字7";
        case VK_NUMPAD8: return "数字8";
        case VK_NUMPAD9: return "数字9";
        case VK_MULTIPLY: return "Num*";
        case VK_ADD: return "Num+";
        case VK_SUBTRACT: return "数字-";
        case VK_DECIMAL: return "数字.";
        case VK_DIVIDE: return "数字/";

        // Letters A-Z (0x41 - 0x5A)
        case 0x41: return "A";
        case 0x42: return "B";
        case 0x43: return "C";
        case 0x44: return "D";
        case 0x45: return "E";
        case 0x46: return "F";
        case 0x47: return "G";
        case 0x48: return "H";
        case 0x49: return "I";
        case 0x4A: return "J";
        case 0x4B: return "K";
        case 0x4C: return "L";
        case 0x4D: return "M";
        case 0x4E: return "N";
        case 0x4F: return "O";
        case 0x50: return "P";
        case 0x51: return "Q";
        case 0x52: return "R";
        case 0x53: return "S";
        case 0x54: return "T";
        case 0x55: return "U";
        case 0x56: return "V";
        case 0x57: return "W";
        case 0x58: return "X";
        case 0x59: return "Y";
        case 0x5A: return "Z";

        // Numbers 0-9 (0x30 - 0x39)
        case 0x30: return "0";
        case 0x31: return "1";
        case 0x32: return "2";
        case 0x33: return "3";
        case 0x34: return "4";
        case 0x35: return "5";
        case 0x36: return "6";
        case 0x37: return "7";
        case 0x38: return "8";
        case 0x39: return "9";

        default:
            sprintf_s(keyName, "Key(0x%02X)", vkCode);
            return keyName;
        }
    }

    // Check for key press during binding
    inline int GetPressedKey()
    {
        // Check mouse buttons
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) return VK_LBUTTON;
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) return VK_RBUTTON;
        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) return VK_MBUTTON;
        if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) return VK_XBUTTON1;
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) return VK_XBUTTON2;

        // Check all keyboard keys
        for (int i = 0x08; i <= 0xFE; i++)
        {
            // Skip some keys that shouldn't be used
            if (i == VK_ESCAPE) continue;  // Esc cancels binding

            if (GetAsyncKeyState(i) & 0x8000)
                return i;
        }

        return 0;
    }

    inline bool AnyBindableKeyDown()
    {
        for (int key = 0x01; key <= 0xFE; ++key) {
            if (GetAsyncKeyState(key) & 0x8000) {
                return true;
            }
        }
        return false;
    }

    inline bool ConfiguredHotkeysReleased()
    {
        const int keys[] = {
            menuToggleKey,
            exitKey,
            aimbotKey,
            triggerbotKey,
            VK_LMENU
        };
        for (int key : keys) {
            if (key > 0 && key <= 0xFF &&
                (GetAsyncKeyState(key) & 0x8000)) {
                return false;
            }
        }
        return true;
    }

    inline const char* FindHotkeyConflict(
        const int* target,
        int candidate)
    {
        struct Binding
        {
            const char* name;
            const int* key;
        };
        const Binding bindings[] = {
            { "切换菜单", &menuToggleKey },
            { "退出程序", &exitKey },
            { "自瞄键", &aimbotKey },
            { "自动扳机键", &triggerbotKey }
        };
        for (const Binding& binding : bindings) {
            if (binding.key != target && *binding.key == candidate) {
                return binding.name;
            }
        }
        return nullptr;
    }

    // Render hotkey button
    inline void RenderHotkeyButton(const char* label, int* keyCode, const char* tooltip = nullptr)
    {
        const float dpiScale = sdl_renderer::getDpiScale();
        ImGui::Text("%s:", label);
        ImGui::SameLine(150.0f * dpiScale);

        char buttonLabel[64];
        if (isBindingKey && bindingKeyTarget == keyCode)
        {
            sprintf_s(buttonLabel, "[Press Key...]##%s", label);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        }
        else
        {
            sprintf_s(buttonLabel, "%s##%s", GetKeyName(*keyCode), label);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
        }

        if (ImGui::Button(buttonLabel, ImVec2(100.0f * dpiScale, 0.0f)))
        {
            isBindingKey = true;
            bindingKeyTarget = keyCode;
            bindingKeyName = label;
            bindingWaitingForRelease = true;
            bindingError.clear();
            suppressHotkeysUntilRelease = true;
        }

        ImGui::PopStyleColor();

        if (tooltip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip);
    }

    // Update key binding (call every frame)
    inline void UpdateKeyBinding()
    {
        if (!isBindingKey || bindingKeyTarget == nullptr)
            return;

        // Check for escape to cancel
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            isBindingKey = false;
            bindingKeyTarget = nullptr;
            bindingKeyName = nullptr;
            bindingWaitingForRelease = false;
            suppressHotkeysUntilRelease = true;
            return;
        }

        // Do not capture the mouse click that opened the binding button, or
        // any modifier that was already held at that moment.
        if (bindingWaitingForRelease) {
            if (!AnyBindableKeyDown()) {
                bindingWaitingForRelease = false;
            }
            return;
        }

        int pressedKey = GetPressedKey();
        if (pressedKey != 0)
        {
            if (const char* conflict =
                    FindHotkeyConflict(bindingKeyTarget, pressedKey)) {
                bindingError =
                    std::string("已分配给 ") + conflict;
                bindingWaitingForRelease = true;
                return;
            }

            *bindingKeyTarget = pressedKey;
            isBindingKey = false;
            bindingKeyTarget = nullptr;
            bindingKeyName = nullptr;
            bindingWaitingForRelease = false;
            suppressHotkeysUntilRelease = true;
        }
    }

    inline void BeginCard(
        const char* id,
        const char* title,
        const char* subtitle,
        float height,
        float width = 0.0f)
    {
        const float dpiScale = sdl_renderer::getDpiScale();
        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ImVec4(0.070f, 0.090f, 0.125f, 1.0f));
        ImGui::PushStyleColor(
            ImGuiCol_Border,
            ImVec4(0.145f, 0.185f, 0.240f, 1.0f));
        ImGui::BeginChild(
            id,
            ImVec2(
                width > 0.0f ? width * dpiScale : 0.0f,
                height * dpiScale),
            true);
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "%s",
            title);
        if (subtitle && subtitle[0] != '\0') {
            ImGui::TextColored(
                ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
                "%s",
                subtitle);
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    inline void EndCard()
    {
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }

    // Render 自瞄 tab content
    inline void Render自瞄Tab()
    {
        ImGui::Checkbox("启用自瞄", &aimbotEnabled);

        if (aimbotEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("智能瞄准(自动锁定)", &smartAimEnabled);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Ignore FOV, auto-aim at best spotted target\n优先级: Spotted > 距离/Health");

            if (smartAimEnabled) {
                ImGui::Indent();
                const char* priorityItems[] = { "距离优先", "血量优先" };
                ImGui::Combo("优先级", &smartAim优先级, priorityItems, IM_ARRAYSIZE(priorityItems));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("距离: Aim at closest enemy\nHealth: Aim at lowest HP enemy");
                ImGui::Unindent();
            }

            if (!smartAimEnabled) {
                ImGui::SliderFloat("FOV", &aimbotFOV, 1.0f, 30.0f, "%.1f deg");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("视野范围 - 仅瞄准此角度内的敌人");
            }

            ImGui::SliderFloat("瞄准平滑度", &aimbotSmoothing, 1.0f, 20.0f, "%.1f");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("1.0 = instant lock, higher = smoother/slower");

            const char* boneItems[] = { "头部", "颈部", "胸部" };
            ImGui::Combo("目标骨骼", &aimbotBone, boneItems, IM_ARRAYSIZE(boneItems));

            if (!smartAimEnabled) {
                ImGui::Checkbox("仅可见的", &aimbotVisibleOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Uses CS2's spotted flag; this is not a geometric ray-cast");
            }

            ImGui::Checkbox("显示视野圈", &aimbotShowFOV);
            if (aimbotShowFOV) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##FOVColor", aimbotFOVColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "头部偏移");

            ImGui::Checkbox("启用(侧身)", &headOffsetEnabled);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("敌人侧身时补偿头部位置");

            if (headOffsetEnabled) {
                ImGui::Indent();
                ImGui::SliderFloat("偏移量", &headOffsetAmount, 0.0f, 15.0f, "%.1f units");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("头部位置偏移量(推荐 5-8)");

                ImGui::SliderFloat("最小角度", &headOffsetAngleMin, 0.0f, 90.0f, "%.0f deg");
                ImGui::SliderFloat("最大角度", &headOffsetAngleMax, 90.0f, 180.0f, "%.0f deg");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Angle range for offset (45-135 = side-facing)");

                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "0=facing you, 90=side, 180=back");
                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "输入");
            ImGui::SliderFloat("鼠标灵敏度", &mouseSensitivity, 0.1f, 10.0f, "%.2f");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("匹配游戏内鼠标灵敏度");

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Hold %s to aim", GetKeyName(aimbotKey));
        }
    }

    // Render 自动扳机 tab content
    inline void Render自动扳机Tab()
    {
        ImGui::Checkbox("启用自动扳机", &triggerbotEnabled);

        if (triggerbotEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderInt("延迟(毫秒)", &triggerbotDelay, 0, 500, "%d ms");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("开火前延迟(毫秒)");

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Hold %s to activate", GetKeyName(triggerbotKey));
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "仅当准星对准存活敌人时开火");
        }
    }

    // Render ESP tab content
    inline void RenderESPTab()
    {
        ImGui::Checkbox("启用透视", &espEnabled);
        ImGui::TextDisabled("左 Alt 切换透视");

        if (espEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 方框透视
            ImGui::Checkbox("方框透视", &espBox);
            if (espBox) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##BoxColor", espBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // 血条
            ImGui::Checkbox("血条", &espHealth);

            // 武器 Display
            ImGui::Checkbox("武器", &espWeapon);
            if (espWeapon) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##WeaponColor", espWeaponColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // View Direction
            ImGui::Checkbox("视野方向(方框颜色)", &espViewAngle);
            if (espViewAngle) {
                ImGui::Indent();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "面向你:红");
                ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "部分面向:橙");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "侧面:黄");
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "背面:绿");
                ImGui::Checkbox("显示角度数值", &espViewAngleText);
                ImGui::Unindent();
            }

            // CS2 spotted-state check. This is intentionally not described as
            // a ray-cast: it is a conservative game-state signal.
            ImGui::Checkbox("可见性检查(三角)", &espWallCheck);
            if (espWallCheck) {
                ImGui::Indent();
                ImGui::Text("可见颜色:");
                ImGui::SameLine();
                ImGui::ColorEdit4("##BoxColor2", espBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Text("不可见 / 未知:");
                ImGui::SameLine();
                ImGui::ColorEdit4("##WallColor", espWallColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::TextColored(
                    ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "使用 CS2 可见性状态;不会假设远处目标可见");
                ImGui::Unindent();
            }

            // 距离
            ImGui::Checkbox("距离", &espDistance);
            if (espDistance) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##DistanceColor", espDistanceColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // 闪光弹眼部指示
            ImGui::Checkbox("闪光弹眼部指示", &espFlashIndicator);
            if (espFlashIndicator) {
                ImGui::Indent();
                ImGui::Text("正常眼睛:");
                ImGui::SameLine();
                ImGui::ColorEdit4("##FlashNormalColor", espFlashNormalColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Text("致盲眼睛:");
                ImGui::SameLine();
                ImGui::ColorEdit4("##FlashColor", espFlashColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Unindent();
            }

            // 射线
            ImGui::Checkbox("射线", &espSnaplines);
            if (espSnaplines) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##SnaplinesColor", espSnaplinesColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Indent();
                const char* origins[] = { "底部", "中心", "顶部" };
                ImGui::Combo("起点", &snaplines起点, origins, IM_ARRAYSIZE(origins));
                ImGui::Unindent();
            }

            // 骨骼
            ImGui::Checkbox("骨骼", &espSkeleton);
            if (espSkeleton) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##SkeletonColor", espSkeletonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }
        }
    }

    // Local and browser modes consume the same complete GameSnapshot and map
    // catalogue. Neither mode rotates or distance-crops the north-up map.
    inline void RenderRadarTab()
    {
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "本地固定地图覆盖");
        ImGui::TextWrapped(
            "在游戏覆盖层内绘制完整正北向上的地图。它 "
            "使用与 Web Radar 相同的图片与校准;仅玩家 "
            "方向标记会旋转。");
        ImGui::Spacing();

        ImGui::Checkbox("启用本地地图覆盖", &localRadarEnabled);
        if (localRadarEnabled) {
            ImGui::Checkbox("显示玩家名", &localRadarShowNames);
            ImGui::SliderFloat(
                "水平位置",
                &localRadarAnchorX,
                0.0f,
                1.0f,
                "%.2f");
            ImGui::SliderFloat(
                "垂直位置",
                &localRadarAnchorY,
                0.0f,
                1.0f,
                "%.2f");
            ImGui::SliderFloat(
                "地图大小",
                &localRadarSize,
                0.18f,
                0.65f,
                "%.2f");
            ImGui::SliderFloat(
                "玩家标记大小",
                &localRadarMarkerSize,
                6.0f,
                24.0f,
                "%.0f px");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "内嵌浏览器 Radar");
        ImGui::TextWrapped(
            "通过内嵌 CivetWeb 服务为本地浏览器 "
            "或受信任的局域网访问者提供同一张固定地图。");
        ImGui::Spacing();

        ImGui::Checkbox("启用 Web Radar", &webRadarEnabled);
        ImGui::输入Int("HTTP 端口", &webRadarPort, 1, 100);
        webRadarPort = std::clamp(webRadarPort, 1024, 65535);
        ImGui::Checkbox("允许本局域网内查看", &webRadarLanAccess);

        ImGui::Checkbox(
            "CS2 失焦时暂停 Browser/Relay 采样",
            &webRadar暂停WhenUnfocused);
        ImGui::TextWrapped(
            "CS2 失焦时本地覆盖层总是暂停;仅显式 "
            "共享的访问者可启用后台采样。");
        ImGui::Checkbox(
            "共享玩家名",
            &webRadarIncludePlayerNames);
        const char* teamPolicies[] = {
            "所有队伍",
            "仅本队",
            "仅敌方"
        };
        ImGui::Combo(
            "共享队伍",
            &webRadarTeamViewPolicy,
            teamPolicies,
            IM_ARRAYSIZE(teamPolicies));
        ImGui::Checkbox(
            "共享 Steam ID(资料链接)",
            &webRadarIncludeSteamIds);

        if (webRadarLanAccess) {
            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(0.930f, 0.650f, 0.260f, 1.0f),
                "局域网模式");
            ImGui::TextWrapped(
                "获得含 token URL 的任何人都可以查看流。"
                "仅在受信私有网络内使用;不要将 "
                "该端口暴露到公网。");
        }

        const WebRadarUiStatus status = getWebRadarStatus();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("服务: %s", status.running ? "运行中" : "已停止");
        ImGui::Text("绑定: %s:%d", status.bindAddress.c_str(), webRadarPort);
        ImGui::Text("访问者: %zu", status.viewers);
        ImGui::Text(
            "帧: 已发布 %llu | 已发送 %llu | 已替换 %llu",
            static_cast<unsigned long long>(status.publishedFrames),
            static_cast<unsigned long long>(status.sentFrames),
            static_cast<unsigned long long>(status.replacedFrames));
        ImGui::Text(
            "流量: %.1f MB | 最大发送延迟 %.1f ms",
            static_cast<double>(status.publishedBytes) /
                (1024.0 * 1024.0),
            status.maximumSendLatencyMilliseconds);

        if (!status.error.empty()) {
            ImGui::TextColored(
                ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
                "错误: %s",
                status.error.c_str());
        }

        const bool canOpen = status.running && !status.viewerUrl.empty();
        ImGui::BeginDisabled(!canOpen);
        if (ImGui::Button("打开 Radar")) {
            ShellExecuteA(
                nullptr,
                "open",
                status.viewerUrl.c_str(),
                nullptr,
                nullptr,
                SW_SHOWNORMAL);
        }
        ImGui::SameLine();
        if (ImGui::Button("复制查看者 URL")) {
            ImGui::SetClipboardText(status.viewerUrl.c_str());
        }
        ImGui::EndDisabled();

        if (canOpen) {
            ImGui::TextWrapped("%s", status.viewerUrl.c_str());
            if (webRadarLanAccess) {
                ImGui::TextWrapped(
                    "在其他设备上,请把此 URL 中的 127.0.0.1 替换为 "
                    "this PC's private LAN IPv4 address.");
            }
        }

        ImGui::Spacing();
        ImGui::Checkbox(
            "录制脱敏 Radar 快照",
            &radarRecordingEnabled);
        const web_radar::SnapshotRecorderStatus recording =
            getRecorderStatus();
        if (recording.recording) {
            ImGui::Text(
                "录制中: %llu 帧 (%.1f MB), 已替换 %llu",
                static_cast<unsigned long long>(recording.framesWritten),
                static_cast<double>(recording.bytesWritten) /
                    (1024.0 * 1024.0),
                static_cast<unsigned long long>(recording.replacedFrames));
        }
        if (!recording.path.empty()) {
            ImGui::TextWrapped("文件: %s", recording.path.c_str());
        }
        if (!recording.lastError.empty()) {
            ImGui::TextColored(
                ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
                "%s",
                recording.lastError.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "公网 Relay(出站 WSS)");
        ImGui::TextWrapped(
            "通过已认证、TLS 加密的出站连接 "
            "发布快照。无需入站端口或局域网模式。");
        ImGui::Checkbox("启用公网 Relay", &publicRelayEnabled);

        ImGui::BeginDisabled(publicRelayEnabled);
        ImGui::输入TextWithHint(
            "Relay WSS URL",
            "wss://radar.example.com/api/v1/publish",
            publicRelayUrl.data(),
            publicRelayUrl.size(),
            ImGui输入TextFlags_CharsNoBlank |
                ImGui输入TextFlags_AutoSelectAll);
        ImGui::输入TextWithHint(
            "Relay 房间",
            "match-room",
            publicRelayRoom.data(),
            publicRelayRoom.size(),
            ImGui输入TextFlags_CharsNoBlank |
                ImGui输入TextFlags_AutoSelectAll);
        ImGui::输入TextWithHint(
            "Producer token",
            "粘贴 producer 专用 token",
            publicRelayToken.data(),
            publicRelayToken.size(),
            ImGui输入TextFlags_Password |
                ImGui输入TextFlags_CharsNoBlank |
                ImGui输入TextFlags_AutoSelectAll);
        if (ImGui::Button("清空 Relay 凭证")) {
            std::fill(publicRelayUrl.begin(), publicRelayUrl.end(), '\0');
            std::fill(publicRelayRoom.begin(), publicRelayRoom.end(), '\0');
            SecureZeroMemory(
                publicRelayToken.data(),
                publicRelayToken.size());
        }
        ImGui::EndDisabled();

        ImGui::Checkbox(
            "通过公网 Relay 共享玩家名",
            &publicRelayIncludePlayerNames);
        ImGui::Combo(
            "Relay 队伍",
            &publicRelayTeamViewPolicy,
            teamPolicies,
            IM_ARRAYSIZE(teamPolicies));
        ImGui::Checkbox(
            "通过公网 Relay 共享 Steam ID",
            &publicRelayIncludeSteamIds);
        ImGui::TextWrapped(
            "producer token 仅保存在内存中,绝 "
            "不出现在状态或日志里。请使用 producer token,不要使用 viewer token。");

        const PublicRelayUiStatus relayStatus = getPublicRelayStatus();
        const char* relayState = "未启用";
        switch (relayStatus.state) {
        case web_radar::PublicRelayState::connecting:
            relayState = "连接中";
            break;
        case web_radar::PublicRelayState::connected:
            relayState = "已连接";
            break;
        case web_radar::PublicRelayState::backoff:
            relayState = "重试退避中";
            break;
        case web_radar::PublicRelayState::retiring:
            relayState = "STOPPING";
            break;
        case web_radar::PublicRelayState::failed:
            relayState = "失败";
            break;
        case web_radar::PublicRelayState::disabled:
            break;
        }
        ImGui::Text("Relay: %s", relayState);
        ImGui::Text(
            "已发送帧: %llu  |  已替换: %llu  |  已丢弃: %llu  |  重新连接: %llu",
            static_cast<unsigned long long>(relayStatus.framesSent),
            static_cast<unsigned long long>(relayStatus.replacedFrames),
            static_cast<unsigned long long>(relayStatus.droppedFrames),
            static_cast<unsigned long long>(relayStatus.reconnects));
        if (!relayStatus.error.empty()) {
            ImGui::TextColored(
                ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
                "Relay 错误: %s",
                relayStatus.error.c_str());
        }
    }

    // Render Hotkeys tab content
    inline void RenderHotkeysTab()
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "按键绑定");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        RenderHotkeyButton("切换菜单", &menuToggleKey, "显示/隐藏菜单的按键");
        RenderHotkeyButton("退出程序", &exitKey, "退出程序的按键");
        RenderHotkeyButton("自瞄键", &aimbotKey, "按住激活自瞄");
        RenderHotkeyButton("自动扳机键", &triggerbotKey, "按住激活自动扳机");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "点击按钮后按任意键绑定");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "按 ESC 取消绑定");
        if (!bindingError.empty()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "%s",
                bindingError.c_str());
        }
    }

    // Render Settings tab content
    inline void RenderMiscTab()
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "杂项功能");
        ImGui::Separator();
        ImGui::Spacing();

        if (!memory::WritesAllowed()) {
            ImGui::BeginDisabled();
        }
        ImGui::Checkbox("防闪光", &antiFlash);
        if (!memory::WritesAllowed()) {
            antiFlash = false;
            ImGui::EndDisabled();
            ImGui::TextColored(
                ImVec4(1.0f, 0.65f, 0.1f, 1.0f),
                "内存写入已锁定。启动时使用 --allow-memory-writes 启用");
        }
        ImGui::Checkbox("C4 倒计时", &bombTimer);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "世界实体透视");
        ImGui::Spacing();
        ImGui::Checkbox("手雷透视", &grenadeESP);
        ImGui::Checkbox("掉落武器透视", &droppedWeaponESP);
    }

    inline void RenderSettingsTab()
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "性能");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(
            ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
            "覆盖层目标: %d FPS (%s)",
            sdl_renderer::getTargetRefreshRate(),
            sdl_renderer::isVsyncEnabled()
                ? "VSync"
                : "节流回退");
        ImGui::Text(
            "渲染器: %s",
            sdl_renderer::isAcceleratedRenderer()
                ? "硬件加速"
                : "软件回退(限 60 FPS)");
        if (!sdl_renderer::isGameOnSingleMonitor()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.5f, 0.1f, 1.0f),
                "将 CS2 完整移到单个显示器,以获得可靠的多 DPI 映射");
        }
        if (!sdl_renderer::isDpiAwarenessReliable()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.25f, 0.25f, 1.0f),
                "无法识别多显示器 DPI");
        }
        const char* viewportModes[] = {
            "自动检测黑边",
            "完整客户端(拉伸)",
            "强制 4:3 黑边",
            "强制 16:10 黑边"
        };
        ImGui::Combo(
            "游戏视口",
            &viewportMode,
            viewportModes,
            IM_ARRAYSIZE(viewportModes));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "推荐自动。仅在受采集保护或场景 "
                "过暗导致无法检测黑边时使用强制模式。");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "系统信息");
        ImGui::Spacing();

        ImGui::Text("分辨率: %dx%d", WIDTH, HEIGHT);
        ImGui::Text(
            "游戏视口: %dx%d @ (%d, %d)",
            VIEWPORT_W,
            VIEWPORT_H,
            VIEWPORT_X,
            VIEWPORT_Y);
        ImGui::Text("帧率: %.1f", ImGui::GetIO().Framerate);
        const float frameRate = ImGui::GetIO().Framerate;
        ImGui::Text(
            "帧耗时: %.3f ms",
            frameRate > 0.0f ? 1000.0f / frameRate : 0.0f);

        const auto samplingMetrics =
            performance_metrics::samplingDuration.snapshot();
        const auto serializationMetrics =
            performance_metrics::serializationDuration.snapshot();
        const auto renderMetrics =
            performance_metrics::renderCpuDuration.snapshot();
        const memory::ReadMetrics readMetrics = memory::GetReadMetrics();
        ImGui::Text(
            "采样: %d Hz | Radar: %d Hz | 平均 %.2f ms | P95 %.2f | P99 %.2f",
            performance_metrics::samplingRateHz.load(
                std::memory_order_relaxed),
            performance_metrics::radarRateHz.load(
                std::memory_order_relaxed),
            samplingMetrics.averageMilliseconds,
            samplingMetrics.p95Milliseconds,
            samplingMetrics.p99Milliseconds);
        ImGui::Text(
            "渲染 CPU: 平均 %.2f ms | P95 %.2f | P99 %.2f",
            renderMetrics.averageMilliseconds,
            renderMetrics.p95Milliseconds,
            renderMetrics.p99Milliseconds);
        ImGui::Text(
            "Radar JSON: 平均 %.2f ms | P95 %.2f | 最大 %.2f",
            serializationMetrics.averageMilliseconds,
            serializationMetrics.p95Milliseconds,
            serializationMetrics.maximumMilliseconds);
        ImGui::Text(
            "读取 RPM: %llu 次 | %.1f MB | 失败 %llu",
            static_cast<unsigned long long>(readMetrics.calls),
            static_cast<double>(readMetrics.bytesRequested) /
                (1024.0 * 1024.0),
            static_cast<unsigned long long>(readMetrics.failures));
        ImGui::Text(
            "超时截止: 采样 %llu | 渲染 %llu",
            static_cast<unsigned long long>(
                performance_metrics::missedSamplingDeadlines.load(
                    std::memory_order_relaxed)),
            static_cast<unsigned long long>(
                performance_metrics::missedRenderDeadlines.load(
                    std::memory_order_relaxed)));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            startupReport.ready()
                ? ImVec4(0.250f, 0.900f, 0.600f, 1.0f)
                : ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
            "启动自检: %s",
            startupReport.ready() ? "就绪" : "需关注");
        ImGui::Text(
            "管理员 %s | SDL %s | Web 资源 %s | 地图 %s",
            startupReport.administrator ? "OK" : "FAIL",
            startupReport.sdlRuntimePresent ? "OK" : "FAIL",
            startupReport.webRadarBundlePresent ? "OK" : "FAIL",
            startupReport.mapMetadataPresent ? "OK" : "FAIL");
        if (!startupReport.installationError.empty()) {
            ImGui::TextWrapped(
                "%s",
                startupReport.installationError.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "CS2 External ESP v2.0");
        ImGui::Text("SDL2 + ImGui Overlay");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "github.com/tiansongyu/cs2_cheat");
    }

    inline void RenderPageHeader(
        const char* title,
        const char* description)
    {
        ImGui::TextColored(
            ImVec4(0.930f, 0.960f, 1.000f, 1.0f),
            "%s",
            title);
        ImGui::TextColored(
            ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
            "%s",
            description);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    inline void RenderCombatPage()
    {
        RenderPageHeader(
            "战斗辅助",
            "目标选择与输入自动化。所有输入均需聚焦窗口");
        BeginCard(
            "##自瞄Card",
            "自瞄",
            "真实骨骼目标,稳定的目标保持",
            590.0f);
        Render自瞄Tab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##TriggerCard",
            "自动扳机",
            "使用准星下的真实实体,不进行角度猜测",
            190.0f);
        Render自动扳机Tab();
        EndCard();
    }

    inline void RenderPlayerVisualsPage()
    {
        RenderPageHeader(
            "玩家视图",
            "配置在已验证存活敌人周围显示的信息");
        BeginCard(
            "##PlayerEspCard",
            "玩家透视",
            "方框、血条、骨骼、装备与威胁方向",
            650.0f);
        RenderESPTab();
        EndCard();
    }

    inline void RenderWorldPage()
    {
        RenderPageHeader(
            "世界与比赛",
            "共享 Web Radar、C4 状态与移动世界实体");
        BeginCard(
            "##RadarCard",
            "固定地图 Radar",
            "一个面向正北的地图模型,供本地覆盖层、CivetWeb 与 Relay 共用",
            870.0f);
        RenderRadarTab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##WorldUtilityCard",
            "比赛工具",
            "C4 倒计时、投掷物、掉落装备与防闪光",
            255.0f);
        RenderMiscTab();
        EndCard();
    }

    inline void RenderSystemPage()
    {
        RenderPageHeader(
            "System",
            "显示映射、性能诊断与按键绑定");
        BeginCard(
            "##DisplayCard",
            "显示与渲染",
            "基于显示器的视口映射与实时诊断",
            460.0f);
        RenderSettingsTab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##HotkeyCard",
            "热键",
            "绑定必须唯一;重新绑定期间暂停输入。",
            330.0f);
        RenderHotkeysTab();
        EndCard();
    }

    inline bool NavigationButton(
        const char* label,
        int page,
        const ImVec2& size)
    {
        const bool selected = currentTab == page;
        if (selected) {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.075f, 0.330f, 0.470f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImVec4(0.085f, 0.390f, 0.540f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImVec4(0.100f, 0.440f, 0.600f, 1.0f));
        } else {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.055f, 0.072f, 0.100f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImVec4(0.095f, 0.130f, 0.175f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImVec4(0.110f, 0.155f, 0.205f, 1.0f));
        }

        const bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        if (pressed) {
            currentTab = page;
        }
        return pressed;
    }

    // Main render function
    inline void render()
    {
        if (!sdl_renderer::menuVisible) return;

        // Update key binding
        UpdateKeyBinding();

        const float dpiScale = sdl_renderer::getDpiScale();
        const float margin = std::max(8.0f, 16.0f * dpiScale);
        const float availableWidth =
            std::max(1.0f, static_cast<float>(WIDTH) - margin * 2.0f);
        const float availableHeight =
            std::max(1.0f, static_cast<float>(HEIGHT) - margin * 2.0f);
        const float defaultWidth =
            std::min(920.0f * dpiScale, availableWidth);
        const float defaultHeight =
            std::min(720.0f * dpiScale, availableHeight);
        const float minimumWidth =
            std::min(680.0f * dpiScale, availableWidth);
        const float minimumHeight =
            std::min(500.0f * dpiScale, availableHeight);

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(minimumWidth, minimumHeight),
            ImVec2(availableWidth, availableHeight));
        ImGui::SetNextWindowSize(
            ImVec2(defaultWidth, defaultHeight),
            ImGuiCond_FirstUseEver
        );
        ImGui::SetNextWindowPos(
            ImVec2(
                (static_cast<float>(WIDTH) - defaultWidth) / 2.0f,
                (static_cast<float>(HEIGHT) - defaultHeight) / 2.0f),
            ImGuiCond_FirstUseEver
        );

        ImGui::Begin(
            "Aegis // CS2 覆盖层",
            nullptr,
            ImGuiWindowFlags_NoCollapse);

        // Saved ImGui positions may belong to another monitor/resolution.
        // Clamp without resetting a valid user-selected position or size.
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 clampedPos(
            std::clamp(
                windowPos.x,
                margin,
                std::max(margin, static_cast<float>(WIDTH) - windowSize.x - margin)),
            std::clamp(
                windowPos.y,
                margin,
                std::max(margin, static_cast<float>(HEIGHT) - windowSize.y - margin)));
        if (clampedPos.x != windowPos.x || clampedPos.y != windowPos.y) {
            ImGui::SetWindowPos(clampedPos);
        }
        {
            const ImVec2 interactivePosition = ImGui::GetWindowPos();
            const ImVec2 interactiveSize = ImGui::GetWindowSize();
            sdl_renderer::setInteractiveRect(
                interactivePosition.x,
                interactivePosition.y,
                interactiveSize.x,
                interactiveSize.y);
        }

        const float sidebarWidth = 184.0f * dpiScale;
        const float navigationHeight = 42.0f * dpiScale;

        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ImVec4(0.038f, 0.052f, 0.075f, 1.0f));
        ImGui::BeginChild(
            "##Navigation",
            ImVec2(sidebarWidth, 0.0f),
            true);
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "AEGIS");
        ImGui::TextColored(
            ImVec4(0.500f, 0.570f, 0.670f, 1.0f),
            "CS2 覆盖层");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const ImVec2 navigationSize(
            ImGui::GetContentRegionAvail().x,
            navigationHeight);
        NavigationButton("战斗", 0, navigationSize);
        NavigationButton("玩家视图", 1, navigationSize);
        NavigationButton("世界与 Radar", 2, navigationSize);
        NavigationButton("系统", 3, navigationSize);
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, 12.0f * dpiScale);
        ImGui::BeginChild(
            "##PageContent",
            ImVec2(0.0f, 0.0f),
            false);
        switch (currentTab) {
        case 0:
            RenderCombatPage();
            break;
        case 1:
            RenderPlayerVisualsPage();
            break;
        case 2:
            RenderWorldPage();
            break;
        case 3:
            RenderSystemPage();
            break;
        default:
            currentTab = 0;
            RenderCombatPage();
            break;
        }
        ImGui::EndChild();

        ImGui::End();
        if (ImGui::IsPopupOpen(
                nullptr,
                ImGuiPopupFlags_AnyPopup)) {
            // Popups may extend beyond the main menu rectangle. Keep the whole
            // overlay interactive while one is open so their first click cannot
            // pass through to CS2.
            sdl_renderer::setInteractiveRect(
                0.0f,
                0.0f,
                static_cast<float>(WIDTH),
                static_cast<float>(HEIGHT));
        }
        publishRuntimeConfig();
    }
}
