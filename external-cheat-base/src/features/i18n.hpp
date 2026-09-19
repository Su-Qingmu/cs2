#pragma once

// Localization for the overlay menu.
//
// One X-macro list is the single source of truth: it generates the Str enum,
// the English table, the Chinese table and the language-stable ID table. A
// string therefore cannot be added without both texts, and two constexpr
// checks fail the build when a translation is empty or when it changes the
// printf placeholders the surrounding ImGui call passes.
//
// Texts are UTF-8. The project compiles with /utf-8 (see the ClCompile
// AdditionalOptions in external-cheat-base.vcxproj), so these literals reach
// ImGui -- which expects UTF-8 throughout -- byte for byte.

#include <Windows.h>

#include <cstddef>
#include <string_view>

namespace i18n
{
    enum class Language
    {
        English,
        Chinese
    };

    // X(ID, English text, Chinese text)
    //
    // The ID becomes i18n::Str::<ID>. It is also the widget ID suffix, so it
    // must stay unique: ImGui derives a widget's identity from its label, and
    // two identical labels in the same window already need distinct IDs.
    //
    // Plain product, protocol and unit names are deliberately not translated:
    // Aegis, CS2, SDL2, ImGui, CivetWeb, DPI, Steam ID, P95/P99, URLs, and the
    // keycap names returned by menu::GetKeyName() -- every keyboard prints
    // "Shift" and "Backspace" in English regardless of the Windows display
    // language.
    // clang-format off
#define AEGIS_UI_STRINGS(X)                                                     \
    /* ---- Navigation and language ---- */                                     \
    X(NavCombat, "Combat", "战斗")                                              \
    X(NavPlayerVisuals, "Player visuals", "玩家透视")                            \
    X(NavWorldRadar, "World & Radar", "世界与雷达")                              \
    X(NavSystem, "System", "系统")                                              \
    X(LanguageSetting, "Language", "界面语言")                                   \
    X(LanguageHint,                                                             \
      "Applies immediately and is saved when the overlay exits.",               \
      "立即生效，退出覆盖层时保存。")                                             \
                                                                                \
    /* ---- Combat page ---- */                                                 \
    X(PageCombatTitle, "Combat assistance", "战斗辅助")                          \
    X(PageCombatDescription,                                                    \
      "Target selection and input automation. All input is focus-gated.",       \
      "目标选择与输入自动化。所有输入仅在游戏获得焦点时生效。")                      \
    X(CardAimbotTitle, "Aimbot", "自瞄")                                        \
    X(CardAimbotSubtitle,                                                       \
      "Real bone targets with stable target retention.",                        \
      "锁定真实骨骼，并保持目标稳定不跳变。")                                      \
    X(CardTriggerbotTitle, "Triggerbot", "自动开火")                             \
    X(CardTriggerbotSubtitle,                                                   \
      "Uses the actual entity under the crosshair; no angular guessing.",       \
      "以准星下真实命中的实体为准，不做角度猜测。")                                 \
                                                                                \
    X(AimbotEnable, "Enable Aimbot", "启用自瞄")                                 \
    X(AimbotSmartAim, "Smart Aim (Auto-Lock)", "智能自瞄（自动锁定）")            \
    X(AimbotSmartAimTooltip,                                                    \
      "Ignore FOV, auto-aim at best spotted target\nPriority: Spotted > Distance/Health", \
      "忽略视场角，自动瞄准已发现目标中最优的一个\n优先级：已发现 > 距离/生命值")     \
    X(AimbotPriority, "Priority", "优先级")                                      \
    X(AimbotPriorityDistance, "Distance First", "距离优先")                      \
    X(AimbotPriorityHealth, "Health First", "生命值优先")                        \
    X(AimbotPriorityTooltip,                                                    \
      "Distance: Aim at closest enemy\nHealth: Aim at lowest HP enemy",         \
      "距离：瞄准最近的敌人\n生命值：瞄准血量最低的敌人")                            \
    X(AimbotFov, "FOV", "视场角")                                               \
    X(AimbotFovFormat, "%.1f deg", "%.1f 度")                                   \
    X(AimbotFovTooltip,                                                         \
      "Field of view - only aim at enemies within this angle",                  \
      "视场角——只瞄准处在该角度范围内的敌人")                                       \
    X(AimbotSmoothing, "Aim Smoothing", "瞄准平滑")                              \
    X(AimbotSmoothingTooltip,                                                   \
      "1.0 = instant lock, higher = smoother/slower",                           \
      "1.0 为瞬间锁定，数值越大越平滑、越慢")                                       \
    X(AimbotTargetBone, "Target Bone", "瞄准骨骼")                               \
    X(AimbotBoneHead, "Head", "头部")                                            \
    X(AimbotBoneNeck, "Neck", "颈部")                                            \
    X(AimbotBoneChest, "Chest", "胸部")                                          \
    X(AimbotSpottedOnly, "Spotted Only", "仅限已发现目标")                        \
    X(AimbotSpottedOnlyTooltip,                                                 \
      "Uses CS2's spotted flag; this is not a geometric ray-cast",              \
      "使用 CS2 的「已发现」标记，并非几何射线检测")                                \
    X(AimbotShowFovCircle, "Show FOV Circle", "显示视场角圆圈")                  \
    X(AimbotHeadOffset, "Head Offset", "头部偏移")                               \
    X(AimbotHeadOffsetEnable, "Enable (Side-facing)", "启用（侧身时）")           \
    X(AimbotHeadOffsetEnableTooltip,                                            \
      "Compensate for head position when enemy is facing sideways",             \
      "敌人侧身时补偿头部位置")                                                    \
    X(AimbotHeadOffsetAmount, "Offset Amount", "偏移量")                         \
    X(AimbotHeadOffsetAmountFormat, "%.1f units", "%.1f 单位")                   \
    X(AimbotHeadOffsetAmountTooltip,                                            \
      "How much to offset the head position (5-8 recommended)",                 \
      "头部位置的偏移量（建议 5-8）")                                              \
    X(AimbotHeadOffsetMinAngle, "Min Angle", "最小角度")                         \
    X(AimbotHeadOffsetMaxAngle, "Max Angle", "最大角度")                         \
    X(AimbotHeadOffsetAngleFormat, "%.0f deg", "%.0f 度")                       \
    X(AimbotHeadOffsetAngleTooltip,                                             \
      "Angle range for offset (45-135 = side-facing)",                          \
      "触发偏移的角度范围（45-135 为侧身）")                                        \
    X(AimbotHeadOffsetAngleLegend,                                              \
      "0=facing you, 90=side, 180=back",                                        \
      "0=正对你，90=侧身，180=背对")                                              \
    X(AimbotInput, "Input", "输入")                                             \
    X(AimbotMouseSensitivity, "Mouse Sensitivity", "鼠标灵敏度")                 \
    X(AimbotMouseSensitivityFormat, "%.2f", "%.2f")                             \
    X(AimbotMouseSensitivityTooltip,                                            \
      "Match your in-game mouse sensitivity",                                   \
      "请与游戏内的鼠标灵敏度保持一致")                                             \
    X(AimbotHoldToAim, "Hold %s to aim", "按住 %s 自瞄")                          \
                                                                                \
    X(TriggerbotEnable, "Enable Triggerbot", "启用自动开火")                     \
    X(TriggerbotDelay, "Delay (ms)", "延迟（毫秒）")                             \
    X(TriggerbotDelayFormat, "%d ms", "%d 毫秒")                                \
    X(TriggerbotDelayTooltip,                                                   \
      "Delay before shooting (milliseconds)",                                   \
      "开火前的延迟（毫秒）")                                                      \
    X(TriggerbotHoldToActivate, "Hold %s to activate", "按住 %s 生效")            \
    X(TriggerbotDescription,                                                    \
      "Fires only when the crosshair is on a live enemy.",                      \
      "仅当准星位于存活敌人身上时开火。")                                           \
                                                                                \
    /* ---- Player visuals page ---- */                                         \
    X(PagePlayerVisualsTitle, "Player visuals", "玩家透视")                      \
    X(PagePlayerVisualsDescription,                                             \
      "Configure information drawn around validated live enemy pawns.",         \
      "配置绘制在已校验的存活敌人身上的信息。")                                      \
    X(CardPlayerEspTitle, "Player ESP", "玩家透视")                              \
    X(CardPlayerEspSubtitle,                                                    \
      "Boxes, health, skeleton, equipment and threat direction.",               \
      "方框、生命值、骨骼、装备与威胁朝向。")                                        \
                                                                                \
    X(EspEnable, "Enable ESP", "启用透视")                                       \
    X(EspEnableHint, "Left Alt toggles ESP", "左 Alt 键切换透视")                 \
    X(EspBox, "Box ESP", "方框透视")                                             \
    X(EspHealth, "Health Bar", "生命值条")                                       \
    X(EspWeapon, "Weapon", "武器")                                               \
    X(EspViewDirection, "View Direction (Box Color)", "朝向（影响方框颜色）")      \
    X(EspFacingYou, "Facing You: RED", "正对你：红色")                            \
    X(EspPartial, "Partial: ORANGE", "部分朝向：橙色")                            \
    X(EspSide, "Side: YELLOW", "侧身：黄色")                                      \
    X(EspBack, "Back: GREEN", "背对：绿色")                                       \
    X(EspShowAngleDegrees, "Show Angle Degrees", "显示角度数值")                  \
    X(EspSpottedCheck, "Spotted Check (Triangle)", "已发现检测（三角标记）")       \
    X(EspSpottedColor, "Spotted Color:", "已发现颜色：")                          \
    X(EspNotSpottedColor, "Not Spotted / Unknown:", "未发现 / 未知：")            \
    X(EspSpottedCheckNote,                                                      \
      "Uses CS2 spotted state; never assumes distant targets visible.",         \
      "使用 CS2 的「已发现」状态，绝不假设远处目标可见。")                           \
    X(EspDistance, "Distance", "距离")                                           \
    X(EspFlashIndicator, "Flashbang Eye Indicator", "闪光弹眼部指示")             \
    X(EspFlashNormalEye, "Normal Eye:", "正常眼睛：")                            \
    X(EspFlashedEye, "Flashed Eye:", "被闪眼睛：")                               \
    X(EspSnaplines, "Snaplines", "连线")                                         \
    X(EspSnaplinesOrigin, "Origin", "起点")                                      \
    X(EspOriginBottom, "Bottom", "底部")                                         \
    X(EspOriginCenter, "Center", "中心")                                         \
    X(EspOriginTop, "Top", "顶部")                                               \
    X(EspSkeleton, "Skeleton", "骨骼")                                           \
                                                                                \
    /* ---- World and Radar page ---- */                                        \
    X(PageWorldTitle, "World and match", "世界与对局")                            \
    X(PageWorldDescription,                                                     \
      "Shared Web Radar, bomb state and moving world entities.",                \
      "共享网页雷达、炸弹状态与移动中的世界实体。")                                  \
    X(CardRadarTitle, "Fixed-map Radar", "固定地图雷达")                          \
    X(CardRadarSubtitle,                                                        \
      "One north-up map model for the local overlay, CivetWeb and Relay.",      \
      "本地覆盖层、CivetWeb 与中继共用同一套正北朝上地图模型。")                      \
    X(CardWorldUtilityTitle, "Match utilities", "对局辅助")                       \
    X(CardWorldUtilitySubtitle,                                                 \
      "Bomb timer, projectiles, dropped equipment and anti-flash.",             \
      "炸弹计时、投掷物、掉落装备与防闪光。")                                        \
                                                                                \
    X(RadarLocalHeading, "LOCAL FIXED-MAP OVERLAY", "本地固定地图覆盖层")          \
    X(RadarLocalDescription,                                                    \
      "Draws the complete north-up map inside the game overlay. It uses the "   \
      "same images and calibration as Web Radar; only player direction "        \
      "markers rotate.",                                                        \
      "在游戏覆盖层内绘制完整的正北朝上地图。所用图片与校准和网页雷达完全一致，"      \
      "只有玩家方向标记会旋转。")                                                  \
    X(RadarLocalEnable, "Enable local map overlay", "启用本地地图覆盖层")          \
    X(RadarLocalShowNames, "Show player names", "显示玩家名称")                   \
    X(RadarLocalAnchorX, "Horizontal position", "水平位置")                       \
    X(RadarLocalAnchorY, "Vertical position", "垂直位置")                         \
    X(RadarLocalSize, "Map size", "地图大小")                                     \
    X(RadarLocalMarkerSize, "Player marker size", "玩家标记大小")                 \
    X(RadarLocalMarkerSizeFormat, "%.0f px", "%.0f 像素")                        \
                                                                                \
    X(RadarWebHeading, "EMBEDDED BROWSER RADAR", "内置网页雷达")                   \
    X(RadarWebDescription,                                                      \
      "Serves the same fixed map through the embedded CivetWeb service for a "  \
      "local browser or trusted LAN viewers.",                                  \
      "通过内置 CivetWeb 服务，把同一份固定地图提供给本机浏览器或可信的局域网观众。")  \
    X(RadarWebEnable, "Enable Web Radar", "启用网页雷达")                         \
    X(RadarWebPort, "HTTP port", "HTTP 端口")                                    \
    X(RadarWebLanAccess, "Allow viewers on this LAN", "允许局域网内观众访问")      \
    X(RadarWebPauseWhenUnfocused,                                               \
      "Pause browser/Relay sampling when CS2 loses focus",                      \
      "CS2 失去焦点时暂停浏览器/中继采样")                                          \
    X(RadarWebPauseWhenUnfocusedNote,                                           \
      "The local overlay always pauses when CS2 is unfocused; only explicitly " \
      "shared viewers can opt into background sampling.",                       \
      "CS2 失去焦点时本地覆盖层总会暂停；只有显式共享出去的观众才能选择后台采样。")     \
    X(RadarWebShareNames, "Share player names", "共享玩家名称")                   \
    X(RadarWebSharedTeams, "Shared teams", "共享队伍")                            \
    X(RadarWebTeamAll, "All teams", "全部队伍")                                   \
    X(RadarWebTeamLocal, "Local team only", "仅本方队伍")                         \
    X(RadarWebTeamOpponents, "Opponents only", "仅对手")                          \
    X(RadarWebShareSteamIds, "Share Steam IDs (profile links)", "共享 Steam ID（个人资料链接）") \
    X(RadarWebLanMode, "LAN MODE", "局域网模式")                                  \
    X(RadarWebLanModeWarning,                                                   \
      "Anyone who receives the tokenized URL can view the stream. Only use it " \
      "on a trusted private network; do not expose the port to the internet.",  \
      "任何拿到带令牌链接的人都能查看该数据流。只可在可信的私有网络中使用，"        \
      "切勿把端口暴露到公网。")                                                     \
    X(RadarWebService, "Service: %s", "服务：%s")                                 \
    X(RadarWebServiceRunning, "RUNNING", "运行中")                                \
    X(RadarWebServiceStopped, "STOPPED", "已停止")                                \
    X(RadarWebBind, "Bind: %s:%d", "监听：%s:%d")                                 \
    X(RadarWebViewers, "Viewers: %zu", "观众：%zu")                               \
    X(RadarWebFrames,                                                           \
      "Frames: %llu published | %llu sent | %llu replaced",                     \
      "帧：已发布 %llu | 已发送 %llu | 已替换 %llu")                               \
    X(RadarWebTraffic,                                                          \
      "Traffic: %.1f MB | max send latency %.1f ms",                            \
      "流量：%.1f MB | 最大发送延迟 %.1f 毫秒")                                    \
    X(RadarWebError, "Error: %s", "错误：%s")                                     \
    X(RadarWebOpen, "Open Radar", "打开雷达")                                     \
    X(RadarWebCopyUrl, "Copy viewer URL", "复制观众链接")                         \
    X(RadarWebLanUrlHint,                                                       \
      "For another device, replace 127.0.0.1 in this URL with this PC's "       \
      "private LAN IPv4 address.",                                              \
      "若要在其他设备上访问，请把该链接中的 127.0.0.1 换成本机在局域网内的 IPv4 地址。") \
    X(RadarRecordEnable, "Record sanitized Radar snapshots", "录制脱敏后的雷达快照") \
    X(RadarRecordProgress,                                                      \
      "Recording: %llu frames (%.1f MB), %llu replaced",                        \
      "录制中：%llu 帧（%.1f MB），已替换 %llu")                                   \
    X(RadarRecordFile, "File: %s", "文件：%s")                                    \
                                                                                \
    X(RelayHeading, "PUBLIC RELAY (OUTBOUND WSS)", "公网中继（出站 WSS）")          \
    X(RelayDescription,                                                         \
      "Publishes snapshots through an authenticated, TLS-protected outbound "   \
      "connection. No inbound port or LAN mode is required.",                   \
      "通过经过身份验证、TLS 保护的出站连接发布快照，无需开放入站端口或启用局域网模式。") \
    X(RelayEnable, "Enable Public Relay", "启用公网中继")                         \
    X(RelayUrl, "Relay WSS URL", "中继 WSS 地址")                                 \
    X(RelayUrlHint, "wss://radar.example.com/api/v1/publish", "wss://radar.example.com/api/v1/publish") \
    X(RelayRoom, "Relay room", "中继房间")                                        \
    X(RelayRoomHint, "match-room", "match-room")                                 \
    X(RelayToken, "Producer token", "发布者令牌")                                 \
    X(RelayTokenHint, "Paste the producer-only token", "粘贴仅用于发布的令牌")      \
    X(RelayClear, "Clear Relay credentials", "清除中继凭据")                       \
    X(RelayShareNames,                                                          \
      "Share player names through Public Relay",                                \
      "通过公网中继共享玩家名称")                                                   \
    X(RelayTeams, "Relay teams", "中继队伍")                                      \
    X(RelayShareSteamIds,                                                       \
      "Share Steam IDs through Public Relay",                                   \
      "通过公网中继共享 Steam ID")                                                 \
    X(RelayTokenNote,                                                           \
      "The producer token is kept in memory only and is never shown in status " \
      "or logs. Use a producer token, never a viewer token.",                   \
      "发布者令牌只保留在内存中，绝不会出现在状态或日志里。请使用发布者令牌，"      \
      "不要使用观众令牌。")                                                         \
    X(RelayState, "Relay: %s", "中继：%s")                                        \
    X(RelayStateDisabled, "DISABLED", "已禁用")                                   \
    X(RelayStateConnecting, "CONNECTING", "连接中")                               \
    X(RelayStateConnected, "CONNECTED", "已连接")                                 \
    X(RelayStateBackoff, "RETRY BACKOFF", "重试退避中")                           \
    X(RelayStateStopping, "STOPPING", "正在停止")                                 \
    X(RelayStateFailed, "FAILED", "失败")                                         \
    X(RelayCounters,                                                            \
      "Frames sent: %llu  |  Replaced: %llu  |  Dropped: %llu  |  Reconnects: %llu", \
      "已发送帧：%llu  |  已替换：%llu  |  已丢弃：%llu  |  重连：%llu")            \
    X(RelayError, "Relay error: %s", "中继错误：%s")                              \
                                                                                \
    /* ---- System page ---- */                                                 \
    X(PageSystemTitle, "System", "系统")                                          \
    X(PageSystemDescription,                                                    \
      "Display mapping, performance diagnostics and key bindings.",             \
      "显示映射、性能诊断与快捷键绑定。")                                            \
    X(CardDisplayTitle, "Display and renderer", "显示与渲染")                     \
    X(CardDisplaySubtitle,                                                      \
      "Monitor-aware viewport mapping and live diagnostics.",                   \
      "感知显示器的视口映射与实时诊断。")                                            \
    X(CardHotkeysTitle, "Hotkeys", "快捷键")                                      \
    X(CardHotkeysSubtitle,                                                      \
      "Bindings must be unique; input pauses while rebinding.",                 \
      "各绑定必须互不冲突；重新绑定期间输入会暂停。")                                 \
                                                                                \
    X(SettingsPerformance, "Performance", "性能")                                 \
    X(SettingsOverlayTarget,                                                    \
      "Overlay target: %d FPS (%s)",                                            \
      "覆盖层目标：%d 帧率（%s）")                                                 \
    X(SettingsVsync, "VSync", "垂直同步")                                         \
    X(SettingsPacedFallback, "paced fallback", "定时回退")                        \
    X(SettingsRenderer, "Renderer: %s", "渲染器：%s")                             \
    X(SettingsRendererHardware, "Hardware accelerated", "硬件加速")               \
    X(SettingsRendererSoftware, "Software fallback (limited to 60 FPS)", "软件回退（限制 60 帧率）") \
    X(SettingsSingleMonitorWarning,                                             \
      "Move CS2 fully onto one monitor for reliable mixed-DPI mapping.",         \
      "请把 CS2 完全置于单个显示器内，以保证混合 DPI 下的映射可靠。")                 \
    X(SettingsDpiWarning, "Per-monitor DPI awareness is unavailable.", "当前无法使用逐显示器 DPI 感知。") \
    X(SettingsViewportMode, "Game Viewport", "游戏视口")                          \
    X(SettingsViewportAuto, "Auto-detect black bars", "自动检测黑边")             \
    X(SettingsViewportFullClient, "Full client (stretched)", "完整客户区（拉伸）") \
    X(SettingsViewportForce43, "Force 4:3 black bars", "强制 4:3 黑边")           \
    X(SettingsViewportForce1610, "Force 16:10 black bars", "强制 16:10 黑边")     \
    X(SettingsViewportTooltip,                                                  \
      "Auto is recommended. Use a forced mode only if a capture-protected or "  \
      "very dark scene prevents black-bar detection.",                          \
      "推荐使用自动。只有在受保护内容或极暗场景导致黑边检测失败时，才使用强制模式。")   \
    X(SettingsSystemInfo, "System Info", "系统信息")                              \
    X(SettingsResolution, "Resolution: %dx%d", "分辨率：%dx%d")                   \
    X(SettingsGameViewport,                                                     \
      "Game viewport: %dx%d at (%d, %d)",                                       \
      "游戏视口：%dx%d，位于 (%d, %d)")                                           \
    X(SettingsFps, "FPS: %.1f", "帧率：%.1f")                                     \
    X(SettingsFrameTime, "Frame Time: %.3f ms", "帧时间：%.3f 毫秒")               \
    X(SettingsSampling,                                                         \
      "Sampling: %d Hz | Radar: %d Hz | avg %.2f ms | P95 %.2f | P99 %.2f",     \
      "采样：%d Hz | 雷达：%d Hz | 平均 %.2f 毫秒 | P95 %.2f | P99 %.2f")          \
    X(SettingsRenderCpu,                                                        \
      "Render CPU: avg %.2f ms | P95 %.2f | P99 %.2f",                          \
      "渲染 CPU：平均 %.2f 毫秒 | P95 %.2f | P99 %.2f")                           \
    X(SettingsRadarJson,                                                        \
      "Radar JSON: avg %.2f ms | P95 %.2f | max %.2f",                          \
      "雷达 JSON：平均 %.2f 毫秒 | P95 %.2f | 最大 %.2f")                          \
    X(SettingsRpm,                                                              \
      "RPM: %llu calls | %.1f MB | %llu failed",                                \
      "读内存：%llu 次调用 | %.1f MB | %llu 次失败")                               \
    X(SettingsMissedDeadlines,                                                  \
      "Missed deadlines: sample %llu | render %llu",                            \
      "超时次数：采样 %llu | 渲染 %llu")                                           \
    X(SettingsStartupCheck, "Startup self-check: %s", "启动自检：%s")              \
    X(SettingsStartupReady, "READY", "就绪")                                      \
    X(SettingsStartupAttention, "ATTENTION", "需要关注")                          \
    X(SettingsStartupComponents,                                                \
      "Admin %s | SDL %s | Web bundle %s | Maps %s",                            \
      "管理员 %s | SDL %s | 网页包 %s | 地图 %s")                                 \
    X(SettingsComponentOk, "OK", "正常")                                          \
    X(SettingsComponentFail, "FAIL", "失败")                                      \
    X(SettingsProductName, "CS2 External ESP v2.0", "CS2 外部透视 v2.0")           \
    X(SettingsProductStack, "SDL2 + ImGui Overlay", "SDL2 + ImGui 覆盖层")        \
                                                                                \
    /* ---- Hotkeys ---- */                                                     \
    X(HotkeysHeading, "Key Bindings", "按键绑定")                                 \
    X(HotkeyMenuToggle, "Menu Toggle", "菜单开关")                                \
    X(HotkeyMenuToggleTooltip, "Key to show/hide menu", "显示/隐藏菜单的按键")     \
    X(HotkeyExitProgram, "Exit Program", "退出程序")                              \
    X(HotkeyExitProgramTooltip, "Key to exit the program", "退出程序的按键")       \
    X(HotkeyAimbotKey, "Aimbot Key", "自瞄按键")                                  \
    X(HotkeyAimbotKeyTooltip, "Hold to activate aimbot", "按住以启用自瞄")         \
    X(HotkeyTriggerbotKey, "Triggerbot Key", "自动开火按键")                      \
    X(HotkeyTriggerbotKeyTooltip, "Hold to activate triggerbot", "按住以启用自动开火") \
    X(HotkeyPressKey, "[Press Key...]", "[请按键...]")                            \
    X(HotkeyBindHint, "Click button and press any key to bind", "点击按钮后按下任意键即可绑定") \
    X(HotkeyCancelHint, "Press ESC to cancel binding", "按 ESC 取消绑定")          \
    X(HotkeyConflictPrefix, "Already assigned to ", "已被占用：")                  \
                                                                                \
    /* ---- Match utilities ---- */                                             \
    X(MiscHeading, "Misc Features", "其他功能")                                   \
    X(MiscAntiFlash, "Anti-Flash", "防闪光")                                      \
    X(MiscMemoryWritesLocked,                                                   \
      "Memory writes locked. Start with --allow-memory-writes to enable.",      \
      "内存写入已锁定。使用 --allow-memory-writes 启动以启用。")                    \
    X(MiscBombTimer, "Bomb Timer", "炸弹计时")                                    \
    X(MiscWorldEsp, "World ESP", "世界透视")                                      \
    X(MiscGrenadeEsp, "Grenade ESP", "投掷物透视")                                \
    X(MiscDroppedWeaponEsp, "Dropped Weapon ESP", "掉落武器透视")                  \
                                                                                \
    /* ---- Waiting screen (drawn from main.cpp) ---- */                        \
    X(WaitingTitle, "Waiting for Counter-Strike 2%s", "正在等待 Counter-Strike 2%s") \
    X(WaitingDescription,                                                       \
      "The client and its active monitor will be detected automatically.",      \
      "将自动检测游戏客户端及其所在的显示器。")                                      \
    X(WaitingDisplayMode, "DISPLAY MODE", "显示模式")                             \
    X(WaitingDisplayModeHint,                                                   \
      "Use Fullscreen Windowed in CS2. The overlay will then map the game "     \
      "viewport to the correct monitor and aspect ratio.",                      \
      "请在 CS2 中使用「全屏窗口化」。覆盖层随后会把游戏视口映射到正确的显示器和宽高比。") \
    X(WaitingFooter,                                                            \
      "Checking every 3 seconds  |  Press F9 to exit",                          \
      "每 3 秒检测一次  |  按 F9 退出")
    // clang-format on

