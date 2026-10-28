# ADR-0001: Own small engine on C++20 + SDL3 + SDL3_mixer

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

Iron Ring is a single-plane 2D sprite fighter. Its hard problems are animation volume, frame data, feel, determinism and (later) rollback netcode — not rendering. The concept kit already contains a working integer combat core and an SDL3 frontend sketch. The owner asked for C++ / SDL3 / SDL_mixer.

## Decision

- Language: **C++20** (concepts/ranges where they clarify; no modules yet — toolchain support is uneven).
- Platform layer: **SDL 3.4** (window, input, renderer via `SDL_Renderer`, PNG I/O, audio device). 3.4 is the floor because of `SDL_LoadPNG_IO`/`SDL_SavePNG` and `SDL_SCALEMODE_PIXELART`.
- Audio: **SDL3_mixer 3.2** (`MIX_Mixer`/`MIX_Audio`/`MIX_Track` API), header-only decoders only (WAV, stb_vorbis, dr_flac).
- Rendering stays on `SDL_Renderer` (2D textured quads, colour scale for HDR). Move to the SDL GPU API only if a measured need appears (custom shaders for palette swaps, post effects).
- Data: JSON (nlohmann/json) for settings and content specs; binary formats only when profiling says so.

## Consequences

- Small, auditable codebase; everything is debuggable source.
- We own tooling (asset packer, later animation tools); that cost is accepted and kept minimal.
- `SDL_Renderer` has no custom shaders: palette swaps will need either pre-baked texture variants or a later GPU-API renderer.

## Alternatives considered

- **Unity / Godot**: heavy for the problem, and rollback-friendly determinism fights the engine. (We borrow Unity's *build layout* for fun only — ADR-0003.)
- **raylib**: pleasant, but SDL3 has stronger HDR, display and input coverage and was requested.
- **SDL_GPU from day one**: more code before the first frame; no current need.
