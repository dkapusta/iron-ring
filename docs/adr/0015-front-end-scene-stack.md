# ADR-0015: Build the front end on a scene stack, with saved options in prefs.cfg

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

Up to M1 the player ran one `FightScene` that loaded every sound, theme and stage itself, and keys like Tab and F2 stood in for menus. M2 needs a title, character and stage select, options and a pause menu; several of them sit on top of each other (pause over a fight, options over pause) and must return to where the player came from. Options must also survive a restart, and pads must work alongside the keyboard.

## Decision

- **Scene stack.** `SceneStack` holds the screens: title at the bottom, then character select, stage select (replaced by the fight once a venue is picked), pause or post-match menu. Scenes ask for `push`, `pop`, `popTo(depth)` or `replaceTop`; the stack applies the request after the call that made it, never under a running scene. The first request in a frame wins, so a double press can't push twice. `Fade` transitions dim to black over 9 ticks, swap, and fade back; input is ignored meanwhile. Overlays (`opaque() == false`) draw over the scene beneath. `onEnter()` runs when a scene becomes the top one again, which is where the music changes.
- **Shared content, loaded once.** `GameData` reads `GameSettings.json` at start-up: fonts, sounds, announcer lines, themes, stages, the roster (`Data/roster.json`; fighters without a definition appear as locked slots) and the menu art. Scenes get a `Services` bundle (assets, audio, display, controls, settings, prefs, data, stack) instead of loading anything themselves. A fight is described by a `FightSetup` (fighters, stage, control, training, attract).
- **Menu actions.** `Controls` replaces `InputMapper`: keyboard bindings for both players plus the first two gamepads (`SDL_Gamepad`, one shared layout from `InputManager.json → gamepad`), producing simulation bits and menu actions (`Up/Down/Left/Right/Confirm/Back/Pause`, tagged with the player). On the panel, light punch confirms and light kick backs out; Return, Backspace, Escape and the pad's south/east/Start do the same. One-player screens accept either player's keys; the 2-player select gives each side its own.
- **Menu logic in Core.** `Core/Menu` (focus skipping disabled items, settings with wrapping or clamped choices, grid cursor movement) has no SDL, so it is unit-tested; `MenuView` adds the sounds and drawing.
- **prefs.cfg.** Options are saved to `<user folder>/prefs.cfg` in `boot.config` syntax with the same keys (`music-volume=0.80`, `vsync=0`), written atomically when the Options screen closes. The order is defaults < `boot.config` < `prefs.cfg` < command line. Batch runs ignore it, so CI and screenshots don't depend on the machine. Non-setting keys (`last-stage`) live in the same file.
- **Live where possible.** Volumes, fullscreen, scaling and vsync apply at once; HDR is stored and applies on the next launch until renderer recreation lands (ROADMAP M2).
- **Batch-testable.** `-scene <name>` opens a screen directly; `-keys "frame:key,..."` taps keys at given frames, so CI drives the title → select → stage → fight → pause path and uploads the frame.

## Consequences

- New screens are a class and a `push`; nothing else changes. Scenes can't free themselves mid-call by accident.
- The fight no longer owns audio loading: one decode at start-up instead of one per match, and the menu theme carries on across screens (`AudioSystem::playMusic` ignores the theme already playing).
- The command line still goes straight into a fight (`-demo`, `-training`, `-stage`), now with the title beneath, so the old smoke tests and the README capture are unchanged.
- Escape no longer quits; it pauses or backs out. Quitting is the title's Quit, or closing the window.
- Remapping, per-device profiles and pad hot-swap UI are not here yet: the layout is fixed by `InputManager.json`.

## Alternatives considered

- **A state enum in one "game" scene:** every screen's state in one class; overlays and returns become special cases.
- **Immediate push/pop:** a scene popping itself while running is a use-after-free waiting to happen.
- **JSON or the registry for prefs:** a second syntax and a second parser for the same keys `boot.config` already has.
- **Per-player pad layouts now:** needs the remapping UI to be useful; it comes with it.
