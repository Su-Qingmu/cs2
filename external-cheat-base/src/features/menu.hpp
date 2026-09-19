#pragma once

#include "imgui.h"
#include "core/renderer/sdl_renderer.h"
#include "core/memory/memory.hpp"
#include "core/diagnostics.hpp"
#include "core/performance_metrics.hpp"
#include "features/i18n.hpp"
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
        bool webRadarPauseWhenUnfocused = true;
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
        int smartAimPriority = 0;
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
    inline bool espWeapon = true;    // Weapon display - Default ON
    inline bool espViewAngle = true; // View angle indicator - Default ON
    inline bool espViewAngleText = false; // Show angle degree text
    inline bool espFlashIndicator = false; // Flashbang eye indicator - Default OFF
    inline bool espWallCheck = true; // CS2 spotted-state indicator
    inline bool espSnaplines = false;

    // Skeleton ESP
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
    inline int snaplinesOrigin = 0; // 0=Bottom, 1=Center, 2=Top

    // Aimbot Settings
    inline bool aimbotEnabled = false;     // Aimbot enabled
    inline float aimbotFOV = 10.0f;        // Field of view for aimbot (degrees)
    inline float aimbotSmoothing = 5.0f;   // Smoothing factor (1.0 = instant, higher = smoother)
    inline int aimbotBone = 0;             // 0=Head, 1=Neck, 2=Chest
    inline bool aimbotVisibleOnly = true;  // Only aim at enemies flagged as spotted
    inline int aimbotKey = VK_SHIFT;       // Aimbot activation key (default: Shift key)
    inline bool aimbotShowFOV = true;      // Show FOV circle on screen
    inline float aimbotFOVColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };  // Color-key overlays use opaque primitives

    // Head Offset Settings (for side-facing enemies)
    inline bool headOffsetEnabled = true;      // Enable head offset compensation
    inline float headOffsetAmount = 5.0f;      // Offset amount in game units (0-15)
    inline float headOffsetAngleMin = 45.0f;   // Minimum angle for offset (degrees)
    inline float headOffsetAngleMax = 135.0f;  // Maximum angle for offset (degrees)

    // Smart Aim Settings (auto-lock spotted enemies by priority)
    inline bool smartAimEnabled = false;      // Smart aim mode (ignores FOV, auto-selects best target)
    inline int smartAimPriority = 0;          // 0=Distance first, 1=Health first

    // Triggerbot Settings
    inline bool triggerbotEnabled = false; // Triggerbot enabled
    inline int triggerbotDelay = 50;       // Delay before shooting (milliseconds)
    inline int triggerbotKey = 0x46;       // Triggerbot activation key (default: F key, 0x46 = 'F')

    // Input Settings
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
    inline bool webRadarPauseWhenUnfocused = true;
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

    // Menu Toggle Key
    inline int menuToggleKey = VK_F4;        // Menu toggle key (default: F4)
    inline int exitKey = VK_F9;              // Exit key (default: F9)

    // Hotkey binding state. The conflict is kept as a string id rather than a
    // formatted message so it re-renders in the active language.
    inline bool isBindingKey = false;
    inline int* bindingKeyTarget = nullptr;
    inline const char* bindingKeyName = nullptr;
    inline bool bindingWaitingForRelease = false;
    inline bool bindingConflictActive = false;
    inline i18n::Str bindingConflictId = i18n::Str::HotkeyMenuToggle;
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
        config.webRadarPauseWhenUnfocused =
            webRadarPauseWhenUnfocused;
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
        config.smartAimPriority = smartAimPriority;
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

    inline std::wstring readPersistentString(
        const wchar_t* key,
        const std::filesystem::path& path)
    {
        std::array<wchar_t, 64> value{};
        GetPrivateProfileStringW(
            L"settings",
            key,
            L"",
            value.data(),
            static_cast<DWORD>(value.size()),
            path.c_str());
        return std::wstring(value.data());
    }

    inline void loadPersistentSettings()
    {
        const std::filesystem::path path = persistentSettingsPath();

        // Resolved before the schema check: a first run has no settings file
        // at all, and still has to follow the system language. An absent or
        // unrecognized value falls back to the system language inside
        // parsePersisted().
        i18n::currentLanguage = i18n::parsePersisted(
            readPersistentString(L"ui_language", path));

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
        webRadarPauseWhenUnfocused = readPersistentInt(
            L"pause_shared_radar_unfocused",
            webRadarPauseWhenUnfocused ? 1 : 0,
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
            L"ui_language",
            i18n::persistValue(i18n::currentLanguage),
            path);
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
            webRadarPauseWhenUnfocused ? L"1" : L"0",
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

    // Convert virtual key code to key name. Deliberately not localized: these
    // are keycap legends, and every keyboard prints them in English.
    inline const char* GetKeyName(int vkCode)
    {
        static char keyName[32];

        switch (vkCode)
        {
        // Special keys
        case VK_LBUTTON: return "Mouse1";
        case VK_RBUTTON: return "Mouse2";
        case VK_MBUTTON: return "Mouse3";
        case VK_XBUTTON1: return "Mouse4";
        case VK_XBUTTON2: return "Mouse5";
        case VK_BACK: return "Backspace";
        case VK_TAB: return "Tab";
        case VK_RETURN: return "Enter";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_PAUSE: return "Pause";
        case VK_CAPITAL: return "CapsLock";
        case VK_ESCAPE: return "Escape";
        case VK_SPACE: return "Space";
        case VK_PRIOR: return "PageUp";
        case VK_NEXT: return "PageDown";
        case VK_END: return "End";
        case VK_HOME: return "Home";
        case VK_LEFT: return "Left";
        case VK_UP: return "Up";
        case VK_RIGHT: return "Right";
        case VK_DOWN: return "Down";
        case VK_INSERT: return "Insert";
        case VK_DELETE: return "Delete";
        case VK_LSHIFT: return "LShift";
        case VK_RSHIFT: return "RShift";
        case VK_LCONTROL: return "LCtrl";
        case VK_RCONTROL: return "RCtrl";
        case VK_LMENU: return "LAlt";
        case VK_RMENU: return "RAlt";

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
        case VK_NUMPAD0: return "Num0";
        case VK_NUMPAD1: return "Num1";
        case VK_NUMPAD2: return "Num2";
        case VK_NUMPAD3: return "Num3";
        case VK_NUMPAD4: return "Num4";
        case VK_NUMPAD5: return "Num5";
        case VK_NUMPAD6: return "Num6";
        case VK_NUMPAD7: return "Num7";
        case VK_NUMPAD8: return "Num8";
        case VK_NUMPAD9: return "Num9";
        case VK_MULTIPLY: return "Num*";
        case VK_ADD: return "Num+";
        case VK_SUBTRACT: return "Num-";
        case VK_DECIMAL: return "Num.";
        case VK_DIVIDE: return "Num/";

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
            if (i == VK_ESCAPE) continue;  // Escape cancels binding

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

    // Reports the binding that already owns `candidate`, if any. The name is
    // returned as a string id so the caller can render it in the active
    // language rather than storing a formatted message.
    inline bool FindHotkeyConflict(
        const int* target,
        const int candidate,
        i18n::Str& conflictName)
    {
        struct Binding
        {
            i18n::Str name;
            const int* key;
        };
        const Binding bindings[] = {
            { i18n::Str::HotkeyMenuToggle, &menuToggleKey },
            { i18n::Str::HotkeyExitProgram, &exitKey },
            { i18n::Str::HotkeyAimbotKey, &aimbotKey },
            { i18n::Str::HotkeyTriggerbotKey, &triggerbotKey }
        };
        for (const Binding& binding : bindings) {
            if (binding.key != target && *binding.key == candidate) {
                conflictName = binding.name;
                return true;
            }
        }
        return false;
    }

    // Render hotkey button
    inline void RenderHotkeyButton(
        const i18n::Str id,
        int* keyCode,
        const i18n::Str tooltipId)
    {
        const float dpiScale = sdl_renderer::getDpiScale();
        const char* const label = i18n::tr(id);
        // The English text is used as the "##" suffix so the button keeps one
        // identity while its visible label changes language.
        const char* const stableId = i18n::trStable(id);
        ImGui::Text("%s:", label);
        ImGui::SameLine(150.0f * dpiScale);

        char buttonLabel[96];
        if (isBindingKey && bindingKeyTarget == keyCode)
        {
            sprintf_s(
                buttonLabel,
                "%s##%s",
                i18n::tr(i18n::Str::HotkeyPressKey),
                stableId);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        }
        else
        {
            sprintf_s(buttonLabel, "%s##%s", GetKeyName(*keyCode), stableId);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
        }

        if (ImGui::Button(buttonLabel, ImVec2(100.0f * dpiScale, 0.0f)))
        {
            isBindingKey = true;
            bindingKeyTarget = keyCode;
            bindingKeyName = label;
            bindingWaitingForRelease = true;
            bindingConflictActive = false;
            suppressHotkeysUntilRelease = true;
        }

        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", i18n::tr(tooltipId));
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
            i18n::Str conflict = i18n::Str::HotkeyMenuToggle;
            if (FindHotkeyConflict(bindingKeyTarget, pressedKey, conflict)) {
                bindingConflictId = conflict;
                bindingConflictActive = true;
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

    // Render Aimbot tab content
    inline void RenderAimbotTab()
    {
        ImGui::Checkbox(
            i18n::trId(i18n::Str::AimbotEnable),
            &aimbotEnabled);

        if (aimbotEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox(
                i18n::trId(i18n::Str::AimbotSmartAim),
                &smartAimEnabled);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(
                    "%s",
                    i18n::tr(i18n::Str::AimbotSmartAimTooltip));

            if (smartAimEnabled) {
                ImGui::Indent();
                const char* priorityItems[] = {
                    i18n::tr(i18n::Str::AimbotPriorityDistance),
                    i18n::tr(i18n::Str::AimbotPriorityHealth)
                };
                ImGui::Combo(
                    i18n::trId(i18n::Str::AimbotPriority),
                    &smartAimPriority,
                    priorityItems,
                    IM_ARRAYSIZE(priorityItems));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "%s",
                        i18n::tr(i18n::Str::AimbotPriorityTooltip));
                ImGui::Unindent();
            }

            if (!smartAimEnabled) {
                ImGui::SliderFloat(
                    i18n::trId(i18n::Str::AimbotFov),
                    &aimbotFOV,
                    1.0f,
                    30.0f,
                    i18n::tr(i18n::Str::AimbotFovFormat));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "%s",
                        i18n::tr(i18n::Str::AimbotFovTooltip));
            }

            ImGui::SliderFloat(
                i18n::trId(i18n::Str::AimbotSmoothing),
                &aimbotSmoothing,
                1.0f,
                20.0f,
                "%.1f");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(
                    "%s",
                    i18n::tr(i18n::Str::AimbotSmoothingTooltip));

            const char* boneItems[] = {
                i18n::tr(i18n::Str::AimbotBoneHead),
                i18n::tr(i18n::Str::AimbotBoneNeck),
                i18n::tr(i18n::Str::AimbotBoneChest)
            };
            ImGui::Combo(
                i18n::trId(i18n::Str::AimbotTargetBone),
                &aimbotBone,
                boneItems,
                IM_ARRAYSIZE(boneItems));

            if (!smartAimEnabled) {
                ImGui::Checkbox(
                    i18n::trId(i18n::Str::AimbotSpottedOnly),
                    &aimbotVisibleOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "%s",
                        i18n::tr(i18n::Str::AimbotSpottedOnlyTooltip));
            }

            ImGui::Checkbox(
                i18n::trId(i18n::Str::AimbotShowFovCircle),
                &aimbotShowFOV);
            if (aimbotShowFOV) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##FOVColor", aimbotFOVColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::AimbotHeadOffset));

            ImGui::Checkbox(
                i18n::trId(i18n::Str::AimbotHeadOffsetEnable),
                &headOffsetEnabled);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(
                    "%s",
                    i18n::tr(i18n::Str::AimbotHeadOffsetEnableTooltip));

            if (headOffsetEnabled) {
                ImGui::Indent();
                ImGui::SliderFloat(
                    i18n::trId(i18n::Str::AimbotHeadOffsetAmount),
                    &headOffsetAmount,
                    0.0f,
                    15.0f,
                    i18n::tr(i18n::Str::AimbotHeadOffsetAmountFormat));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "%s",
                        i18n::tr(i18n::Str::AimbotHeadOffsetAmountTooltip));

                ImGui::SliderFloat(
                    i18n::trId(i18n::Str::AimbotHeadOffsetMinAngle),
                    &headOffsetAngleMin,
                    0.0f,
                    90.0f,
                    i18n::tr(i18n::Str::AimbotHeadOffsetAngleFormat));
                ImGui::SliderFloat(
                    i18n::trId(i18n::Str::AimbotHeadOffsetMaxAngle),
                    &headOffsetAngleMax,
                    90.0f,
                    180.0f,
                    i18n::tr(i18n::Str::AimbotHeadOffsetAngleFormat));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(
                        "%s",
                        i18n::tr(i18n::Str::AimbotHeadOffsetAngleTooltip));

                ImGui::TextColored(
                    ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::AimbotHeadOffsetAngleLegend));
                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::AimbotInput));
            ImGui::SliderFloat(
                i18n::trId(i18n::Str::AimbotMouseSensitivity),
                &mouseSensitivity,
                0.1f,
                10.0f,
                i18n::tr(i18n::Str::AimbotMouseSensitivityFormat));
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(
                    "%s",
                    i18n::tr(i18n::Str::AimbotMouseSensitivityTooltip));

            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                i18n::tr(i18n::Str::AimbotHoldToAim),
                GetKeyName(aimbotKey));
        }
    }

    // Render Triggerbot tab content
    inline void RenderTriggerbotTab()
    {
        ImGui::Checkbox(
            i18n::trId(i18n::Str::TriggerbotEnable),
            &triggerbotEnabled);

        if (triggerbotEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderInt(
                i18n::trId(i18n::Str::TriggerbotDelay),
                &triggerbotDelay,
                0,
                500,
                i18n::tr(i18n::Str::TriggerbotDelayFormat));
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(
                    "%s",
                    i18n::tr(i18n::Str::TriggerbotDelayTooltip));

            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                i18n::tr(i18n::Str::TriggerbotHoldToActivate),
                GetKeyName(triggerbotKey));
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::TriggerbotDescription));
        }
    }

    // Render ESP tab content
    inline void RenderESPTab()
    {
        ImGui::Checkbox(
            i18n::trId(i18n::Str::EspEnable),
            &espEnabled);
        ImGui::TextDisabled("%s", i18n::tr(i18n::Str::EspEnableHint));

        if (espEnabled)
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Box ESP
            ImGui::Checkbox(i18n::trId(i18n::Str::EspBox), &espBox);
            if (espBox) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##BoxColor", espBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // Health Bar
            ImGui::Checkbox(i18n::trId(i18n::Str::EspHealth), &espHealth);

            // Weapon Display
            ImGui::Checkbox(i18n::trId(i18n::Str::EspWeapon), &espWeapon);
            if (espWeapon) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##WeaponColor", espWeaponColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // View Direction
            ImGui::Checkbox(
                i18n::trId(i18n::Str::EspViewDirection),
                &espViewAngle);
            if (espViewAngle) {
                ImGui::Indent();
                ImGui::TextColored(
                    ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::EspFacingYou));
                ImGui::TextColored(
                    ImVec4(1.0f, 0.65f, 0.0f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::EspPartial));
                ImGui::TextColored(
                    ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::EspSide));
                ImGui::TextColored(
                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::EspBack));
                ImGui::Checkbox(
                    i18n::trId(i18n::Str::EspShowAngleDegrees),
                    &espViewAngleText);
                ImGui::Unindent();
            }

            // CS2 spotted-state check. This is intentionally not described as
            // a ray-cast: it is a conservative game-state signal.
            ImGui::Checkbox(
                i18n::trId(i18n::Str::EspSpottedCheck),
                &espWallCheck);
            if (espWallCheck) {
                ImGui::Indent();
                ImGui::Text("%s", i18n::tr(i18n::Str::EspSpottedColor));
                ImGui::SameLine();
                ImGui::ColorEdit4("##BoxColor2", espBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Text("%s", i18n::tr(i18n::Str::EspNotSpottedColor));
                ImGui::SameLine();
                ImGui::ColorEdit4("##WallColor", espWallColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::TextColored(
                    ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "%s",
                    i18n::tr(i18n::Str::EspSpottedCheckNote));
                ImGui::Unindent();
            }

            // Distance
            ImGui::Checkbox(i18n::trId(i18n::Str::EspDistance), &espDistance);
            if (espDistance) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##DistanceColor", espDistanceColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            }

            // Flashbang Eye Indicator
            ImGui::Checkbox(
                i18n::trId(i18n::Str::EspFlashIndicator),
                &espFlashIndicator);
            if (espFlashIndicator) {
                ImGui::Indent();
                ImGui::Text("%s", i18n::tr(i18n::Str::EspFlashNormalEye));
                ImGui::SameLine();
                ImGui::ColorEdit4("##FlashNormalColor", espFlashNormalColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Text("%s", i18n::tr(i18n::Str::EspFlashedEye));
                ImGui::SameLine();
                ImGui::ColorEdit4("##FlashColor", espFlashColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Unindent();
            }

            // Snaplines
            ImGui::Checkbox(i18n::trId(i18n::Str::EspSnaplines), &espSnaplines);
            if (espSnaplines) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##SnaplinesColor", espSnaplinesColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
                ImGui::Indent();
                const char* origins[] = {
                    i18n::tr(i18n::Str::EspOriginBottom),
                    i18n::tr(i18n::Str::EspOriginCenter),
                    i18n::tr(i18n::Str::EspOriginTop)
                };
                ImGui::Combo(
                    i18n::trId(i18n::Str::EspSnaplinesOrigin),
                    &snaplinesOrigin,
                    origins,
                    IM_ARRAYSIZE(origins));
                ImGui::Unindent();
            }

            // Skeleton
            ImGui::Checkbox(i18n::trId(i18n::Str::EspSkeleton), &espSkeleton);
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
        const char* teamPolicies[] = {
            i18n::tr(i18n::Str::RadarWebTeamAll),
            i18n::tr(i18n::Str::RadarWebTeamLocal),
            i18n::tr(i18n::Str::RadarWebTeamOpponents)
        };

        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::RadarLocalHeading));
        ImGui::TextWrapped(
            "%s",
            i18n::tr(i18n::Str::RadarLocalDescription));
        ImGui::Spacing();

        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarLocalEnable),
            &localRadarEnabled);
        if (localRadarEnabled) {
            ImGui::Checkbox(
                i18n::trId(i18n::Str::RadarLocalShowNames),
                &localRadarShowNames);
            ImGui::SliderFloat(
                i18n::trId(i18n::Str::RadarLocalAnchorX),
                &localRadarAnchorX,
                0.0f,
                1.0f,
                "%.2f");
            ImGui::SliderFloat(
                i18n::trId(i18n::Str::RadarLocalAnchorY),
                &localRadarAnchorY,
                0.0f,
                1.0f,
                "%.2f");
            ImGui::SliderFloat(
                i18n::trId(i18n::Str::RadarLocalSize),
                &localRadarSize,
                0.18f,
                0.65f,
                "%.2f");
            ImGui::SliderFloat(
                i18n::trId(i18n::Str::RadarLocalMarkerSize),
                &localRadarMarkerSize,
                6.0f,
                24.0f,
                i18n::tr(i18n::Str::RadarLocalMarkerSizeFormat));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(0.330f, 0.800f, 1.000f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::RadarWebHeading));
        ImGui::TextWrapped(
            "%s",
            i18n::tr(i18n::Str::RadarWebDescription));
        ImGui::Spacing();

        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarWebEnable),
            &webRadarEnabled);
        ImGui::InputInt(i18n::trId(i18n::Str::RadarWebPort), &webRadarPort, 1, 100);
        webRadarPort = std::clamp(webRadarPort, 1024, 65535);
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarWebLanAccess),
            &webRadarLanAccess);

        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarWebPauseWhenUnfocused),
            &webRadarPauseWhenUnfocused);
        ImGui::TextWrapped(
            "%s",
            i18n::tr(i18n::Str::RadarWebPauseWhenUnfocusedNote));
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarWebShareNames),
            &webRadarIncludePlayerNames);
        ImGui::Combo(
            i18n::trId(i18n::Str::RadarWebSharedTeams),
            &webRadarTeamViewPolicy,
            teamPolicies,
            IM_ARRAYSIZE(teamPolicies));
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarWebShareSteamIds),
            &webRadarIncludeSteamIds);

        if (webRadarLanAccess) {
            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(0.930f, 0.650f, 0.260f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::RadarWebLanMode));
            ImGui::TextWrapped(
                "%s",
                i18n::tr(i18n::Str::RadarWebLanModeWarning));
        }

        const WebRadarUiStatus status = getWebRadarStatus();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text(
            i18n::tr(i18n::Str::RadarWebService),
            status.running
                ? i18n::tr(i18n::Str::RadarWebServiceRunning)
                : i18n::tr(i18n::Str::RadarWebServiceStopped));
        ImGui::Text(
            i18n::tr(i18n::Str::RadarWebBind),
            status.bindAddress.c_str(),
            webRadarPort);
        ImGui::Text(i18n::tr(i18n::Str::RadarWebViewers), status.viewers);
        ImGui::Text(
            i18n::tr(i18n::Str::RadarWebFrames),
            static_cast<unsigned long long>(status.publishedFrames),
            static_cast<unsigned long long>(status.sentFrames),
            static_cast<unsigned long long>(status.replacedFrames));
        ImGui::Text(
            i18n::tr(i18n::Str::RadarWebTraffic),
            static_cast<double>(status.publishedBytes) /
                (1024.0 * 1024.0),
            status.maximumSendLatencyMilliseconds);

        if (!status.error.empty()) {
            ImGui::TextColored(
                ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
                i18n::tr(i18n::Str::RadarWebError),
                status.error.c_str());
        }

        const bool canOpen = status.running && !status.viewerUrl.empty();
        ImGui::BeginDisabled(!canOpen);
        if (ImGui::Button(i18n::trId(i18n::Str::RadarWebOpen))) {
            ShellExecuteA(
                nullptr,
                "open",
                status.viewerUrl.c_str(),
                nullptr,
                nullptr,
                SW_SHOWNORMAL);
        }
        ImGui::SameLine();
        if (ImGui::Button(i18n::trId(i18n::Str::RadarWebCopyUrl))) {
            ImGui::SetClipboardText(status.viewerUrl.c_str());
        }
        ImGui::EndDisabled();

        if (canOpen) {
            ImGui::TextWrapped("%s", status.viewerUrl.c_str());
            if (webRadarLanAccess) {
                ImGui::TextWrapped(
                    "%s",
                    i18n::tr(i18n::Str::RadarWebLanUrlHint));
            }
        }

        ImGui::Spacing();
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RadarRecordEnable),
            &radarRecordingEnabled);
        const web_radar::SnapshotRecorderStatus recording =
            getRecorderStatus();
        if (recording.recording) {
            ImGui::Text(
                i18n::tr(i18n::Str::RadarRecordProgress),
                static_cast<unsigned long long>(recording.framesWritten),
                static_cast<double>(recording.bytesWritten) /
                    (1024.0 * 1024.0),
                static_cast<unsigned long long>(recording.replacedFrames));
        }
        if (!recording.path.empty()) {
            ImGui::TextWrapped(
                i18n::tr(i18n::Str::RadarRecordFile),
                recording.path.c_str());
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
            "%s",
            i18n::tr(i18n::Str::RelayHeading));
        ImGui::TextWrapped(
            "%s",
            i18n::tr(i18n::Str::RelayDescription));
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RelayEnable),
            &publicRelayEnabled);

        ImGui::BeginDisabled(publicRelayEnabled);
        ImGui::InputTextWithHint(
            i18n::trId(i18n::Str::RelayUrl),
            i18n::tr(i18n::Str::RelayUrlHint),
            publicRelayUrl.data(),
            publicRelayUrl.size(),
            ImGuiInputTextFlags_CharsNoBlank |
                ImGuiInputTextFlags_AutoSelectAll);
        ImGui::InputTextWithHint(
            i18n::trId(i18n::Str::RelayRoom),
            i18n::tr(i18n::Str::RelayRoomHint),
            publicRelayRoom.data(),
            publicRelayRoom.size(),
            ImGuiInputTextFlags_CharsNoBlank |
                ImGuiInputTextFlags_AutoSelectAll);
        ImGui::InputTextWithHint(
            i18n::trId(i18n::Str::RelayToken),
            i18n::tr(i18n::Str::RelayTokenHint),
            publicRelayToken.data(),
            publicRelayToken.size(),
            ImGuiInputTextFlags_Password |
                ImGuiInputTextFlags_CharsNoBlank |
                ImGuiInputTextFlags_AutoSelectAll);
        if (ImGui::Button(i18n::trId(i18n::Str::RelayClear))) {
            std::fill(publicRelayUrl.begin(), publicRelayUrl.end(), '\0');
            std::fill(publicRelayRoom.begin(), publicRelayRoom.end(), '\0');
            SecureZeroMemory(
                publicRelayToken.data(),
                publicRelayToken.size());
        }
        ImGui::EndDisabled();

        ImGui::Checkbox(
            i18n::trId(i18n::Str::RelayShareNames),
            &publicRelayIncludePlayerNames);
        ImGui::Combo(
            i18n::trId(i18n::Str::RelayTeams),
            &publicRelayTeamViewPolicy,
            teamPolicies,
            IM_ARRAYSIZE(teamPolicies));
        ImGui::Checkbox(
            i18n::trId(i18n::Str::RelayShareSteamIds),
            &publicRelayIncludeSteamIds);
        ImGui::TextWrapped(
            "%s",
            i18n::tr(i18n::Str::RelayTokenNote));

        const PublicRelayUiStatus relayStatus = getPublicRelayStatus();
        const char* relayState = i18n::tr(i18n::Str::RelayStateDisabled);
        switch (relayStatus.state) {
        case web_radar::PublicRelayState::connecting:
            relayState = i18n::tr(i18n::Str::RelayStateConnecting);
            break;
        case web_radar::PublicRelayState::connected:
            relayState = i18n::tr(i18n::Str::RelayStateConnected);
            break;
        case web_radar::PublicRelayState::backoff:
            relayState = i18n::tr(i18n::Str::RelayStateBackoff);
            break;
        case web_radar::PublicRelayState::retiring:
            relayState = i18n::tr(i18n::Str::RelayStateStopping);
            break;
        case web_radar::PublicRelayState::failed:
            relayState = i18n::tr(i18n::Str::RelayStateFailed);
            break;
        case web_radar::PublicRelayState::disabled:
            break;
        }
        ImGui::Text(i18n::tr(i18n::Str::RelayState), relayState);
        ImGui::Text(
            i18n::tr(i18n::Str::RelayCounters),
            static_cast<unsigned long long>(relayStatus.framesSent),
            static_cast<unsigned long long>(relayStatus.replacedFrames),
            static_cast<unsigned long long>(relayStatus.droppedFrames),
            static_cast<unsigned long long>(relayStatus.reconnects));
        if (!relayStatus.error.empty()) {
            ImGui::TextColored(
                ImVec4(0.930f, 0.420f, 0.430f, 1.0f),
                i18n::tr(i18n::Str::RelayError),
                relayStatus.error.c_str());
        }
    }

    // Render Hotkeys tab content
    inline void RenderHotkeysTab()
    {
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::HotkeysHeading));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        RenderHotkeyButton(
            i18n::Str::HotkeyMenuToggle,
            &menuToggleKey,
            i18n::Str::HotkeyMenuToggleTooltip);
        RenderHotkeyButton(
            i18n::Str::HotkeyExitProgram,
            &exitKey,
            i18n::Str::HotkeyExitProgramTooltip);
        RenderHotkeyButton(
            i18n::Str::HotkeyAimbotKey,
            &aimbotKey,
            i18n::Str::HotkeyAimbotKeyTooltip);
        RenderHotkeyButton(
            i18n::Str::HotkeyTriggerbotKey,
            &triggerbotKey,
            i18n::Str::HotkeyTriggerbotKeyTooltip);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::HotkeyBindHint));
        ImGui::TextColored(
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::HotkeyCancelHint));
        if (bindingConflictActive) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                "%s%s",
                i18n::tr(i18n::Str::HotkeyConflictPrefix),
                i18n::tr(bindingConflictId));
        }
    }

    // Render Settings tab content
    inline void RenderMiscTab()
    {
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::MiscHeading));
        ImGui::Separator();
        ImGui::Spacing();

        if (!memory::WritesAllowed()) {
            ImGui::BeginDisabled();
        }
        ImGui::Checkbox(i18n::trId(i18n::Str::MiscAntiFlash), &antiFlash);
        if (!memory::WritesAllowed()) {
            antiFlash = false;
            ImGui::EndDisabled();
            ImGui::TextColored(
                ImVec4(1.0f, 0.65f, 0.1f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::MiscMemoryWritesLocked));
        }
        ImGui::Checkbox(i18n::trId(i18n::Str::MiscBombTimer), &bombTimer);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::MiscWorldEsp));
        ImGui::Spacing();
        ImGui::Checkbox(i18n::trId(i18n::Str::MiscGrenadeEsp), &grenadeESP);
        ImGui::Checkbox(
            i18n::trId(i18n::Str::MiscDroppedWeaponEsp),
            &droppedWeaponESP);
    }

    inline void RenderSettingsTab()
    {
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::SettingsPerformance));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(
            ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
            i18n::tr(i18n::Str::SettingsOverlayTarget),
            sdl_renderer::getTargetRefreshRate(),
            sdl_renderer::isVsyncEnabled()
                ? i18n::tr(i18n::Str::SettingsVsync)
                : i18n::tr(i18n::Str::SettingsPacedFallback));
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsRenderer),
            sdl_renderer::isAcceleratedRenderer()
                ? i18n::tr(i18n::Str::SettingsRendererHardware)
                : i18n::tr(i18n::Str::SettingsRendererSoftware));
        if (!sdl_renderer::isGameOnSingleMonitor()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.5f, 0.1f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::SettingsSingleMonitorWarning));
        }
        if (!sdl_renderer::isDpiAwarenessReliable()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.25f, 0.25f, 1.0f),
                "%s",
                i18n::tr(i18n::Str::SettingsDpiWarning));
        }
        const char* viewportModes[] = {
            i18n::tr(i18n::Str::SettingsViewportAuto),
            i18n::tr(i18n::Str::SettingsViewportFullClient),
            i18n::tr(i18n::Str::SettingsViewportForce43),
            i18n::tr(i18n::Str::SettingsViewportForce1610)
        };
        ImGui::Combo(
            i18n::trId(i18n::Str::SettingsViewportMode),
            &viewportMode,
            viewportModes,
            IM_ARRAYSIZE(viewportModes));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "%s",
                i18n::tr(i18n::Str::SettingsViewportTooltip));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::SettingsSystemInfo));
        ImGui::Spacing();

        ImGui::Text(
            i18n::tr(i18n::Str::SettingsResolution),
            WIDTH,
            HEIGHT);
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsGameViewport),
            VIEWPORT_W,
            VIEWPORT_H,
            VIEWPORT_X,
            VIEWPORT_Y);
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsFps),
            ImGui::GetIO().Framerate);
        const float frameRate = ImGui::GetIO().Framerate;
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsFrameTime),
            frameRate > 0.0f ? 1000.0f / frameRate : 0.0f);

        const auto samplingMetrics =
            performance_metrics::samplingDuration.snapshot();
        const auto serializationMetrics =
            performance_metrics::serializationDuration.snapshot();
        const auto renderMetrics =
            performance_metrics::renderCpuDuration.snapshot();
        const memory::ReadMetrics readMetrics = memory::GetReadMetrics();
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsSampling),
            performance_metrics::samplingRateHz.load(
                std::memory_order_relaxed),
            performance_metrics::radarRateHz.load(
                std::memory_order_relaxed),
            samplingMetrics.averageMilliseconds,
            samplingMetrics.p95Milliseconds,
            samplingMetrics.p99Milliseconds);
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsRenderCpu),
            renderMetrics.averageMilliseconds,
            renderMetrics.p95Milliseconds,
            renderMetrics.p99Milliseconds);
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsRadarJson),
            serializationMetrics.averageMilliseconds,
            serializationMetrics.p95Milliseconds,
            serializationMetrics.maximumMilliseconds);
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsRpm),
            static_cast<unsigned long long>(readMetrics.calls),
            static_cast<double>(readMetrics.bytesRequested) /
                (1024.0 * 1024.0),
            static_cast<unsigned long long>(readMetrics.failures));
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsMissedDeadlines),
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
            i18n::tr(i18n::Str::SettingsStartupCheck),
            startupReport.ready()
                ? i18n::tr(i18n::Str::SettingsStartupReady)
                : i18n::tr(i18n::Str::SettingsStartupAttention));
        ImGui::Text(
            i18n::tr(i18n::Str::SettingsStartupComponents),
            startupReport.administrator
                ? i18n::tr(i18n::Str::SettingsComponentOk)
                : i18n::tr(i18n::Str::SettingsComponentFail),
            startupReport.sdlRuntimePresent
                ? i18n::tr(i18n::Str::SettingsComponentOk)
                : i18n::tr(i18n::Str::SettingsComponentFail),
            startupReport.webRadarBundlePresent
                ? i18n::tr(i18n::Str::SettingsComponentOk)
                : i18n::tr(i18n::Str::SettingsComponentFail),
            startupReport.mapMetadataPresent
                ? i18n::tr(i18n::Str::SettingsComponentOk)
                : i18n::tr(i18n::Str::SettingsComponentFail));
        if (!startupReport.installationError.empty()) {
            ImGui::TextWrapped(
                "%s",
                startupReport.installationError.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
            "%s",
            i18n::tr(i18n::Str::SettingsProductName));
        ImGui::Text(
            "%s",
            i18n::tr(i18n::Str::SettingsProductStack));
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

    // The two languages each name themselves, so this control reads correctly
    // in either language and needs no translation of its own.
    inline void RenderLanguageChoices()
    {
        struct Choice
        {
            i18n::Language language;
            const char* label;
        };
        const Choice choices[] = {
            { i18n::Language::English, "English" },
            { i18n::Language::Chinese, "中文" }
        };

        int selectedIndex =
            i18n::currentLanguage == i18n::Language::Chinese ? 1 : 0;
        const char* labels[IM_ARRAYSIZE(choices)];
        for (int index = 0; index < IM_ARRAYSIZE(choices); ++index) {
            labels[index] = choices[index].label;
        }

        // "###" keeps the combo's ID fixed while the item list stays readable
        // in its own language.
        if (ImGui::Combo(
                i18n::trId(i18n::Str::LanguageSetting),
                &selectedIndex,
                labels,
                IM_ARRAYSIZE(labels))) {
            i18n::currentLanguage = choices[selectedIndex].language;
        }
    }

    // Two-state switch for the navigation column. The third element of each
    // label is a fixed ID, so the buttons keep their identity across a switch.
    inline void RenderLanguageSwitch(const ImVec2& size)
    {
        struct Choice
        {
            i18n::Language language;
            const char* label;
            const char* id;
        };
        const Choice choices[] = {
            { i18n::Language::English, "EN", "En" },
            { i18n::Language::Chinese, "中文", "Zh" }
        };

        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const ImVec2 buttonSize(
            (size.x - spacing * (IM_ARRAYSIZE(choices) - 1)) /
                static_cast<float>(IM_ARRAYSIZE(choices)),
            size.y);

        for (int index = 0; index < IM_ARRAYSIZE(choices); ++index) {
            const Choice& choice = choices[index];
            if (index > 0) {
                ImGui::SameLine();
            }

            const bool selected =
                i18n::currentLanguage == choice.language;
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                selected
                    ? ImVec4(0.075f, 0.330f, 0.470f, 1.0f)
                    : ImVec4(0.055f, 0.072f, 0.100f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                selected
                    ? ImVec4(0.085f, 0.390f, 0.540f, 1.0f)
                    : ImVec4(0.095f, 0.130f, 0.175f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                selected
                    ? ImVec4(0.100f, 0.440f, 0.600f, 1.0f)
                    : ImVec4(0.110f, 0.155f, 0.205f, 1.0f));

            char label[48];
            sprintf_s(
                label,
                "%s###UiLanguage%s",
                choice.label,
                choice.id);
            if (ImGui::Button(label, buttonSize) && !selected) {
                i18n::currentLanguage = choice.language;
            }
            ImGui::PopStyleColor(3);
        }
    }

    inline void RenderCombatPage()
    {
        RenderPageHeader(
            i18n::tr(i18n::Str::PageCombatTitle),
            i18n::tr(i18n::Str::PageCombatDescription));
        BeginCard(
            "##AimbotCard",
            i18n::tr(i18n::Str::CardAimbotTitle),
            i18n::tr(i18n::Str::CardAimbotSubtitle),
            590.0f);
        RenderAimbotTab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##TriggerCard",
            i18n::tr(i18n::Str::CardTriggerbotTitle),
            i18n::tr(i18n::Str::CardTriggerbotSubtitle),
            190.0f);
        RenderTriggerbotTab();
        EndCard();
    }

    inline void RenderPlayerVisualsPage()
    {
        RenderPageHeader(
            i18n::tr(i18n::Str::PagePlayerVisualsTitle),
            i18n::tr(i18n::Str::PagePlayerVisualsDescription));
        BeginCard(
            "##PlayerEspCard",
            i18n::tr(i18n::Str::CardPlayerEspTitle),
            i18n::tr(i18n::Str::CardPlayerEspSubtitle),
            650.0f);
        RenderESPTab();
        EndCard();
    }

    inline void RenderWorldPage()
    {
        RenderPageHeader(
            i18n::tr(i18n::Str::PageWorldTitle),
            i18n::tr(i18n::Str::PageWorldDescription));
        BeginCard(
            "##RadarCard",
            i18n::tr(i18n::Str::CardRadarTitle),
            i18n::tr(i18n::Str::CardRadarSubtitle),
            870.0f);
        RenderRadarTab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##WorldUtilityCard",
            i18n::tr(i18n::Str::CardWorldUtilityTitle),
            i18n::tr(i18n::Str::CardWorldUtilitySubtitle),
            255.0f);
        RenderMiscTab();
        EndCard();
    }

    inline void RenderSystemPage()
    {
        RenderPageHeader(
            i18n::tr(i18n::Str::PageSystemTitle),
            i18n::tr(i18n::Str::PageSystemDescription));
        BeginCard(
            "##LanguageCard",
            i18n::tr(i18n::Str::LanguageSetting),
            i18n::tr(i18n::Str::LanguageHint),
            105.0f);
        RenderLanguageChoices();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##DisplayCard",
            i18n::tr(i18n::Str::CardDisplayTitle),
            i18n::tr(i18n::Str::CardDisplaySubtitle),
            460.0f);
        RenderSettingsTab();
        EndCard();
        ImGui::Spacing();
        BeginCard(
            "##HotkeyCard",
            i18n::tr(i18n::Str::CardHotkeysTitle),
            i18n::tr(i18n::Str::CardHotkeysSubtitle),
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

        // The title is a product name and stays untranslated, which also keeps
        // the window's imgui.ini entry -- and so the saved size and position --
        // stable across a language switch.
        ImGui::Begin(
            "Aegis // CS2 Overlay",
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
            "CS2 OVERLAY");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float contentWidth = ImGui::GetContentRegionAvail().x;
        RenderLanguageSwitch(ImVec2(contentWidth, 28.0f * dpiScale));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const ImVec2 navigationSize(
            ImGui::GetContentRegionAvail().x,
            navigationHeight);
        NavigationButton(
            i18n::trId(i18n::Str::NavCombat),
            0,
            navigationSize);
        NavigationButton(
            i18n::trId(i18n::Str::NavPlayerVisuals),
            1,
            navigationSize);
        NavigationButton(
            i18n::trId(i18n::Str::NavWorldRadar),
            2,
            navigationSize);
        NavigationButton(
            i18n::trId(i18n::Str::NavSystem),
            3,
            navigationSize);
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
