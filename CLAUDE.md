# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Summary

`cs2_cheat` is an external CS2 ESP/overlay teaching project. It is a Windows x64 C++20 program that reads CS2's `client.dll` from another process (no module injection — uses `ReadProcessMemory`) and renders an SDL2 + ImGui overlay on top of the game. It also bundles a static React/Vite Web Radar served by an embedded CivetWeb HTTP/WebSocket server.

The runtime ships **only ESP, the in-game local radar, and the embedded Web Radar**. The aimbot, triggerbot, public `wss://` relay producer, and NDJSON snapshot recorder were removed; `memory::gWritesAllowed`, `webRadarEnabled`, `localRadarEnabled`, and `antiFlash` all default to `true`. The user can rebind a single ESP activation hotkey (default left Alt) with two modes: tap-to-toggle or hold-to-show.

This project is **for learning reverse engineering and graphics rendering only** — only run with CS2 launched with `-insecure` in offline mode, never on VAC servers. Administrator elevation is required at runtime (the binary carries `requireAdministrator` in its manifest).

## Repository Layout

```
cs2_cheat/
├── external-cheat-base/        # Windows C++ overlay executable (SDL2 + ImGui + CivetWeb)
│   ├── src/                    # main.cpp, features/, core/, utils/
│   ├── generated/              # offsets.hpp / client_dll.hpp / buttons.hpp (downloaded from cs2-dumper)
│   ├── vendor/                 # bundled SDL2 + ImGui source
│   └── external-cheat-base.vcxproj
├── web-radar/                  # React 19 + Vite + TypeScript radar frontend (npm project)
├── radar-relay/                # Go public-relay service (single-node, in-memory) — for users deploying their own relay
├── deploy/public-relay/        # Caddy + docker-compose for public radar deployment
├── tests/                      # Cross-platform C++ regression tests (g++)
├── vendor/civetweb/            # Third-party C/C++ HTTP/WS server
├── docs/                       # Reverse-engineering notes, 3D projection math, design docs
├── scripts/                    # generate_fixed_map_catalog.py, sync_web_radar_maps.py
├── Dockerfile                  # Multi-stage reproducible build (Node → Ubuntu+g++ → artifacts)
└── external-cheat-base.sln
```

## Build Commands

### Local Windows build (Visual Studio 2022, MSBuild)

The MSBuild post-build step copies `web-radar/dist` into the output directory, so build the frontend first:

```powershell
cd web-radar
npm ci
npm test          # vitest
npm run build     # tsc -b && vite build
cd ..

msbuild external-cheat-base.sln /p:Configuration=Release /p:Platform=x64
```

The output `x64/Release/` must contain `external-cheat-base.exe`, `SDL2.dll`, `CIVETWEB-LICENSE.md`, and `web-radar/dist/`. The MSBuild post-build step refuses to build unless `web-radar/dist/index.html` exists and validates the bundle.

### Reproducible Docker build (Linux + MinGW, no Windows host needed)

```bash
docker build --target artifacts -t cs2-cheat-build .
# artifacts stage writes /artifacts/external-cheat-base.exe + SDL2.dll + web-radar/dist + CIVETWEB-LICENSE.md

docker build -f radar-relay/Dockerfile -t cs2-radar-relay:local .
```

### C++ regression tests (run from repo root)

Same commands CI uses. Cross-platform (g++) and don't require CS2 or Windows:

```bash
python3 scripts/generate_fixed_map_catalog.py --check

g++ -std=c++20 -O2 -Wall -Wextra -Werror \
  -Iexternal-cheat-base/src \
  tests/viewport_math_tests.cpp -o viewport_math_tests && ./viewport_math_tests

g++ -std=c++20 -O2 -Wall -Wextra -Werror \
  -Iexternal-cheat-base/src \
  tests/runtime_timing_tests.cpp -o runtime_timing_tests && ./runtime_timing_tests

g++ -std=c++20 -O2 -Wall -Wextra -Werror \
  -Iexternal-cheat-base/src \
  tests/web_radar_model_tests.cpp \
  external-cheat-base/src/core/game/web_radar_json.cpp \
  -o web_radar_model_tests && ./web_radar_model_tests
```

> The third test (`web_radar_model_tests`) uses `std::string::starts_with` and `std::filesystem::path::operator!=`. MinGW gcc 8.x (e.g. the `mingw64` bundled with some Code::Blocks installations) cannot compile it; gcc 13+ via MSYS2 or the Docker image's MinGW does. `viewport_math_tests` and `runtime_timing_tests` build with gcc 8.

### Web Radar frontend

```bash
cd web-radar
npm ci && npm test && npm run build
node scripts/validate-bundle.mjs dist   # required before MSBuild post-build
```

### Radar Relay (Go)

```bash
cd radar-relay
go test -mod=vendor -race ./...
go vet -mod=vendor ./...
```

### Update CS2 offsets (Windows)