    enum class Str
    {
#define AEGIS_UI_ENUM_ENTRY(id, english, chinese) id,
        AEGIS_UI_STRINGS(AEGIS_UI_ENUM_ENTRY)
#undef AEGIS_UI_ENUM_ENTRY
        Count
    };

    inline constexpr std::size_t kStringCount =
        static_cast<std::size_t>(Str::Count);

    inline constexpr const char* kEnglish[] = {
#define AEGIS_UI_ENGLISH_ENTRY(id, english, chinese) english,
        AEGIS_UI_STRINGS(AEGIS_UI_ENGLISH_ENTRY)
#undef AEGIS_UI_ENGLISH_ENTRY
    };

    inline constexpr const char* kChinese[] = {
#define AEGIS_UI_CHINESE_ENTRY(id, english, chinese) chinese,
        AEGIS_UI_STRINGS(AEGIS_UI_CHINESE_ENTRY)
#undef AEGIS_UI_CHINESE_ENTRY
    };

    // Chinese text followed by "###" and the English text. ImGui stops drawing
    // at the first "##" and starts hashing at "###", so this displays the
    // Chinese label while hashing to exactly the same ID the English label
    // would produce. A widget therefore keeps its identity across a language
    // switch instead of losing focus, drag state or combo popup.
    inline constexpr const char* kChineseIds[] = {
#define AEGIS_UI_CHINESE_ID_ENTRY(id, english, chinese) chinese "###" english,
        AEGIS_UI_STRINGS(AEGIS_UI_CHINESE_ID_ENTRY)
#undef AEGIS_UI_CHINESE_ID_ENTRY
    };

