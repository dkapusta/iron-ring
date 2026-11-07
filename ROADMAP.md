# Roadmap

The guiding rule comes from the concept kit and still holds: **polish one matchup before expanding the roster.** Most of the effort in a sprite fighter is animation, frame data, feel and testing; the engine stays small on purpose.

Milestones are ordered; items inside a milestone are roughly ordered. ✅ done · 🔜 next · ⬜ planned.

## How art is made

1. **Placeholder art and guidelines (here).** New content first ships with placeholder art from `Tools/Art/` and a written contract: the [asset spec](docs/art/asset-spec.md), [prompts](docs/art/generation-prompts.md) and pose references.
2. **Final art (elsewhere).** The project owner generates the final art outside this repository against that contract.
3. **Review and corrections (here).** The art is imported and reviewed in training mode and with `Tools/Art/review_frames.py`, and correction notes are written ([example](docs/art/frame-review.md)).
4. **Polish.** A corrected pack is generated and imported, followed by a final pass.

Final art replaces placeholder art only when a **vertical slice** needs it. That's the end of M2: the complete local loop for one matchup at release quality. Until then, new fighters and venues ship with placeholder art. Art Pack v1 (Dee, Bram, three venues), Art Pack v2 (its corrections, plus the slice's remaining art: champion poses, select portraits, every venue layered, the final Foundry, title logo and menu backdrop) and Art Pack v2.1 (redrawn crowds) are in: the vertical slice is complete. Art Pack v3 (v3A characters, v3A.1 corrections, v3B venues) replaced the M3 placeholders.

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

## M2 — Complete local play ✅

- ✅ Title (with an attract demo), character select (the full roster, unbuilt fighters locked), stage select, options, pause and post-match menus, on a scene stack with fades ([ADR-0015](docs/adr/0015-front-end-scene-stack.md)); a front-end theme and menu sounds
- ✅ Gamepads via SDL_Gamepad: two pads, one layout from `InputManager.json`
- ✅ Remapping (Options → Controls): keys per player, pad buttons per device, saved to `prefs.cfg`; a move list in the pause menu; [docs/controls.md](docs/controls.md) ([ADR-0016](docs/adr/0016-remapping-and-renderer-recreation.md))
- ✅ Persistent settings: `prefs.cfg` in the user folder, between `boot.config` and the command line
- ✅ Renderer recreation when the HDR option, the display's HDR state or the display changes; fullscreen, scaling and vsync change live
- ✅ Stage layers and parallax; animated crowds that react to the fight; camera follow (no zoom: pixel art) on stages wider than the screen; **The Foundry**, the first layered venue, with its own theme ([ADR-0017](docs/adr/0017-layered-stages-and-camera.md))
- ✅ Hit feel: counter hits, combo counts with damage scaling, special cancels; shake and flash (with a reduced option); HDR spark glow; layered hit audio (counter sting, rising combo blip); HUD callouts. Heat reworked (A + B + C of [docs/design/heat.md](docs/design/heat.md)): the Breakaway is a guard cancel that staggers, light attacks pressure on block, Heat lasts the match ([ADR-0018](docs/adr/0018-hit-feel-and-heat.md))
- ✅ Arcade ladder (`Data/arcade.json`: four bouts to the final at The Crown, continues, an ending) against a new CPU with five levels and a difficulty option; `-autoplay` soak test in CI ([ADR-0019](docs/adr/0019-cpu-levels-and-arcade.md))
- ✅ **Vertical slice**: the complete local loop for Dee vs Bram at release quality; final art takes over from placeholder art where the slice needs it (see *How art is made*)
  - ✅ Art brief for Art Pack v2: [docs/art/vertical-slice-art-brief.md](docs/art/vertical-slice-art-brief.md) (frame corrections, the champion pose, select portraits, ending pictures, The Foundry, layered Pier 9 / Lantern Row / The Crown, title logo, menu backdrop)
  - ✅ Engine ready for it: champion animations, optional select portraits and ending pictures, procedural pose references for new frames (`import_art.py refs`)
  - ✅ Art Pack v2 delivered, reviewed and imported: corrected frames and six champion frames per fighter, select portraits, four layered venues on an 800 px world, title logo and menu backdrop; animated champion endings ([ADR-0020](docs/adr/0020-adopting-art-pack-v2.md))
  - ✅ Polish pass: a full playthrough at release quality (both fighters, every venue, the arcade loop), fixing what it turns up; the owner's full arcade playthrough on 2026-10-31 found no further issues
    - ✅ Crowds: the spectator groups the pack cut off at the head are rebuilt from intact ones, in every venue and frame; stage previews recomposed; the art self-test guards both ([ADR-0020](docs/adr/0020-adopting-art-pack-v2.md))
    - ✅ Text over busy art: a backing behind the training-box legend, dark bands behind the arcade ending's lines
  - ✅ Art Pack v2.1: redrawn crowds for the four venues (42 distinct spectators each, heads in every frame, cheering arms with hands) and a legible Foundry sign ([brief](docs/art/art-pack-v2.1-brief.md), [review](ArtSource/Pack-v2.1/REVIEW.md))

