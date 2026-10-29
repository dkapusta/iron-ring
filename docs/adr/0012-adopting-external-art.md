# ADR-0012: Adopt Art Pack v1, re-registered at one scale per fighter, with sources in ArtSource/

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

Iron Ring Art Pack v1 replaces the procedural fighters, portraits and stages (ADR-0011) with generated pixel art. It meets the file contract (frame names, pivots, partners, palette, timing) and the stages respect the layout rules. Review found three problems:

1. **Per-frame scaling.** The pack resized every raw sprite to the bounding box of the matching procedural frame. Frames were scaled independently: horizontally by 0.31–0.54, vertically by 0.45–0.59 for Dee, and similarly for Bram. On screen the characters grow and shrink between frames, and walk cycles pulse in width (up to 37 % narrower than the generated art). This breaks the asset spec's "one scale for all frames" rule.
2. **Ramp quantisation.** Snapping the art to the procedural material ramps produced blotches (red patches on skin and on Bram's overalls) where the generated tones fall between ramp entries.
3. **Ownership.** The generator and its self-test would overwrite or reject the new files.

## Decision

- Re-import the pack's isolated raw sprites with `Tools/Art/adopt_art_pack.py`:
  - one scale per fighter, from the idle height (Dee 130 px, Bram 134 px)
  - each frame's bottom edge at its old height above the ground
  - grounded frames aligned on the feet; other frames on the old proportional pivot
- Quantise each fighter to **48 colours fitted to its own frames**, instead of the shared ramps; silhouette-edge fringes still go to the ramp outline colours. Stages and portraits are taken from the pack unchanged.
- Keep the editable frames (256 × 256, origin 128,232) in `ArtSource/Fighters/<Name>/frames/`. `Assets/` atlases are packed from them, and `Tools/Art/selftest.py` checks they agree.
- `Tools/Art/ironart/ownership.py` lists the externally authored folders. `generate.py` no longer writes fighters or stages into `Assets/` (procedural versions stay available as pose references), and the self-test skips them in its generator comparison.
- Fit the gameplay boxes where the art moved:
  - Dee's Shin Kick hitbox goes to `[48, 68, 36, 20]`, where the new foot lands.
  - Crouch hurtboxes grow to cover the taller crouching art: Dee 88 → 92, Bram 92 → 100.

## Consequences

- Characters keep a constant size and proportions across every frame, and their reach now matches or exceeds the hitboxes.
- Palette discipline is per fighter rather than global; the asset spec says so. New fighters can follow either route, but one fighter uses one palette.
- About 1.4 MB of sources enter the repository; the 30 MB of generated originals stay outside (ADR-0008).
- The adoption script is specific to the pack's layout. It stays for provenance and re-runs, not as a general tool; general imports go through `import_art.py`.

## Alternatives considered

- **Install the pack as delivered**: the pulsing scale is visible in every walk cycle.
- **Keep the ramp quantisation**: introduces artefacts the generated art doesn't have.
- **Keep all generated colours**: about 75,000 colours per fighter, at odds with the limited-palette style and larger atlases.