    // --- Compile-time table checks -------------------------------------------

    constexpr bool allTranslated()
    {
        for (std::size_t index = 0; index < kStringCount; ++index)
        {
            if (kEnglish[index][0] == '\0' || kChinese[index][0] == '\0')
            {
                return false;
            }
        }
        return true;
    }

    constexpr bool isConversionChar(const char character)
    {
        switch (character)
        {
        case 'd': case 'i': case 'u': case 'o': case 'x': case 'X':
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
        case 'a': case 'A': case 'c': case 's': case 'p':
            return true;
        default:
            return false;
        }
    }

    // Index just past the conversion character of the specifier starting at
    // the '%' at `start`. An unterminated specifier ends at the string end.
    constexpr std::size_t specifierEnd(
        const std::string_view text,
        const std::size_t start)
    {
        std::size_t index = start + 1;
        while (index < text.size() && !isConversionChar(text[index]))
        {
            ++index;
        }
        return index < text.size() ? index + 1 : index;
    }

    // A translation may reorder words around a placeholder but must not alter
    // the placeholder itself. "%llu" that becomes "%d" reads a 32-bit value out
    // of a 64-bit argument, and "%s" that becomes "%d" prints a pointer as an
    // integer; both are silent corruption at runtime.
    //
    // This is the only real guard available: IM_FMTARGS is a no-op on MSVC
    // (imgui.h), and a compiler cannot type-check ImGui::Text(tr(Str::X), ...)
    // because the format is no longer a literal.
    constexpr bool placeholdersMatch(
        const std::string_view english,
        const std::string_view chinese)
    {
        std::size_t left = 0;
        std::size_t right = 0;
        while (true)
        {
            while (left < english.size() && english[left] != '%')
            {
                ++left;
            }
            while (right < chinese.size() && chinese[right] != '%')
            {
                ++right;
            }

            const bool englishDone = left >= english.size();
            const bool chineseDone = right >= chinese.size();
            if (englishDone || chineseDone)
            {
                return englishDone == chineseDone;
            }

            // "%%" is a literal percent sign, not a conversion.
            const bool englishEscaped =
                left + 1 < english.size() && english[left + 1] == '%';
            const bool chineseEscaped =
                right + 1 < chinese.size() && chinese[right + 1] == '%';
            if (englishEscaped || chineseEscaped)
            {
                if (englishEscaped != chineseEscaped)
                {
                    return false;
                }
                left += 2;
                right += 2;
                continue;
            }

            const std::size_t englishEnd = specifierEnd(english, left);
            const std::size_t chineseEnd = specifierEnd(chinese, right);
            if (englishEnd - left != chineseEnd - right)
            {
                return false;
            }
            for (std::size_t offset = 0; offset < englishEnd - left; ++offset)
            {
                if (english[left + offset] != chinese[right + offset])
                {
                    return false;
                }
            }
            left = englishEnd;
            right = chineseEnd;
        }
    }

