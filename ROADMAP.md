# Roadmap

The guiding rule comes from the concept kit and still holds: **polish one matchup before expanding the roster.** Most of the effort in a sprite fighter is animation, frame data, feel and testing; the engine stays small on purpose.

Milestones are ordered; items inside a milestone are roughly ordered. ✅ done · 🔜 next · ⬜ planned.

## How art is made

1. **Placeholder art and guidelines (here).** New content first ships with placeholder art from `Tools/Art/` and a written contract: the [asset spec](docs/art/asset-spec.md), [prompts](docs/art/generation-prompts.md) and pose references.
2. **Final art (elsewhere).** The project owner generates the final art outside this repository against that contract.
3. **Review and corrections (here).** The art is imported and reviewed in training mode and with `Tools/Art/review_frames.py`, and correction notes are written ([example](docs/art/frame-review.md)).
4. **Polish.** A corrected pack is generated and imported, followed by a final pass.

Final art replaces placeholder art only when a **vertical slice** needs it. That's the end of M2: the complete local loop for one matchup at release quality. Until then, new fighters and venues ship with placeholder art. Art Pack v1 (Dee, Bram, three venues) is already in; its corrections (Art Pack v2) are scheduled for M3.

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

## M1 — First playable matchup: Dee vs Bram ✅

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
- ✅ Frame review of Art Pack v1 with correction notes ([frame-review.md](docs/art/frame-review.md)), animation rules in the asset spec (§2.6) and `Tools/Art/review_frames.py`
- ✅ Training mode: training rules in the simulation (clock stops, no K.O., health and Heat refill), a deterministic dummy (stand, crouch, jump, guard, guard all, CPU; F6, `-dummy`), input display, position reset (F4)
- ✅ Per-phase hurtboxes for moves that change the silhouette: sweeps duck, extended limbs can be hit, Dee's uppercut dips then rises
- ✅ **Audio**: NES-style effects and the "Iron Ring" fight theme generated from a 2A03 model; music streamed and looping past its intro; SFX/music mix groups with volume settings; a soft limiter; round-call, tech, slam, K.O. and victory cues ([ADR-0013](docs/adr/0013-nes-style-generated-audio.md))
- ✅ A theme per venue ("Iron Ring", "Curfew Market", "Crown at Dawn") and the ring announcer through the DMC channel, with a voice mix group ([ADR-0014](docs/adr/0014-venue-themes-and-announcer.md))
- ✅ `cmake --install`: the player folder, plus a Linux desktop entry and hicolor icon; the app id is passed to SDL so the window matches the entry ([docs/install.md](docs/install.md))

## M2 — Complete local play 🔜

- ✅ Title (with an attract demo), character select (the full roster, unbuilt fighters locked), stage select, options, pause and post-match menus, on a scene stack with fades ([ADR-0015](docs/adr/0015-front-end-scene-stack.md)); a front-end theme and menu sounds
- ✅ Gamepads via SDL_Gamepad: two pads, one layout from `InputManager.json`
- ✅ Remapping (Options → Controls): keys per player, pad buttons per device, saved to `prefs.cfg`; a move list in the pause menu; [docs/controls.md](docs/controls.md) ([ADR-0016](docs/adr/0016-remapping-and-renderer-recreation.md))
- ✅ Persistent settings: `prefs.cfg` in the user folder, between `boot.config` and the command line
- ✅ Renderer recreation when the HDR option, the display's HDR state or the display changes; fullscreen, scaling and vsync change live
- ✅ Stage layers and parallax; animated crowds that react to the fight; camera follow (no zoom: pixel art) on stages wider than the screen; **The Foundry**, the first layered venue, with placeholder art and its own theme ([ADR-0017](docs/adr/0017-layered-stages-and-camera.md))
- ⬜ Hitstop/shake/flash tuning pass; HDR-aware VFX (float textures for sparks); hit-confirm audio layering (needs counter hits and combo counts in the simulation)
- ⬜ **Vertical slice**: the complete local loop for Dee vs Bram at release quality; final art takes over from placeholder art where the slice needs it (see *How art is made*)
- ⬜ Arcade ladder with CPU difficulty levels

## M3 — Roster and world

- ⬜ Art Pack v2 for Dee and Bram: the P1/P2 corrections from the [frame review](docs/art/frame-review.md) (Bram's punches and grabs, Dee's uppercut and throw, idle and walk loops, outline pass), then a polish pass

- ⬜ Sable, Idris, Greta, Jun (identities in `setting.md`; a fighter file, atlas and portrait each — no code), with placeholder art first
- ⬜ Final art for Lantern Row and The Crown, and for The Foundry (placeholder in M2); layered versions of the Art Pack v1 venues; new venues (dry dock, ship's hold, the Broker's club)
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
| 2026-10-30 | Sound | Kit placeholders replaced by generated NES-style effects and theme ([ADR-0013](docs/adr/0013-nes-style-generated-audio.md)) |
| 2026-10-30 | Music and voice | One theme per venue; an announcer ([ADR-0014](docs/adr/0014-venue-themes-and-announcer.md)) |
| 2026-10-30 | Art process | Placeholder art and guidelines here, final art made by the owner elsewhere, review and corrections here, then polish; final art replaces placeholders at the vertical slice; Art Pack v2 in M3 |
| 2026-10-30 | Announcer | Keep the eSpeak NG source for now |
| 2026-10-30 | Front end | Scene stack; options saved to `prefs.cfg`; Escape pauses instead of quitting; the Arcade entry stays disabled until the ladder lands ([ADR-0015](docs/adr/0015-front-end-scene-stack.md)) |
| 2026-10-30 | Final art | Art Pack v1 adopted; frames re-registered at one scale, 48 colours per fighter ([ADR-0012](docs/adr/0012-adopting-external-art.md)) |

## Open questions for the project owner

- **Heat's role.** Heat fills fast and its only use, the Breakaway, rarely helps (every attack is already minus on block). Options and a recommendation: [docs/design/heat.md](docs/design/heat.md).
