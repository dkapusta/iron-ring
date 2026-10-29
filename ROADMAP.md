# Roadmap

The guiding rule comes from the concept kit and still holds: **polish one matchup before expanding the roster.** Most of the effort in a sprite fighter is animation, frame data, feel and testing; the engine stays small on purpose.

Milestones are ordered; items inside a milestone are roughly ordered. ✅ done · 🔜 next · ⬜ planned.

## M0 — Foundation ✅

- ✅ CMake ≥ 3.25 project, presets (Ninja debug/release, VS 2022, core-only), FetchContent with pinned tags
- ✅ Unity-style player layout: launcher + `UnityPlayer` shared library + `IronRing_Data/` with `app.info`, `boot.config`
- ✅ AssetPacker: `Assets/` + `ProjectSettings/` → deterministic ZIP archives (`globalgamemanagers`, `resources.assets`, `resources.resource`, `sharedassets0.assets`); `--list` inspector
- ✅ Runtime VFS: archive mounts, loose-folder override (`-assets-folder`), sandboxed virtual paths
- ✅ Fullscreen at native desktop resolution, 640×360 logical canvas with integer scaling, HDR detection and scRGB highlight boost
- ✅ SDL3_mixer one-shot SFX from archives; silent fallback without a device
- ✅ Combat core ported from the kit prototype, bit-exact (golden checksum), presentation events
- ✅ Placeholder versus screen: stage, two fighters, effects, HUD from `hud_layout.json`, CPU or 2P
- ✅ `Player.log`, Unity-style command-line arguments, `-batchmode` smoke runs with PNG capture
- ✅ 21 unit tests (simulation, archives, settings) + packer smoke test
- ✅ README, AGENTS, ADRs 0001–0008, architecture and pipeline notes, research notes

## M1 — First playable matchup: Dee vs Bram 🔜

Goal: two characters that feel good in local play on one stage. Target platforms: **Windows and Linux** (macOS later).