    constexpr bool allPlaceholdersMatch()
    {
        for (std::size_t index = 0; index < kStringCount; ++index)
        {
            if (!placeholdersMatch(kEnglish[index], kChinese[index]))
            {
                return false;
            }
        }
        return true;
    }

    static_assert(
        allTranslated(),
        "every entry in AEGIS_UI_STRINGS needs both an English and a Chinese text");
    static_assert(
        allPlaceholdersMatch(),
        "a Chinese translation changed a printf placeholder: keep every % specifier "
        "byte-identical to the English text");

    // --- Active language ------------------------------------------------------

    // Constant-initialized so that any read is valid regardless of static
    // initialization order. applySystemDefault() replaces it at startup, and
    // loadPersistentSettings() may replace it again from disk.
    inline Language currentLanguage = Language::English;

    // The Windows display language, not the user locale: someone running an
    // English Windows with a Chinese regional format still expects English.
    // Every Chinese sublanguage maps here, so a zh-TW install also gets a
    // Chinese menu rather than falling back to English.
    inline Language detectSystemLanguage()
    {
        return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_CHINESE
            ? Language::Chinese
            : Language::English;
    }

    inline void applySystemDefault()
    {
        currentLanguage = detectSystemLanguage();
    }

    inline const char* tr(const Str id)
    {
        const std::size_t index = static_cast<std::size_t>(id);
        return currentLanguage == Language::Chinese
            ? kChinese[index]
            : kEnglish[index];
    }

    // Use for any widget label ImGui turns into an ID (Checkbox, Button,
    // SliderFloat, Combo, InputText, Begin). Plain text calls -- Text,
    // TextColored, TextWrapped, TextDisabled, SetTooltip -- take tr() instead.
    inline const char* trId(const Str id)
    {
        const std::size_t index = static_cast<std::size_t>(id);
        return currentLanguage == Language::Chinese
            ? kChineseIds[index]
            : kEnglish[index];
    }

    // The untranslated text, for the few places that need a language-independent
    // string built at runtime (the "##" suffix of a composed widget label).
    inline const char* trStable(const Str id)
    {
        return kEnglish[static_cast<std::size_t>(id)];
    }

    // --- Persistence ----------------------------------------------------------

    inline const wchar_t* persistValue(const Language language)
    {
        return language == Language::Chinese ? L"zh" : L"en";
    }

    // An absent or unrecognized value follows the system language, so a first
    // run and an upgrade both land on the right default.
    inline Language parsePersisted(const std::wstring_view text)
    {
        if (text == L"zh")
        {
            return Language::Chinese;
        }
        if (text == L"en")
        {
            return Language::English;
        }
        return detectSystemLanguage();
    }
}

#undef AEGIS_UI_STRINGS