`external-cheat-base/update_offset.bat` pulls `client_dll.hpp`, `offsets.hpp`, `buttons.hpp` from `a2x/cs2-dumper` and records the commit SHA in `last_commit.txt`. CI has a `.github/workflows/update-files.yml` that does the same hourly. The generated headers live in `external-cheat-base/generated/` and are excluded from CI build triggers.

## Architecture

### Process model

`main.cpp` is the entry point. The program has two cooperating roles:

1. **Data thread** — runs `esp::updateEntities` and `esp::refreshEntityCache` in a loop. Opens the CS2 process once via `memory::GetProcID` + `OpenProcess`, then repeatedly reads memory with `memory::TryRead<T>` / `memory::ReadRaw`. Snapshots are stored in shared_ptr-typed global state (`esp::enemies`, `esp::gameSnapshot`) guarded by `esp::dataMutex`.
2. **Render thread** — SDL2 main loop in `sdl_renderer`. Calls `esp::render`, `local_fixed_radar::render`, the ImGui `menu` overlay, and pumps the `WebRadarService` worker.

Read-side metrics (`memory::GetReadMetrics`) are accumulated per-thread and reported in the diagnostics menu.

### Memory reading (`core/memory`)

- `core/memory/memory.cpp` opens the CS2 process and exposes `TryRead<T>`, `Read<T>`, `Write<T>`. The writer is the anti-flash path (`memory::Write<float>` for flash duration); `memory::gWritesAllowed` defaults to `true` — there is no `--allow-memory-writes` CLI flag anymore.
- Layout values that aren't in `cs2-dumper` (entity-list stride, bone-array pointer math, `GlobalVars::mapname`) are isolated in `core/memory/game_layout.hpp` so they can be reviewed in one place. The `BONE_ARRAY_IN_MODEL_STATE` constant is the well-known hand-derived offset for skeleton access.

### Offsets & schema

