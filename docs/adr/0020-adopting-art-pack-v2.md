# ADR-0020: Adopt Art Pack v2 as delivered, with animated champion endings

- **Status:** Accepted
- **Date:** 2026-10-31

## Context

Iron Ring Art Pack v2 answers the [vertical-slice art brief](../art/vertical-slice-art-brief.md). It contains:

- corrected and polished frame sets for Dee (55 frames) and Bram (56), each with six champion frames;
- 48 × 48 select portraits and 640 × 360 ending pictures;
- final layered art for The Foundry and new layered versions of Pier 9, Lantern Row and The Crown (25 layers, each on an 800 px world);
- the title logo and the menu backdrop;
- integration files: `stages.json`, the fighter JSON, an ownership patch, review reports.

Review against the brief and in the game found:

1. **Fighters meet the contract.** The frames are 256 × 256 with the fixed origin (128,232), binary alpha and one scale per fighter. Dee has 47 colours, Bram 48. `import_art.py` packs the frames into atlases pixel-identical to the pack's own. `review_frames.py` reports no findings. In training (F3) the hit, hurt and grab boxes sit on the new limbs, so no box or frame data changes.
2. **Stages meet the width rule.** AssetPacker validation passes. The feet, the chain ring (world 400,326) and the HUD bands hold at both camera ends. The `front` layers stay at the stage ends.
3. **The ending pictures overlap the champion animation.** Each one is The Crown with the champion pose painted in. With `ending` set, `ArcadeScene` shows the still instead of the six-frame animation. Drawing the animation over the picture would show the champion twice.
4. **The pack's `stages.json` reorders the venues** (Foundry first) and drops the file comment. The stage select, F2 in a fight and the attract demos follow file order (the arcade ladder names its venues itself).

## Decision

- Adopt the frames, select portraits, stage layers, previews and UI pictures unchanged. Repack the atlases with `import_art.py` from `ArtSource/Fighters/<Name>/frames/`.
- Fighter JSON: change only the champion animations (frames `champion_0..2` for the intro, `champion_3..5` for the loop) and add `selectPortrait`. No `ending` field: the arcade ending is the animated champion over the dimmed final venue.
- Keep the ending pictures and the pack's "still ending" JSON variants in `ArtSource/Pack-v2/Endings/`, out of the game. They can serve as key art. Switching to them takes one JSON field.
- `stages.json`: take the pack's layer lists, but keep the repository's venue order and the file comment.
- Ownership (from the pack's patch): `Stages/Foundry` and the two front-end pictures become external. `generate.py` stops writing the `foundry` and `ui` parts into `Assets/`, and the self-test skips them. The procedural versions remain as references.
- Provenance: `ArtSource/Pack-v2/` holds the pack's notes and reports, plus its small source records (palettes, registration, generation jobs), about 0.5 MB. The 44 MB of generation originals, the previews and the pack's rebuild scripts stay outside the repository ([ADR-0008](0008-repository-and-binary-assets.md)).

## Consequences

- Every venue is layered, and the four share one stack contract (asset spec §3.1). No flat stages remain in the game, though the engine still supports them.
- The procedural art now owns effects, fonts, the HUD layout, the icon and the palette files only.
- The simulation is untouched, so the golden checksum holds.
- *Later correction (polish pass):* three of the six spectator groups in every crowd layer were cut off at the head. `Tools/Art/repair_crowds.py` rebuilt them from the intact groups, and the art self-test now checks the crowds and that each preview matches its layers ([CORRECTIONS.md](../../ArtSource/Pack-v2/CORRECTIONS.md)). Art Pack v2.1 then replaced the four crowd layers with redrawn ones and made the Foundry's sign legible; the repair tool is retired, and the self-test's crowd check now reads every crowd layer listed in `stages.json` ([Pack-v2.1/REVIEW.md](../../ArtSource/Pack-v2.1/REVIEW.md)).
- The crouch hurtboxes stay at Dee 92 and Bram 100, the values from ADR-0012. The brief quoted the older 88 and 92; the pack drew to the real values.

## Alternatives considered

- **Use the still endings, as the brief allowed:** loses the animation the champion frames were drawn for.
- **Animate over the still:** shows the champion twice. A champion-free version of the ending picture would allow it later.
- **Take the pack's `stages.json` verbatim:** would reorder the stage select and the attract demos for no gain.
