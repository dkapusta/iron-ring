# ADR-0016: Remap controls per player and per pad; recreate the renderer for HDR

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

[ADR-0015](0015-front-end-scene-stack.md) left two things fixed: the bindings (only `InputManager.json`), and HDR (chosen once at launch, because textures belong to a renderer). Player 2's default keys need a numeric keypad that many laptops lack; pads differ in layout; and a display can switch HDR on while the game runs.

## Decision

**Bindings**

- `Core/InputBindings` holds the bindings by name: SDL scancode names for keys, SDL gamepad names for pad buttons and triggers. It has no SDL, so its rules are unit-tested.
- Keyboard bindings are **per player**.
- Pad buttons are **per device**, keyed by the pad's GUID, so a pad keeps its layout in either slot. Directions on a pad are always the d-pad and the left stick.
- Binding a key takes it away from every other action of both players. The screen names the action that lost it, so one key never does two things.
- `prefs.cfg` stores only the differences from `InputManager.json`, for example `p2.key.lightPunch=J` and `pad.<guid>.guard=leftshoulder`. A lone `|` means deliberately unbound.
- **Options → Controls** rebinds: choose an action, then press a key or a pad button. Escape cancels; a press that ends a capture never also acts as a menu input.
  - Escape and the F-keys can't be bound.
  - On pads, Start and the d-pad can't be bound.
- Menus follow the bindings: light punch confirms, light kick backs out.

**Move list**

- The pause menu shows both fighters' moves from their definitions.
- Motions are drawn with arrow glyphs (font code points 0x11–0x19, numpad direction + 0x10), mirrored for the way each side faces at that moment.
- Buttons are shown as that player's keys. The CPU's side shows LP/HP/LK/HK.

**Renderer recreation**

- `AssetLibrary` can release every texture and decode them again for a new renderer. `Texture` objects keep their addresses, so fonts and sprite sheets stay valid.
- `Display::recreateRenderer` makes a new renderer for the same window from the current settings.
- It runs when the HDR option changes, and when the window's display reports HDR (or the window moves to another display) while `hdr=auto` and output is still SDR.

## Consequences

- P2 is playable on any keyboard; every change survives a restart.
- Recreation decodes every loaded texture again (tens of milliseconds for the current content). That is acceptable for a settings change, not for every frame.
- Every texture must come from `AssetLibrary`. A future render target or streaming texture must register there too, or it will dangle after a recreation.
- Pad remapping is exercised by unit tests and code review only: CI has no pads.

## Alternatives considered

- **One pad layout per slot:** swapping two pads would swap their layouts.
- **Rebuild the whole scene stack after a renderer change:** loses the player's place; reloading textures in place is invisible.
- **Several keys per action in the UI:** the file format allows it (`InputManager.json` lists), but one key per action keeps the screen and the conflict rule simple.
