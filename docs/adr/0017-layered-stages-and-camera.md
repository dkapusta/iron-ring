# ADR-0017: Layered stages wider than the screen, a follow camera, no zoom

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

Every stage was one flattened 640 × 360 image, and the simulation's walls sat at x 30 and 610 of the screen. M2 asks for stage layers, parallax, animated crowds, camera follow and zoom limits. The Art Pack v1 venues are flat, and splitting them by rows would tear their crane legs and pillars across layers moving at different speeds.

## Decision

**Stage width belongs to the simulation.**
- `Round::stageWidth` defaults to 640 and is set from the stage. `reset()` keeps it.
- The walls stay 30 px in from each end, and the round starts 90 px either side of the centre.
- The fighters can never be more than 580 px apart, so both always fit in the 640 px view (as with the first fighting games that scrolled).
- The width enters the checksum only when it isn't 640, so the existing replays and golden checksums stand.

**Layers are data.**
- `stages.json` gives a stage a `width` and `layers` (`content::StageDefinition`): parallax factor, position, drift, vertical animation frames, a `crowd` flag, a `front` flag.
- `image` stays as the flat stage, or as a 640 × 360 preview.
- AssetPacker checks that every layer covers the whole camera travel, using `640 + ceil((width − 640) × parallax) − x`, and that frames divide the image.

**Presentation.**
- `Camera` (Core, unit-tested) eases towards the fighters' midpoint, stays on the stage, and always keeps both fighters at least 24 px inside the edges.
- It is drawn at whole pixels. Everything in world space (fighters, shadows, effects, training boxes) takes the same offset; the HUD doesn't.
- `StageView` draws the layers and keeps the crowd's excitement. Hits, throws, Breakaways and K.O.s raise it, it halves in about 1.5 s, and crowd layers bob faster with it and switch to cheering frames above 0.45. This is presentation only: the simulation never sees it.

**No zoom.**
- The zoom limit is 1×.
- Non-integer scaling of 640 × 360 pixel art shimmers. Integer zoom (2×) would show a 320 px slice of the stage, too little for two 120 px fighters.
- Wider stages and a scrolling camera give the sense of space instead.

**Content.**
- The Art Pack v1 venues stay flat and render exactly as before.
- The first layered venue is a new one from the M3 list: **The Foundry**. It has procedural placeholder art (`generate.py foundry`, six layers at 800 px) and its own theme, as agreed for placeholder art.
- Final art, and layered versions of the other venues, come with the next stage art pass.

## Consequences

- New venues can scroll; old ones are untouched.
- CPU behaviour and every test on 640 px stages are unchanged.
- The camera and the crowd are presentation, so replays and checksums don't depend on them.
- Effects, overlays and anything else drawn in world space must take the camera offset. Forgetting it is the easy mistake (both current ones take it).
- Layered stages need more art. The spec (asset-spec §3.1), generation prompts and the placeholder give the sizes exactly.

## Alternatives considered

- **Split the flat v1 images into rows:** vertical structures tear apart.
- **A camera-only stage of 640 px with zoom:** no room to move without zooming, and zooming pixel art shimmers.
- **Crowd excitement in the simulation:** it would change checksums for no gameplay effect.
