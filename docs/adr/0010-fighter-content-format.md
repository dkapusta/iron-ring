# ADR-0010: Fighters are data: one definition file per fighter, with an animation timing contract

- **Status:** Accepted
- **Date:** 2026-10-29

## Context

The kit hard-coded two fighters in C++ (`kitBoxer()`, `kitGrappler()`), and frame data lived next to pose indices in a separate `moves.json`. Adding throws, specials and knockdowns, and swapping the art for generated sprites, would have meant touching simulation code for every content change. Art and gameplay also had no shared definition of "how long is this animation", so an art change could quietly desynchronise visuals from hitboxes.

## Decision

- Each fighter is `Assets/Fighters/<Name>/<id>.fighter.json`: identity, physics, hurtboxes, six move slots (LP, HP, LK, HK, throw, special) and named animations. `GameSettings.json → fighters` lists them. Schema: [asset-spec §2.5](../art/asset-spec.md#25-fighter-definition-idfighterjson).
- The simulation receives a plain `sim::FighterSpec` (integers only). Parsing lives in a separate, SDL-free **Content** library (`Source/Content`) that the runtime, AssetPacker and tests share; the simulation does not know JSON exists.
- Sprites come from a separate atlas (`<id>_atlas.json`) with per-frame pivots; animations reference frames by name. Throw holds reference the opponent's `held_*` frame and a per-frame `partner` offset, so either fighter can be thrown by any other.
- **Timing contract**, enforced at build time by AssetPacker (`content::validateProject`): a move's animation lasts exactly `startup + active + recovery`; a throw's hold animation lasts `throwDuration`; `knockdown` lasts `knockdownTicks`; every referenced frame exists; required animations are present. Violations fail the build.
- Frame timing is data, not art. Designers retime moves without new art; artists replace art without touching timing.
- The kit's two fighters remain as `kitBoxer()`/`kitGrappler()` for the original golden-checksum test, with throws and specials disabled.

## Consequences

- New fighters need no C++: a definition, an atlas and a portrait.
- Content errors surface in the build (and in `ContentTests`), not at runtime.
- The simulation stays deterministic and testable without assets; tests also run the shipped data through a full match.
- Specials are limited to what `MoveSpec` can express (motion + button, strike or command throw). Projectiles, multi-hit moves and cancels will need schema and simulation changes — deliberately deferred.
- No desperation/super moves (owner decision, 2026-10-29): Heat only pays for the Breakaway.

## Alternatives considered

- **Keep fighters in C++**: every balance tweak needs a rebuild and a programmer.
- **One big moves.json for all fighters** (the kit's layout): couples unrelated fighters and splits a fighter's data across files.
- **Derive move timing from the art** (frame counts × fixed duration): ties balance to drawing count and breaks on every art pass.
