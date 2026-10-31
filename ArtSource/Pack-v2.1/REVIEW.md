# Art Pack v2.1: review

Delivered 2026-10-31 against the [v2.1 correction brief](../../docs/art/art-pack-v2.1-brief.md) and adopted as delivered. It contains nine runtime files: the four crowd layers, the four recomposed stage previews and the Foundry wall. The large generation originals, per-spectator masters, people layers, pose guides and rebuild scripts (about 14 MB) stay outside the repository. The pack's small records are in `Source/`.

## Checked here

| Check | Result |
| --- | --- |
| Scope | Exactly the nine files the brief allows; nothing else in `Assets/` changed |
| Checksums | `CONTENTS_SHA256.txt` matches every file in the pack |
| Format | 768 × 672 RGBA, 8 frames of 84 px, alpha strictly 0 or 255 |
| Barrier | Pixel-identical to v2 in all 8 frames: rows 60–83 at the Foundry, Pier 9 and The Crown; the rope pixels and rows 65–83 at Lantern Row |
| Heads | Rest-frame head tops in rows 26–28 in every 128 px strip; nothing above row 4 |
| Repeats | No repeat at 128, 192 or 256 px; 42 distinct spectators per venue |
| Sign | 191 pixels changed on `foundry_wall.png`, all within rows 54–71, columns 427–464; reads VOLL WORKS at 1× |
| Previews | `compose_previews.py` changes nothing; `selftest.py` passes |
| In the game | Every venue at rest (training) and cheering (after a demo K.O.) at 1280 × 720: heads everywhere, arms on bodies, the barrier and the floor aligned |

## Notes for a later art round (not blocking)

- **Arm length.** Raised arms reach from the shoulder up to row 4, about four head-heights. At 4× they read as long; in play at 1× they read as raised arms.
- **Rim light.** Every venue uses the same dotted warm edge on both sides of each figure. The brief asked for the light from the venue's key direction: from the right at Pier 9, from above at Lantern Row, from behind at The Crown.
- **Gestures shared across venues.** The cheer poses (waving hand, hat on a raised arm, slip) recur across the four venues; 25–48 % of the raised-arm pixels coincide between venues. Only one venue is on screen at a time.
- **Spacing.** The spectators stand evenly in one file with gaps between them. A denser, overlapping second rank would read more like a crowd.
- **Pack hygiene.** `Previews/foundry_in_stage.gif` in the pack is empty (0 bytes). It is not used by the game.
