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
- ✅ **MinGW static runtime**: binaries run from the build folder without MinGW DLLs (ADR-0009)
- ✅ **Iron Ring setting** and renamed content ([setting.md](docs/design/setting.md), ADR-0007)
- 🔜 **License** files once chosen ([docs/licensing.md](docs/licensing.md))
- 🔜 **Content data at runtime**: move/frame data loaded from JSON instead of `kBaseMoves`; schema validated by AssetPacker at bake time (bad data fails the build, not the game)
- ⬜ **Animation format**: per-frame source rect, pivot, duration, hurt boxes, hit boxes, push box; authoring as JSON next to the atlas (or Aseprite export → converter in AssetPacker)
- ⬜ **Art pass**: generate Dee and Bram for the Iron Ring setting on the 640×360 grid (idle, walk, jump, crouch, 4 attacks × startup/active/recovery, block, hit, KO, win), plus Pier 9 at night; switch `textureFilter` to `pixelart`. Needs an art-generation workflow decision (tools, prompts from `setting.md`, cleanup pass)
- ⬜ Bram's throws; throw tech; the first special move per character
- ⬜ Round flow: intro, best-of-3, round/KO/time-over banners, win markers
- ⬜ HUD with a bitmap font and portraits (replaces SDL debug text)
- ⬜ Audio: mix groups (SFX / voice / music), music streaming from `.resource`, hit-confirm layering
- ⬜ Training mode basics: frame advantage readout, hitbox overlay, input display, dummy settings
- ⬜ Platform polish: Windows icon + version resource; Linux `.desktop` file and icon

## M2 — Complete local play

- ⬜ Title, character select, stage select, options scenes (scene stack)
- ⬜ Gamepads via SDL_Gamepad; remapping UI; per-device profiles
- ⬜ Persistent settings (a `PlayerPrefs`-style store in the user folder)
- ⬜ Renderer recreation when HDR state or display changes; options for HDR on/off, presentation mode, vsync
- ⬜ Stage layers and parallax; animated crowds; camera follow and zoom limits
- ⬜ Hitstop/shake/flash tuning pass; HDR-aware VFX (float textures for sparks)
- ⬜ Arcade ladder with CPU difficulty levels

## M3 — Roster and world

- ⬜ Sable, Idris, Greta, Jun (identities in `setting.md`; kit designs as visual reference)
- ⬜ Lantern Row and The Crown art; new venues (dry dock, foundry, ship's hold, the Broker's club)
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
- Low-health desperation moves (another Fatal Fury 2 trait) vs. the meter-based Heat system — pick one, not both.

## Decisions log

| Date | Question | Decision |
| --- | --- | --- |
| 2026-10-29 | Setting | Iron Ring brief as written; kit is visual reference only, art to be generated ([ADR-0007](docs/adr/0007-concept-kit-as-placeholder-content.md)) |
| 2026-10-29 | Names | Renamed for Iron Ring ([setting.md](docs/design/setting.md)) |
| 2026-10-29 | Company name | Keep `dkapusta` |
| 2026-10-29 | Platforms | Windows + Linux; macOS later (M5) |
| 2026-10-29 | MinGW runtime DLLs | Link statically ([ADR-0009](docs/adr/0009-mingw-static-runtime.md)) |

## Open questions for the project owner

1. **License** for code and for content — options compared in [docs/licensing.md](docs/licensing.md); MIT is the working assumption for code.
2. **Art generation workflow** for the M1 art pass: which tools, who does pixel cleanup, how sprites are kept consistent across frames.
3. **Desperation moves vs. Heat** — keep only the meter, or add a Fatal Fury-style low-health super? (affects M1 special-move design)