`external-cheat-base/generated/{client_dll.hpp,offsets.hpp,buttons.hpp}` are the vendored output of [`a2x/cs2-dumper`](https://github.com/a2x/cs2-dumper). Treat them as generated code — never hand-edit, regenerate via `update_offset.bat` or the `update-files.yml` workflow.

### Snapshot pipeline (`core/game`)

The hot loop produces an immutable `game::GameSnapshot` (`core/game/game_snapshot.hpp`) — a sanitizer-friendly struct containing `PlayerSnapshot[]`, `WorldEntityInfo`-equivalent entries, `MapPhase`, weapons, etc. Player IDs are opaque (`makeOpaquePlayerId`); never a Steam ID or pawn address.

`core/game/web_radar_json.cpp` is the single JSON serializer for the public protocol (`protocolVersion = 1`). `WebRadarService` consumes it via `serializeSnapshotCached` in `main.cpp`, which keeps a thread-local 4-entry cache keyed by snapshot pointer + `SerializationOptions` so consecutive frames at the same configuration skip the serialization cost.

### ESP activation hotkey (`features/menu.hpp`)

The activation feature lives entirely in `menu.hpp` (no `menu.cpp` — the menu is a single ~1.6k-line header):

- `enum class EspActivationMode : std::uint8_t { Toggle = 0, Hold = 1 }` plus `int espActivationKey = VK_LMENU` on both `RuntimeConfig` and the matching `inline` globals.
- `updateEspActivation()` is called from `publishRuntimeConfig()` each frame. In `Toggle` mode, a rising edge of the activation key flips `espEnabled`. In `Hold` mode it's a no-op because the effective state is computed in `buildRuntimeConfig()` from the live key state.
- `effectiveEspEnabled` (a field on `RuntimeConfig` and a global) is what render and sampling gate on. `main.cpp` reads `config.effectiveEspEnabled` for `samplingDemand` and the render gate; `esp.cpp`'s `esp::render` reads `menu::effectiveEspEnabled`. The raw `espEnabled` checkbox is the baseline in Toggle mode and is greyed out (`ImGui::BeginDisabled`) in Hold mode.
- The rebind UI uses the existing `RenderHotkeyButton(...)` helper, and the activation key is folded into `ConfiguredHotkeysReleased` / `FindHotkeyConflict` so it can't collide with the menu-toggle / exit hotkeys.
- Persisted to `%LOCALAPPDATA%\AegisCS2\settings-v1.ini` under `esp_activation_mode` (int 0/1) and `esp_activation_key` (int VK code).

### Web Radar publishing (`features/web_radar`)

- `WebRadarService` — RAII wrapper around embedded CivetWeb. Default bind `127.0.0.1:22006`, token in URL `?token=...`. Each viewer has a single replaceable pending snapshot, so a slow viewer drops stale frames instead of applying backpressure. WebSocket is receive-only; the server uses `NO_SSL`/`USE_WEBSOCKET`/`USE_IPV6` defines (see `web_radar/README.md` for required include dirs).
- The local `wss://` outbound producer and the NDJSON recorder were removed; the Go `radar-relay` deployment in `radar-relay/` still ships for users who want to host their own public relay, but the C++ side no longer publishes to it.
- Adaptive sampling lives in `main.cpp`: 4 Hz idle, 10 Hz background sharing, 20 Hz when a viewer is connected. Sampling rate is decoupled from the render loop's full draw rate.

### Radar (in-game overlay, `features/local_radar`)

`local_fixed_radar::render` consumes the same `game::GameSnapshot` as Web Radar — it never reads game memory directly. It loads precomputed PNGs from `web-radar/public/maps/<map_id>/radar.png` and uses the generated catalog (`core/game/fixed_map_catalog.hpp`, generated by `scripts/generate_fixed_map_catalog.py` from `web-radar/public/maps/manifest.json`).

### ESP (`features/esp.cpp`, `features/esp.hpp`)

- `esp.cpp` owns the data-thread read loop and the ImGui draw calls. State shared with the render thread lives in `esp::enemies` / `esp::gameSnapshot` (`shared_ptr<const T>`) under `esp::dataMutex`.
- The render gate `if (menu::effectiveEspEnabled)` honours the activation hotkey; sub-features (`espWeapon`, `espFlashIndicator`, `espSkeleton`, `espWallCheck`, `grenadeESP`, `droppedWeaponESP`, `bombTimer`) keep their own bools.
- Anti-flash is a `memory::Write<float>` on the local pawn's flash duration; it runs unconditionally now that writes are default-on.

### Performance / diagnostics (`core/`)

- `performance_metrics.hpp` — lock-free `DurationHistogram` with P50/P95/P99/max (15 microsecond buckets).
- `runtime_timing.hpp` — coarser frame budget tracking.
- `diagnostics.hpp` — startup self-check (administrator, `SDL2.dll` adjacent, `web-radar/dist/index.html`, `maps/manifest.json`+`SOURCE.json`) and a logger that writes to `%TEMP%\cs2-esp.log` and `OutputDebugStringW`.

### Vendor

- `external-cheat-base/vendor/SDL2/` — prebuilt `SDL2.dll` + headers (`SDL2.lib` for x64).
- `external-cheat-base/vendor/imgui/` — vendored ImGui source + SDL2/renderer backends, built directly into the executable.
- `vendor/civetweb/` — used as a sibling-directory include path (`$(SolutionDir)vendor\civetweb\include`); built with `NO_SSL`/`NO_CGI`/`USE_WEBSOCKET`/`USE_IPV6`.

## Cross-cutting Conventions

- **C++20, MSVC v143 or v145**, warning policy `-Wall -Wextra -Wpedantic -Werror` for project-owned sources. Third-party (SDL2, ImGui, civetweb C/C++) is compiled with its own policy. The Dockerfile compiles with `x86_64-w64-mingw32-g++-posix` from Ubuntu 24.04 (gcc 13+, supports all C++20 features the code uses).
- **Generated files** under `external-cheat-base/generated/` are the single source of truth for runtime offsets and schemas; never hand-edit.
- **`-insecure` required.** Memory writes default to ON; the only writer is anti-flash. Do not add new write paths without confirming the intent.
- **Token safety.** Web Radar tokens are generated via `BCryptGenRandom`. They must never appear in logs, URLs to third parties, or committed config. The public relay stores SHA-256 digests only (see `radar-relay/README.md`).
- **Shared_ptr snapshots.** Producers publish immutable snapshots (shared_ptr<const T>); consumers never mutate. This is what makes the data-thread/render-thread split race-free without deep copies.
- **Defaults that matter**: `espEnabled`, `webRadarEnabled`, `localRadarEnabled`, `antiFlash`, `memory::gWritesAllowed` all start `true`. If you add a new boolean feature, decide its default deliberately — the trend in this codebase is "feature on, opt-out in the menu".

## Useful starting points

- `external-cheat-base/src/main.cpp` — top-level lifecycle, runtime classes, snapshot cache, the render and sampling gates that branch on `effectiveEspEnabled`.
- `external-cheat-base/src/features/menu.hpp` — the central menu: `RuntimeConfig`, ESP activation hotkey logic, persistence (`loadPersistentSettings`/`savePersistentSettings`), UI tabs, and the `WebRadarUiStatus` panel. This is the largest file in the project.
- `external-cheat-base/src/features/esp.hpp` — data structs (`EnemyInfo`, `LocalPlayerCache`, `BombInfo`, `WorldEntityInfo`, `CachedPawn`) and shared state between threads.
- `external-cheat-base/src/core/memory/memory.hpp` — the read/write primitives every other module uses, plus `gWritesAllowed`.
- `external-cheat-base/src/core/game/web_radar_json.cpp` — the single JSON serializer (`protocolVersion = 1`).
- `external-cheat-base/src/features/web_radar/web_radar_service.cpp` — CivetWeb embed.
- `docs/` — reverse-engineering writeups for the memory layout, view matrix math, and 3D projection used by ESP.
- `radar-relay/README.md` — API surface, config, and threat model for the public relay (host-side only; the C++ producer is gone).
- `Dockerfile` — exact CI compilation flags; copy them when reproducing locally.