# Corrections to Art Pack v2

Changes made in the repository after adoption ([ADR-0020](../../docs/adr/0020-adopting-art-pack-v2.md)). For a later art round, these are also notes on what to avoid.

## Crowds cut off at the head (2026-10-31)

*Superseded the same day by Art Pack v2.1: the crowds were redrawn ([Pack-v2.1/REVIEW.md](../Pack-v2.1/REVIEW.md)) and `repair_crowds.py` was retired. The self-test still checks that every 128 px strip of a crowd shows heads.*

**Found in play.** All four crowd layers (`Assets/Stages/<Venue>/<venue>_crowd.png`) are 768 × 672: six 128 px spectator groups side by side and 8 frames of 84 px. In every venue and every frame, groups 2, 3 and 5 (counting from 0) were a flat band of shoulders with no heads. Their tops sat at row 38, where the intact groups' heads start at rows 30–34. At The Crown the clipped groups left an empty bench. In the cheer frames, the raised arms of those groups hung over bodies that had no heads. The barrier in front of them (rail, rope, bench) was intact. The pack's validation checks sizes and frame counts, not content, so it didn't catch this.

**Repair.** `Tools/Art/repair_crowds.py` fixes it:

- Each clipped group is rebuilt from an intact group of the same venue and the same frame. The donor map is 2 ← 4, 3 ← 0 and 5 ← 1, so the row reads 0 1 4 0 4 1 and no group stands next to itself.
- The original barrier stays in front of the donor bodies.
- Raised arms and head tops move with the group they belong to.
- Arms left hanging without a body under them are removed.
- The four stage previews are recomposed from the repaired layers.

**Guarded.** `Tools/Art/selftest.py` fails if any group is clipped again, or if a preview stops matching its layers.

**For the next pack.** Every spectator should show head and shoulders above the barrier in every frame, along the whole width (asset spec §3.1). The crowd repeats three distinct groups. Six distinct groups per venue would read better at the 0.8 parallax.
