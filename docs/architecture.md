# Architecture

## Module map

```mermaid
flowchart LR
    subgraph Build time
        AP[AssetPacker] -->|reads| PS[ProjectSettings/AssetPipeline.json]
        AP -->|reads| AS[Assets/ + ProjectSettings/]
        AP -->|writes| DATA[(IronRing_Data/*.assets, *.resource, globalgamemanagers)]
    end
    subgraph Player
        EXE[IronRing.exe / .x86_64] -->|UnityMain / PlayerMain| ENG[UnityPlayer.dll / .so]
        ENG --> CORE[IronRingCore\nsettings]
        ENG --> VFS[IronRingVfs\narchives + mounts]
        ENG --> SDL[SDL3 + SDL3_mixer\nstatic]
        VFS -->|mounts| DATA
    end
    CON[IronRingContent\nfighter data, atlases, validation] --> SIM[IronRingSimulation\ncombat]
    ENG --> CON
    AP -->|validates with| CON
    AP --> VFS
```

Static libraries without SDL (`Core`, `Vfs`, `Simulation`, `Content`) carry everything that can be unit-tested (the simulation also holds the training rules and the deterministic training dummy, `Simulation/Training`); `Content` turns JSON into the simulation's plain `FighterSpec` and checks the timing contract ([ADR-0010](adr/0010-fighter-content-format.md)), so the simulation never sees JSON; the engine module adds the platform. The launcher knows only `Public/IronRing/Player.h`.

## Engine internals (`Source/Runtime`)

| Folder | Responsibility |
| --- | --- |
| `Player/PlayerMain.cpp` | exported entry points; resolve data folder; `boot.config` + command line → `PlayerSettings`; `app.info`; `Player.log`; fatal-error dialog |
| `Player/Application` | composition root; mounts archives; owns Display, Audio, AssetLibrary, Scene; fixed-step loop; F-key handling; screenshots |
| `Platform/Display` | fullscreen window at desktop resolution; renderer with sRGB or scRGB output; HDR state; logical presentation |
| `Platform/PlayerLog` | mirrors SDL logging into `Player.log` |
| `Assets/AssetLibrary` | virtual path → JSON / decoded texture (cached) |
| `Audio/AudioSystem` | SDL3_mixer mixer; one-shot SFX by id; silent fallback |
| `Game/Scene` | scene interface (`handleEvent`, `fixedUpdate`, `render`, `status`) |
| `Game/FightScene` | versus screen: loads fighters and stages from data; `Match` + views + effects + HUD + overlay; events → sounds; vs CPU / 2P / demo |
| `Game/FighterView` | simulation state → animation frame (and the held opponent's pose during throws) |
| `Game/Effects` | short sprite effects spawned from `StepEvents`, HDR-boosted |
| `Game/Hud` | health, Heat, timer, portraits, win markers, round banners, from `hud_layout.json` |
| `Game/TrainingOverlay` | F3: hit/hurt boxes (per move phase), move frame data, frame advantage |
| `Game/InputDisplay` | training: both players' recent inputs (ticks, numpad direction, buttons) |
| `Game/InputMapper` | keyboard → simulation input bits, edge latch between ticks |
| `Render/SpriteSheet`, `Render/BitmapFont` | atlas frames drawn at their pivots (flippable); monospaced bitmap text |

## Startup sequence

1. Launcher calls `UnityMain` (Windows) or `PlayerMain`.
2. Command line is probed for `-data-folder`; default `<exe dir>/IronRing_Data`.
3. `PlayerSettings` = defaults ← `boot.config` ← command line. Unknown entries become log warnings, never errors.
4. `app.info` gives company/product → per-user folder → `Player.log` (previous run rotated to `Player-prev.log`).
5. `Application`: `-batchmode` selects offscreen video and dummy audio → `SDL_Init(VIDEO)` → mount every archive (sorted), then `-assets-folder` → read `ProjectSettings/GameSettings.json` → `Display` → `AudioSystem` → `AssetLibrary` → `FightScene`.
6. Main loop: poll events → accumulate time (cap 250 ms) → `fixedUpdate()` per 1/60 s → `render()` → present. Batch mode runs one tick per frame and exits after `-frames`, optionally saving the last frame.

Any exception during setup is logged as `Fatal:` and shown in a message box (except in batch mode); the process exits with code 1.

## Frame anatomy

```text
fixedUpdate (60 Hz)                         render (display rate)
  InputMapper.sample(p1, p2 | CPU)            clear → stage → fighters (z by attacker)
  Match.step(inputs) → MatchStep              → effects (colour scale = HDR boost)
  events → AudioSystem.play(...), Effects     → HUD → [training overlay] → [debug overlay] → [capture] → present
```

## Conventions that cross modules

- **Coordinates:** simulation in 1/256 logical pixels, origin at stage left, y up from the floor; rendering in 640×360 logical pixels, y down, floor at 316.
- **Virtual paths:** `Assets/`-relative (`Stages/...`, `Resources/...`) or `ProjectSettings/...`.
- **Names from CMake:** `IR_VERSION_STRING`, `IR_COMPANY_NAME`, `IR_PRODUCT_DISPLAY_NAME`, `IR_DATA_DIR_NAME`, `IR_PLAYER_LIBRARY_NAME` are compile definitions of the engine.
