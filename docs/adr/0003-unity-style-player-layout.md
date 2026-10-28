# ADR-0003: Unity-style player layout

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The owner wants the shipped build to *look* like a Unity player: directory structure, naming, `UnityPlayer.dll`, data-file extensions. This is a stylistic gimmick, but it also nudges us toward a clean split that Unity got right: a tiny launcher, one engine module, and a data folder.

A Unity Windows player looks like this:

```text
Game.exe  UnityPlayer.dll  UnityCrashHandler64.exe  MonoBleedingEdge/
Game_Data/  app.info  boot.config  globalgamemanagers  globalgamemanagers.assets
            level0  resources.assets  resources.resource  sharedassets0.assets  sharedassets0.assets.resS
            Managed/  Plugins/  Resources/  StreamingAssets/
```

On Linux the executable is `Game.x86_64` and the engine is `UnityPlayer.so`.

## Decision

Mirror the parts that correspond to something real in our engine; skip the rest.

| Unity file | Iron Ring | Contents |
| --- | --- | --- |
| `Game.exe` / `Game.x86_64` | `IronRing.exe` / `IronRing.x86_64` | launcher: `wWinMain` → `UnityMain`, `main` → `PlayerMain` |
| `UnityPlayer.dll` / `.so` | `UnityPlayer.dll` / `.so` | the whole engine; SDL3 + SDL3_mixer linked in statically |
| `Game_Data/app.info` | same | line 1 company, line 2 product (read at startup) |
| `Game_Data/boot.config` | same | `key=value` player boot options |
| `globalgamemanagers` | same | `ProjectSettings/*` |
| `resources.assets` / `.resource` | same | `Assets/Resources/**` (audio in `.resource`) |
| `sharedassets0.assets` | same | gameplay content |
| `level0`, `sharedassetsN` | planned (M3) | per-stage streaming |
| `UnityCrashHandler64.exe` | planned (M5) | our own crash reporter |
| `MonoBleedingEdge/`, `Managed/` | **not mirrored** | we have no managed runtime; empty decoys would be dishonest clutter |

Rules:

- Names come from CMake cache variables: `IR_PRODUCT_NAME` (`IronRing`), `IR_PLAYER_LIBRARY_NAME` (`UnityPlayer`), `IR_PRODUCT_DISPLAY_NAME`, `IR_COMPANY_NAME`. C++ never hard-codes them.
- The engine exports `PlayerMain(argc, argv)` everywhere and, on Windows, `UnityMain(HINSTANCE, HINSTANCE, LPWSTR, int)` with Unity's signature.
- Command-line flags reuse Unity's names where the meaning matches (`-screen-fullscreen`, `-screen-width`, `-screen-height`, `-logFile`, `-batchmode`). The log is `Player.log` in the per-user folder, previous run `Player-prev.log`.
- The file *contents* are our own formats (ADR-0004). We do not imitate Unity's serialized-file binary format.
- Nothing claims to be Unity: no Unity logos or splash, version resources name our company, README carries a trademark note.

## Consequences

- The build output is a self-contained folder that can be zipped as is.
- Data-mining tools that recognise Unity layouts (AssetRipper, UABE, …) will try and fail to parse our archives; anyone curious can still open them with any ZIP tool. That is part of the joke and harmless.
- **Trademark caution:** "Unity" and "UnityPlayer" are Unity Technologies trademarks. Shipping a file called `UnityPlayer.dll` in a commercial product could confuse users or support channels. Before any public release, decide whether to keep the name; switching is `-DIR_PLAYER_LIBRARY_NAME=IronRingPlayer` and touches no code.
- Linux needs `RPATH=$ORIGIN` on the launcher (set by CMake) — exactly what Unity's Linux players do.

## Alternatives considered

- **Single static executable**: simplest to ship, but loses the requested look and the clean launcher/engine split.
- **Mirroring every Unity folder** (empty `Managed/`, `MonoBleedingEdge/`): pure decoration with maintenance cost and a misleading picture of the runtime.
