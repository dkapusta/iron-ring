# AGENTS.md — working agreements

Guidance for anyone changing this repository, human or AI coding agent. Read this before the first edit; read [docs/architecture.md](docs/architecture.md) before touching more than one module.

## Build and verify

```sh
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
```

- `core` preset (no SDL) is the fastest loop for `Source/Core`, `Source/Vfs`, `Source/Simulation` and `Tests/`.
- Player smoke test (no display needed):
  `Build/debug/Player/Debug/IronRing.x86_64 -batchmode -screen-fullscreen 0 -screen-width 1920 -screen-height 1080 -frames 205 -screenshot /tmp/shot.png`
  Exit code 0 and a sensible PNG is the bar for any change to `Source/Runtime` or `Assets/`. Look at the image.
- Configure with `-DIR_WARNINGS_AS_ERRORS=ON` before declaring work done. CI (`.github/workflows/ci.yml`) enforces it on Linux GCC/Clang and Windows MSVC/MinGW — a change isn't done until every job is green.
- Names and world facts come from [docs/design/setting.md](docs/design/setting.md); the Brass Current kit is visual reference only.
- Offline or slow network: point FetchContent at local checkouts, e.g. `-DFETCHCONTENT_SOURCE_DIR_SDL3=/src/SDL` (also `_SDL3_MIXER`, `_MINIZ`, `_NLOHMANN_JSON`, `_DOCTEST`).

## Module boundaries (enforced by CMake targets)

| Target | May depend on | Must not depend on |
| --- | --- | --- |
| `IronRingSimulation` | C++ standard library | SDL, I/O, clocks, randomness, floating point in state |
| `IronRingCore`, `IronRingVfs` | std, nlohmann/json, miniz | SDL |
| `AssetPacker` | Core libraries | SDL, Runtime |
| `IronRingEngine` (UnityPlayer) | everything above + SDL3, SDL3_mixer | — |
| `IronRingPlayer` (launcher) | `Public/IronRing/Player.h` only | anything else |

If a change seems to need a forbidden edge, stop and write an ADR instead.

The launcher ↔ engine API (`Public/IronRing/Player.h`) is **C only**: plain C types, no `std::` objects, no exceptions crossing it. With MinGW each side may link its own static C++ runtime ([ADR-0009](docs/adr/0009-mingw-static-runtime.md)).

## Simulation rules (determinism)

- Integer state only; positions in 1/256 logical pixels. No `float`/`double` in `Match` or `Fighter`.
- One `step()` = one 60 Hz tick. No wall-clock time, no `rand()`, no static mutable state.
- Every new state field goes into `Match::checksum()` in a fixed position.
- Presentation (sounds, sparks, camera) reacts to `StepEvents`, never by diffing state.
- The golden checksum test in `Tests/SimulationTests.cpp` pins behaviour. If a gameplay change is intended, update the constant in the same commit and say why in the message.

## Unity-style layout (the gimmick)

- Output names come from CMake cache variables (`IR_PRODUCT_NAME`, `IR_PLAYER_LIBRARY_NAME`). Never hard-code `UnityPlayer` or `IronRing_Data` in C++; use the `IR_*` compile definitions.
- Archive names are defined only in `ProjectSettings/AssetPipeline.json`. CMake and AssetPacker both read it.
- The gimmick is cosmetic. Don't imitate Unity's binary formats or embed Unity branding (logos, splash screens, version resources claiming Unity Technologies). See [ADR-0003](docs/adr/0003-unity-style-player-layout.md).

## Assets

- Source content lives in `Assets/`; runtime code addresses it by **virtual path** relative to `Assets/` (`"Stages/PierNine/pier_nine.png"`). Settings live under `"ProjectSettings/..."`.
- A file under `Assets/` that no pipeline rule packs produces an AssetPacker warning — fix the rule, don't ignore the warning.
- Use `-assets-folder Assets` to iterate on content without repacking.
- Never commit franchise material (Fatal Fury or otherwise) — reference images stay outside the repository.
- Prefer formats SDL can load without extra libraries: PNG, WAV, OGG, FLAC.

## Code style

- C++20, `.clang-format` at the root (4 spaces, 120 columns). Namespaces: `iron`, `iron::sim`, `iron::vfs`, `iron::tools`.
- Files and types `PascalCase`, functions and variables `camelCase`, members `trailing_`, constants `kName`.
- RAII for every SDL handle (`Platform/SdlHandles.h`). No naked `new`/`delete`.
- Exceptions for unrecoverable setup errors (missing data, bad JSON); never in the per-tick hot path.
- SOLID where it buys something: interfaces exist at real seams (`vfs::Mount`, `Scene`). Don't add an interface with one implementation and no foreseeable second one.
- Comments say *why*. Keep functions short enough to read without scrolling.
- Watch for range-for over a member of a temporary (`for (auto& x : makeJson().at("k"))`) — it dangles before C++23. Bind the temporary to a local first.

## Documentation duties

- Architectural or cross-cutting decision → new ADR in `docs/adr/` (copy `0000-template.md`, next number, add to the index).
- Milestone scope changes → `ROADMAP.md`.
- New command-line flag, control or build option → `README.md`.
- New asset convention → `docs/asset-pipeline.md`.

## Commits

- Small, focused commits; imperative subject ≤ 72 chars (`Add per-frame hurt boxes to sprite atlas`).
- Code, tests and docs for one change travel together.
- Don't commit `Build/`, generated archives, logs or screenshots (see `.gitignore`).
