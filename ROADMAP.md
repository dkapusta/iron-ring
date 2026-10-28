# Roadmap

The guiding rule comes from the concept kit and still holds: **polish one matchup before expanding the roster.** Most of the effort in a sprite fighter is animation, frame data, feel and testing; the engine stays small on purpose.

Milestones are ordered; items inside a milestone are roughly ordered. ✅ done · 🔜 next · ⬜ planned.

## M0 — Foundation ✅ (this commit)

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

## M1 — First playable matchup: Mara vs Rook 🔜

Goal: two characters that feel good in local play on one stage.

- 🔜 **CI**: GitHub Actions matrix (Windows MSVC, Ubuntu GCC/Clang, macOS Clang): build, `ctest`, batch-mode capture uploaded as an artifact
- 🔜 **Content data at runtime**: move/frame data loaded from JSON instead of `kBaseMoves`; schema validated by AssetPacker at bake time (bad data fails the build, not the game)
- ⬜ **Animation format**: per-frame source rect, pivot, duration, hurt boxes, hit boxes, push box; authoring as JSON next to the atlas (or Aseprite export → converter in AssetPacker)
- ⬜ **Art pass**: redraw Mara and Rook on the 640×360 grid (idle, walk, jump, crouch, 4 attacks × startup/active/recovery, block, hit, KO, win); switch `textureFilter` to `pixelart`
- ⬜ Rook's throws; throw tech; the first special move per character
- ⬜ Round flow: intro, best-of-3, round/KO/time-over banners, win markers
- ⬜ HUD with a bitmap font and portraits (replaces SDL debug text)
- ⬜ Audio: mix groups (SFX / voice / music), music streaming from `.resource`, hit-confirm layering
- ⬜ Training mode basics: frame advantage readout, hitbox overlay, input display, dummy settings
- ⬜ Platform polish: Windows icon + version resource; macOS `.app` bundle (`Contents/Frameworks/UnityPlayer.dylib`, `Contents/Resources/Data`)

## M2 — Complete local play

- ⬜ Title, character select, stage select, options scenes (scene stack)
- ⬜ Gamepads via SDL_Gamepad; remapping UI; per-device profiles
- ⬜ Persistent settings (a `PlayerPrefs`-style store in the user folder)
- ⬜ Renderer recreation when HDR state or display changes; options for HDR on/off, presentation mode, vsync
- ⬜ Stage layers and parallax; animated crowds; camera follow and zoom limits
- ⬜ Hitstop/shake/flash tuning pass; HDR-aware VFX (float textures for sparks)
- ⬜ Arcade ladder with CPU difficulty levels

## M3 — Roster and world

- ⬜ Lina, Tariq, Edda, Kei (designs exist in the kit)
- ⬜ Remaining stages; "Iron Ring" underground variants (see open question 1)
- ⬜ Boss character(s), story intros/endings
- ⬜ Asset streaming: per-stage `sharedassetsN.assets` + `levelN` loaded on demand

## M4 — Online

- ⬜ Versioned, explicit state serialization (no struct copies); replay files
- ⬜ Rollback netcode: input prediction, resimulation, input delay, desync detection via checksums
- ⬜ Event reconciliation so sounds/VFX never double-fire on resimulation (`StepEvents` is the hook)
- ⬜ Cross-platform determinism tests (Windows/Linux/macOS produce identical checksums)

## M5 — Release hardening

- ⬜ Crash handler process (a `UnityCrashHandler64.exe` look-alike in spirit: minidump + log upload prompt)
- ⬜ Installer/packaging, store integration as needed
- ⬜ Localization, accessibility (colour-blind safe HUD, input assistance)
- ⬜ Performance budget and capture tooling

## Later / maybe

- A Fatal Fury-style **two-plane** (foreground/background line) mechanic, prototyped as an optional stage rule — only if the single-plane game is already fun.
- Low-health desperation moves (another Fatal Fury 2 trait) vs. the meter-based Current system — pick one, not both.

## Open questions for the project owner

1. **Setting.** The project brief says *illegal underground fight clubs and a tournament*; the Brass Current kit describes a public, family-friendly harbor festival ("no gore"). Proposal: keep Port Meridian and its legal Floodlight Circuit as the public face, and make the **Iron Ring** the illegal after-hours bracket held in the same places after closing (darker lighting variants of existing stages, new basement/warehouse stages). Needs a decision before M1 art. See [ADR-0007](docs/adr/0007-concept-kit-as-placeholder-content.md).
2. **Names.** Keep the kit's names (Mara Voss, Rook Calder, "Current" meter, "Current Break") or rename for Iron Ring?
3. **License** for code and for art (separately).
4. **Company name** used in `app.info` and the user-data folder (currently `dkapusta`).
5. **Target platforms** for M1 CI: Windows + Linux certainly; macOS?