- ✅ **CI**: GitHub Actions — Linux GCC/Clang, Windows MSVC and MinGW: build with warnings as errors, `ctest`, batch-mode capture uploaded as an artifact
- ✅ **MinGW runtime**: binaries run from the build folder without MinGW on PATH (ADR-0009)
- ✅ **Iron Ring setting** and renamed content ([setting.md](docs/design/setting.md), ADR-0007)
- ✅ **License**: zlib (`LICENSE`), third-party notices (`THIRD_PARTY_NOTICES.md`)
- ✅ **MinGW toolchain robustness**: static runtime when it links, runtime DLLs copied otherwise (CLion 2026.2 bundled GCC 15.2); CI covers an msvcrt GCC 15.2 toolchain
- ✅ **Fighters as data**: one `<id>.fighter.json` per fighter (frame data, boxes, physics, animations), parsed by the SDL-free Content library and validated by AssetPacker at bake time, timing contract included ([ADR-0010](docs/adr/0010-fighter-content-format.md))
- ✅ **Animation format**: named frames in a trimmed atlas with pivots and throw-partner offsets; per-animation tick timing in the fighter file. Hurt boxes are per state (stand/crouch), hit boxes per move
- ✅ **Interim art**: procedural pixel art for Dee, Bram, Pier 9, Lantern Row, The Crown, effects, fonts, portraits and icon on the 640×360 grid; `textureFilter: pixelart` ([ADR-0011](docs/adr/0011-procedural-interim-art.md))
- ✅ **Art contract for final art**: [asset specification](docs/art/asset-spec.md), [generation prompts](docs/art/generation-prompts.md), importer with pose-reference export, art self-test in CI
- ✅ Throws for both fighters (LP + LK) with a throw-tech window; one special each (Dee's Rush Uppercut 236P, Bram's Dock Hoist 63214P command throw); knockdowns
- ✅ Round flow: intro, best-of-3 (up to 5 rounds on draws), ROUND/FIGHT/K.O./TIME/DRAW banners, win markers, match winner
- ✅ HUD with bitmap fonts, portraits, ghost health bars and Heat meters
- ✅ Training overlay (F3 / `-training`): hit and hurt boxes, move names with frame data, frame advantage
- ✅ CPU vs CPU demo mode (`-demo`, Tab); CI smoke run uses it
- ✅ Windows icon and version resources; window icon on both platforms
- ✅ **Final art** for Dee, Bram, their portraits and all three stages (Art Pack v1, re-registered at one scale per fighter; sources in `ArtSource/`, [ADR-0012](docs/adr/0012-adopting-external-art.md))
- ⬜ Animator pass on the generated frames in training mode (punch extension on Bram's LP/HP, walk-cycle smoothness, held-pose contact)
- ⬜ Training mode, rest: input display, dummy settings (block all / stand / crouch / jump), position reset
- ⬜ Per-frame hurt boxes for moves that change the silhouette (sweeps, uppercut)
- ⬜ Audio: mix groups (SFX / voice / music), music streaming from `.resource`, hit-confirm layering; throw, tech and KO sounds
- ⬜ Linux `.desktop` file and icon

## M2 — Complete local play

- ⬜ Title, character select, stage select, options scenes (scene stack)
- ⬜ Gamepads via SDL_Gamepad; remapping UI; per-device profiles
- ⬜ Persistent settings (a `PlayerPrefs`-style store in the user folder)
- ⬜ Renderer recreation when HDR state or display changes; options for HDR on/off, presentation mode, vsync
- ⬜ Stage layers and parallax; animated crowds; camera follow and zoom limits
- ⬜ Hitstop/shake/flash tuning pass; HDR-aware VFX (float textures for sparks)
- ⬜ Arcade ladder with CPU difficulty levels

## M3 — Roster and world

- ⬜ Sable, Idris, Greta, Jun (identities in `setting.md`; a fighter file, atlas and portrait each — no code)
- ⬜ Final art for Lantern Row and The Crown; new venues (dry dock, foundry, ship's hold, the Broker's club)
- ⬜ The Broker (boss), story intros/endings, win quotes
- ⬜ Asset streaming: per-stage `sharedassetsN.assets` + `levelN` loaded on demand

## M4 — Online

- ⬜ Versioned, explicit state serialization (no struct copies); replay files
- ⬜ Rollback netcode: input prediction, resimulation, input delay, desync detection via checksums
- ⬜ Event reconciliation so sounds/VFX never double-fire on resimulation (`StepEvents` is the hook)
- ⬜ Cross-platform determinism tests (Windows/Linux, MSVC/GCC/Clang/MinGW produce identical checksums)

## M5 — Release hardening

- ⬜ Crash handler process (a `UnityCrashHandler64.exe` look-alike in spirit: minidump + log upload prompt)
- ⬜ Installer/packaging, store integration as needed
- ⬜ Localization, accessibility (colour-blind safe HUD, input assistance)
- ⬜ Performance budget and capture tooling
- ⬜ macOS: `.app` bundle (`Contents/Frameworks/UnityPlayer.dylib`, `Contents/Resources/Data`), CI job, notarization

## Later / maybe

- A Fatal Fury-style **two-plane** (foreground/background line) mechanic, prototyped as an optional stage rule — only if the single-plane game is already fun.

## Decisions log

| Date | Question | Decision |
| --- | --- | --- |
| 2026-10-29 | Setting | Iron Ring brief as written; kit is visual reference only, art to be generated ([ADR-0007](docs/adr/0007-concept-kit-as-placeholder-content.md)) |
| 2026-10-29 | Names | Renamed for Iron Ring ([setting.md](docs/design/setting.md)) |
| 2026-10-29 | Company name | Keep `dkapusta` |
| 2026-10-29 | Platforms | Windows + Linux; macOS later (M5) |
| 2026-10-29 | MinGW runtime DLLs | Static when the toolchain allows, DLLs copied next to executables otherwise ([ADR-0009](docs/adr/0009-mingw-static-runtime.md)) |
| 2026-10-29 | License | zlib, matching SDL ([LICENSE](LICENSE), [docs/licensing.md](docs/licensing.md)) |
| 2026-10-29 | Desperation moves | None. Heat pays only for the Breakaway |
| 2026-10-29 | Art | Procedural interim art now; final art made elsewhere against the [asset spec](docs/art/asset-spec.md) and imported ([ADR-0011](docs/adr/0011-procedural-interim-art.md)) |
| 2026-10-30 | Final art | Art Pack v1 adopted; frames re-registered at one scale, 48 colours per fighter ([ADR-0012](docs/adr/0012-adopting-external-art.md)) |

## Open questions for the project owner

1. **Final art production**: which generator(s) and who does the pixel cleanup pass. The [workflow](docs/art/generation-prompts.md) assumes a tool that accepts a pose or structure reference (img2img or ControlNet-style).
2. **Sound**: keep the kit's placeholder sound effects for M1, or replace them alongside the final art?