## M3 — Roster and world ✅

New fighters arrive with procedural placeholder art and their own frame data, move list and announcer line; final art for them comes in a later art pack, as it did for Dee and Bram ([ADR-0021](docs/adr/0021-adding-fighters.md)).

- ✅ **Sable Reyes**, mobile kickboxer: a low Calf Kick that cancels, the head-high Wire Kick, the Switch Step flying knee (236 + K); placeholder art; in versus, training and the arcade
- ✅ Arcade ladder for a growing roster: each bout lists candidates, and a run meets everyone once before anyone twice; Bram still waits at The Crown
- ✅ `-fighters <p1>,<p2>` for demos, training and tests; the CPU throws kick specials
- ✅ **Idris Okafor**, long-range striker (the Mast Kick push kick, the Bowline Heel 214 + K) and **Greta Voll**, power brawler (the Slag Knuckle, the Ladle Drop 623 + P); placeholder art; Greta meets every challenger at her old Foundry
- ✅ CPU: a special that out-ranges every normal (Idris's heel, Greta's lunge) is thrown as a poke; a CPU-vs-CPU win matrix test for tuning frame data
- ✅ **Jun Arata**, rushdown / feint (the fastest jab, the Lift Step low, the Sleight Feint 236 + P that's +2 on block); placeholder art; he works the Lantern Row crowd. The roster of six is complete
- ✅ Move list on the character select: heavy punch (pad Y) shows the highlighted fighter's moves in that player's keys
- ✅ New venues with placeholder art, layered like the M2 four, each with its own theme: **Dry Dock 4** ("Low Tide"), **The Hold** ("Below Decks", where Idris waits in the arcade), **The Broker's Club** ("Gilt and Smoke", saved for the Broker)
- ✅ Arcade ladder of five bouts: Pier 9, Lantern Row, The Hold, The Foundry, The Crown
- ✅ **The Broker**, the boss (placeholder art): after the title bout, by invitation, at his club; not on the select. Every fighter has a story intro, an ending and three win quotes (`story` in the fighter file, checked against the fonts' charset) ([ADR-0022](docs/adr/0022-boss-and-story.md))
- ✅ Asset streaming: per-stage `sharedassetsN.assets` (layers) + `levelN` (theme), mounted and loaded when a fight uses the stage; the two most recent stay resident, so a session holds about 17 MiB of stage textures instead of 48 ([ADR-0023](docs/adr/0023-stage-streaming.md))
- ✅ Art brief for Art Pack v3, the production art that replaces every M3 placeholder: Sable, Idris, Greta, Jun and the Broker (frames, champion poses, portraits) and Dry Dock 4, The Hold and The Broker's Club ([docs/art/art-pack-v3-brief.md](docs/art/art-pack-v3-brief.md))
- ✅ Art Pack v3A (characters): final frames, champion poses and portraits for Sable, Idris, Greta, Jun and the Broker; adopted as delivered ([ADR-0024](docs/adr/0024-adopting-art-pack-v3a.md), [review](ArtSource/Pack-v3A/REVIEW.md))
  - ✅ v3A.1 corrections: the Broker's skin colour, two of Sable's walk frames, Idris's Mast Kick and Bowline Heel heights, Sable's Wire Kick ([brief](docs/art/art-pack-v3a.1-brief.md))
- ✅ Art Pack v3B (venues): final layered art for Dry Dock 4, The Hold and The Broker's Club; no placeholder art is left in the game ([ADR-0025](docs/adr/0025-adopting-art-pack-v3b.md), [review](ArtSource/Pack-v3B/REVIEW.md))
- ✅ Final art polish (Art Pack v3.1): one portrait backdrop for all fourteen portraits, Greta's and Jun's skin out of their apron and shoe ramps, the chain ring on the three M3 floors, Dry Dock 4's beam and sky, The Hold's lamp ([brief](docs/art/final-polish-brief.md), [review](ArtSource/Pack-v3.1/REVIEW.md))

## M4 — Online ✅

Two players anywhere fight over the internet with rollback netcode. They meet on an Iron Ring Dedicated Server: a room code, or one pasted invite string, puts a friend in your room. Design: [docs/design/online.md](docs/design/online.md); decisions: [ADR-0026](docs/adr/0026-online-play.md).

- ✅ **M4.1 Foundations:**
  - versioned, explicit serialization of the match state (no struct copies on the wire);
  - replay files, saved after every match, and `-replay`;
  - `sim::Rollback` (prediction, re-simulation, input delay, checksums), tested against a simulated network;
  - pinned checksums for every fighter pairing, so all four compilers must agree.
- ✅ **M4.2 Server:**
  - `IronRingNet`: ENet, the protocol, room codes, invites;
  - the `IronRingServer` target: lobby, rooms, relay, referee, `server.cfg`, logs, clean shutdown;
  - the deployment guide `docs/server.md`, with a systemd unit and a Dockerfile;
  - server binaries as CI artifacts.
- ✅ **M4.3 Online front end:**
  - the ONLINE menu, and a text field with keyboard, pad grid, paste and history;
  - the player name, the server lobby and rooms (private by default, OPEN ROOMS opt-in);
  - COPY / PASTE INVITE and `-connect`.
- ✅ **M4.4 Online fights:**
  - online character and stage select, rollback fights through the relay, rematch;
  - event reconciliation, so sounds and effects never fire twice on re-simulation;
  - disconnect and desync handling, the ping and delay HUD, the ONLINE options, `-netsim`.
- ✅ **M4.5 Soak and hardening:**
  - `Tools/Server/soak.sh`, run by CI: a real server and two headless clients fight match after match over a simulated 40 ms / 10 % loss link; the referee must never see a desync, and every saved replay must verify;
  - a load test of 100 refereed rooms (200 players) at once, and tests of every abuse limit (rooms per address, rooms, players, silent connections, message floods);
  - the server drops connections that flood it (`maxMessagesPerSecond`) and tells the one player too many that it is full;
  - `docker-compose.yml`, built and started in CI; `build-server.sh`; the default server in `GameSettings.json`;
  - batch mode draws with SDL's software renderer: a headless run drops from about 1.5 GB to about 140 MB;
  - **HOST ON THIS PC** moved to *Later / maybe*.
- **M4 is closed.** Bugs found in online play from here on are fixed under M5.

## M5 — Release hardening ✅ (1.0.0)

Iron Ring 1.0: Windows, Linux and macOS builds, release archives from one script per platform, and itch.io channels. The macOS builds are being tested by the owner on Intel and Apple silicon Macs; fixes from that go into 1.0.x.

- ✅ Fixes from online play tests (anything found after M4 closed):
  - ✅ the online select marks each side **READY**; the host picks the venue only once both are; the guest sees both fighters and *HOST IS CHOOSING THE STAGE* meanwhile (0.4.2)
  - ✅ a match could hang at the end, waiting for the opponent's last inputs: the server now relays the finished match's tail, and a client takes the referee's result after 3 s (0.4.2; found by the soak test)
  - ✅ crowds that face the ring: Art Pack v3.2 ([brief](docs/art/art-pack-v3.2-crowds-brief.md), [review](ArtSource/Pack-v3.2/REVIEW.md))
  - ✅ the band of old spectators left above the barrier (the v3.2 brief's protected rows were wrong): Art Pack v3.2.1 ([brief](docs/art/art-pack-v3.2.1-crowd-band-brief.md), [review](ArtSource/Pack-v3.2.1/REVIEW.md))

- ✅ macOS: `IronRing.app` (`Contents/Frameworks/UnityPlayer.dylib`, `Contents/Resources/Data`), universal (Apple silicon + Intel), a CI job, notarization in `build-macos.sh` once there is a Developer ID ([ADR-0027](docs/adr/0027-macos-app-bundle.md); 0.4.5)
- ✅ Build scripts for every platform, making the release archives: `build-linux.sh`, `build-windows.sh` (MinGW cross), `build-windows.bat`, `build-macos.sh`, next to `build-server.sh`; CI runs them all ([docs/building.md](docs/building.md); 0.4.5)
- ✅ No default server: the game ships no server address (0.4.5)
- ✅ itch.io: an `.itch.toml` launch manifest in every build, and `Tools/Build/itch-push.sh` to push the windows, linux and mac channels with butler ([docs/building.md](docs/building.md#itchio); 1.0.0)
- ✅ The source package, `IronRing-<version>-source.zip`, from `Tools/Build/source-package.sh` (1.0.0)

Dropped from M5 by the owner (2026-11-07): a crash handler, installers, other stores, localization, accessibility features and performance tooling.

## Later / maybe

- **HOST ON THIS PC**: run a server from the game's ONLINE menu for a LAN or a forwarded port (was M4.5; [docs/design/online.md](docs/design/online.md)).

- A Fatal Fury-style **two-plane** (foreground/background line) mechanic, prototyped as an optional stage rule — only if the single-plane game is already fun.

## Decisions log

| Date | Question | Decision |
| --- | --- | --- |
| 2026-10-29 | Setting | Iron Ring brief as written; kit is visual reference only, art to be generated ([ADR-0007](docs/adr/0007-concept-kit-as-placeholder-content.md)) |
| 2026-10-29 | Names | Renamed for Iron Ring ([setting.md](docs/design/setting.md)) |
| 2026-10-29 | Company name | Keep `dkapusta` |
| 2026-10-29 | Platforms | Windows + Linux; macOS later (M5) |
| 2026-11-07 | macOS | An `.app` bundle laid out like a Unity Mac player, universal, macOS 11+; ad hoc signed until there is a Developer ID ([ADR-0027](docs/adr/0027-macos-app-bundle.md)) |
| 2026-10-29 | MinGW runtime DLLs | Static when the toolchain allows, DLLs copied next to executables otherwise ([ADR-0009](docs/adr/0009-mingw-static-runtime.md)) |
| 2026-10-29 | License | zlib, matching SDL ([LICENSE](LICENSE), [docs/licensing.md](docs/licensing.md)) |
| 2026-10-29 | Desperation moves | None. Heat pays only for the Breakaway |
| 2026-10-29 | Art | Procedural interim art now; final art made elsewhere against the [asset spec](docs/art/asset-spec.md) and imported ([ADR-0011](docs/adr/0011-procedural-interim-art.md)) |
| 2026-10-30 | Sound | Kit placeholders replaced by generated NES-style effects and theme ([ADR-0013](docs/adr/0013-nes-style-generated-audio.md)) |
| 2026-10-30 | Music and voice | One theme per venue; an announcer ([ADR-0014](docs/adr/0014-venue-themes-and-announcer.md)) |
| 2026-10-30 | Art process | Placeholder art and guidelines here, final art made by the owner elsewhere, review and corrections here, then polish; final art replaces placeholders at the vertical slice |
| 2026-10-30 | Art Pack v2 | Brought into the M2 vertical slice: frame corrections, a champion pose per fighter, layered versions of every venue, final Foundry, title logo and menu backdrop ([brief](docs/art/vertical-slice-art-brief.md)) |
| 2026-10-30 | Announcer | Keep the eSpeak NG source for now |
| 2026-10-30 | Front end | Scene stack; options saved to `prefs.cfg`; Escape pauses instead of quitting; the Arcade entry stays disabled until the ladder lands ([ADR-0015](docs/adr/0015-front-end-scene-stack.md)) |
| 2026-10-30 | Heat | A + B + C: Breakaway as a guard cancel, light-attack pressure on block, Heat carries across rounds ([docs/design/heat.md](docs/design/heat.md)) |
| 2026-10-30 | Final art | Art Pack v1 adopted; frames re-registered at one scale, 48 colours per fighter ([ADR-0012](docs/adr/0012-adopting-external-art.md)) |
| 2026-10-31 | Art Pack v2 | Adopted as delivered; the arcade ending animates the champion, the still ending pictures are kept as key art ([ADR-0020](docs/adr/0020-adopting-art-pack-v2.md)) |
| 2026-10-31 | Art Pack v2.1 | Redrawn crowds and the Foundry sign adopted as delivered; the vertical slice and M2 are complete |
| 2026-11-01 | Online (M4) | Through a dedicated server, with rollback; private rooms with codes and an opt-in open list; the server referees; no default server unless `GameSettings.json` ships one; HOST ON THIS PC in M4.5; no spectators, rankings, accounts, server browser or voice ([ADR-0026](docs/adr/0026-online-play.md)) |
| 2026-11-02 | Default server | The game ships `195.161.54.95:7727` as `defaultServer` (the owner's server) |
| 2026-11-02 | M4 closed | HOST ON THIS PC left out of M4.5 (now *Later / maybe*); online bugs found from here on are M5 work |
| 2026-11-01 | Version numbers | `0.<milestone>.<patch>` until 1.0: the minor number is the milestone the build delivers (0.4.0 brings M4's online play), the patch counts fixes on it; **1.0.0** when M5 (release hardening) is done. The version is informational: online compatibility is decided by the protocol number and the game-data hash, replays by their content hash. Set once, in the top-level `CMakeLists.txt` |
| 2026-11-07 | Default server removed | Release builds ship no `defaultServer`: players type a server (the owner's or their own) and the game remembers it |
| 2026-11-07 | 1.0 scope | M5 closes with the platform builds and itch.io; no crash handler, installers, localization, accessibility features or performance tooling. Archives are named by `major.minor` (`IronRing-1.0-…`), with the patch only when it isn't 0 |

## Open questions for the project owner

None at the moment.
