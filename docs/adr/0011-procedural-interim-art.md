# ADR-0011: Procedural interim art, replaceable through a written spec and an importer

- **Status:** Accepted. Fighters, portraits and stages have since been replaced by external art ([ADR-0012](0012-adopting-external-art.md)); effects, fonts, palette and icon remain procedural.
- **Date:** 2026-10-29

## Context

ADR-0007 kept the concept kit's art as placeholders, but it contradicts the setting (sunset festival vs. night ring), and its names and characters belong to another project. The owner asked for art "in the given style, as described in setting.md", and/or a specification so art can be generated elsewhere. No image-generation model is available in the development environment, and generated art from outside tools is inconsistent across frames unless anchored.

## Decision

- **Interim art is procedural**, drawn by `Tools/Art/generate.py` (Python, Pillow, numpy): fighters from a skeleton with 2-bone IK and shaded primitives (capsules, ellipses), coloured outlines and inner contour lines, 4-tone ramps with ordered dithering; stages, effects, fonts and the icon from painting helpers. Output is committed under `Assets/`; the game never runs Python.
- The generator is **deterministic**. `Tools/Art/selftest.py` (run in CI) fails when committed assets differ from its output, so generated files are never edited by hand.
- The **contract** for all art is [docs/art/asset-spec.md](../art/asset-spec.md); procedural art follows it like any other source.
- **Outside art** comes in through `Tools/Art/import_art.py`: pose references are exported from the current atlases on a fixed canvas, generated or drawn frames are imported back with one scale for all frames, keyed, hardened, palette-snapped, cleaned and packed. Prompts and workflow: [docs/art/generation-prompts.md](../art/generation-prompts.md). The self-test proves export → import is lossless.
- Kit images are removed from `Assets/`; kit sounds remain as placeholders. Kit material stays in `docs/concept/` as reference.

## Consequences

- The game shows Iron Ring's own characters and venues now, at a consistent quality, and every frame meets the gameplay contract (hitboxes, pivots, floor line).
- Procedural sprites look stiffer than hand-drawn ones; this is accepted as interim. They double as pose references, so the final art inherits correct proportions, timings and hit points.
- Art can be replaced fighter by fighter; a replaced fighter is dropped from regeneration.
- Python with Pillow, numpy and scipy becomes a tool dependency (not a build dependency).

## Alternatives considered

- **Keep the kit art until final art exists**: contradicts the setting and the renaming.
- **Hand-pixel everything now**: high quality, but far beyond the scope of an engineering milestone.
- **Specification only, no interim art**: leaves the game unrepresentative and the pipeline untested end to end.
