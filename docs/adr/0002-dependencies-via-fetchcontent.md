# ADR-0002: Dependencies via CMake FetchContent, pinned tags, static linking

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The project needs SDL3, SDL3_mixer, a ZIP library, a JSON library and a test framework on Windows, Linux and macOS without asking contributors to install them system-wide. SDL 3.4 is newer than most distribution packages.

## Decision

- All dependencies are declared in `cmake/Dependencies.cmake` with `FetchContent_Declare(... GIT_REPOSITORY ... GIT_TAG <release tag> GIT_SHALLOW ON SYSTEM)`.

  | Library | Tag | Purpose |
  | --- | --- | --- |
  | SDL | `release-3.4.18` | platform, renderer, input, audio device |
  | SDL_mixer | `release-3.2.4` | sound playback |
  | miniz | `3.1.2` | ZIP read/write for archives |
  | nlohmann/json | `v3.12.0` | settings and content data |
  | doctest | `v2.5.3` | unit tests (only when `IR_BUILD_TESTS`) |

- Tags are cache variables (`IR_SDL3_TAG`, …) so a bump is one line and can be tried without editing files.
- Every dependency is built **static**, with its own tests/examples/install disabled. SDL3 and SDL3_mixer are linked into the engine module, so the player folder has no extra DLLs.
- `SYSTEM` keeps third-party headers out of our warning flags.
- Offline builds: `FETCHCONTENT_SOURCE_DIR_<NAME>` for local checkouts, or `FETCHCONTENT_FULLY_DISCONNECTED=ON` after one successful configure. Each preset keeps its dependencies in its own `Build/<preset>/_deps`. *(Revised 2026-10-29: presets first shared one `Build/_deps`, but FetchContent keeps the dependencies' build trees there too, so debug and release overwrote each other's SDL objects and could link the other configuration's code. A shallow clone per preset is the price of correctness.)*

## Consequences

- One command builds everything; first configure takes a few minutes (SDL is large).
- Static zlib-licensed SDL is fine for distribution; keep the licence texts in the release package (M5).
- On Linux, SDL needs X11/Wayland/ALSA/PulseAudio **headers** at build time (it `dlopen`s them at runtime). README lists the packages.
- Tags are mutable in theory; if supply-chain hardening becomes a concern, switch to commit SHAs (losing `GIT_SHALLOW`) or URL tarballs with `URL_HASH`.

## Alternatives considered

- **vcpkg / Conan**: good tools, but an extra install step and manifest for five libraries. Can be adopted later; FetchContent's `FIND_PACKAGE_ARGS` makes the switch gradual.
- **Git submodules**: easy to forget to update, awkward shallow history.
- **Shared SDL next to the exe**: would add `SDL3.dll` to the player folder and break the single-module look of ADR-0003.
