# Iron Ring: crowd band brief (Art Pack v3.2.1)

**For:** whoever produced Art Pack v3.2.
**From:** the Iron Ring project, 2026-11-07.
**With:**
- the current Iron Ring source package, which includes v3.2 as delivered;
- the reference image `docs/art/crowd-reference/crowd_v3.2_legacy_band.png`.

Thank you for v3.2: the crowds face the ring now, and the pack is in the game ([review](../../ArtSource/Pack-v3.2/REVIEW.md)). This is a small follow-up to fix **a mistake in our brief**.

## 1. The problem

The v3.2 brief protected each sheet "from the top rail down" and gave the top rail as the first row that is opaque across the width. That row was wrong. It is where the **old** spectators' coats merged into one band, not the barrier. So between your new figures and the real barrier, the old lower bodies are still there, with their dotted `sodium_2` and `sodium_1` rims. In the game it shows as:
- a band of dark torsos with vertical orange dots under the crowd;
- hands "on the rail" that sit mid-torso, above the real barrier.

`crowd_v3.2_legacy_band.png` marks the band (magenta) in frame 0 of each sheet, at 4×.

## 2. What to deliver

The seven crowd sheets again, with **only the band repainted**, and their seven recomposed previews:

| Venue | File | Band to repaint | The barrier, kept exactly |
| --- | --- | --- | --- |
| Pier 9 | `PierNine/pier_nine_crowd.png` | rows 49–59 | the steel rail from row 60, its posts and everything below |
| Lantern Row | `LanternRow/lantern_row_crowd.png` | rows 49–60 | the red rope and the brass posts (with their caps and highlights) |
| The Foundry | `Foundry/foundry_crowd.png` | rows 48–59 | the steel rail from row 60 and below |
| The Crown | `TheCrown/the_crown_crowd.png` | rows 48–59 | the parapet from row 60 and below |
| Dry Dock 4 | `DryDock/dry_dock_crowd.png` | rows 50–67, everything that isn't post or chain | the posts, the slack chain, and the wall from row 68 |
| The Hold | `ShipHold/ship_hold_crowd.png` | rows 49–69, behind the fence | the chain-link mesh, its posts and top rail, and everything from row 70 |
| The Broker's Club | `BrokersClub/brokers_club_crowd.png` | rows 54–67, everything that isn't post or rope | the posts, the red rope, and the base from row 68 |

**In the band:**
1. **Continue your figures down to the barrier.** The coats, aprons, overalls and trousers of the figures above carry on, in the same construction: lit front planes and no outer rims. They go darker toward the barrier, the way a crowd falls into shadow behind a rail. Where the barrier is see-through (chain, fence, rope), the bodies show through it, drawn behind it.
2. **Move the rail hands to the real barrier.**
   - Pier 9, the Foundry and the Crown: the top of the rail or parapet (row 60).
   - Lantern Row and the Club: the rope.
   - Dry Dock 4: the chain or a post.
   - The Hold: the fence's top rail.

   Remove the hands at the old row. The forearms reach down to the new grip.
3. **No old pixels left** in the band. There are no `sodium_2` pixels on outer edges and no dotted vertical runs, in any frame.

**Unchanged:** everything above the band, and every barrier pixel (rails, ropes, chains, posts, fences and walls), byte-exact in all 8 frames. The only exceptions are the hands, which may cover the barrier's top edge as before.

All other rules of the [v3.2 brief](art-pack-v3.2-crowds-brief.md) §4–§6 still apply: the sheet format, the 8 frames, stable people, the palette, at most 28 colours, contrast below the fighters, and seamless loops. In the cheer frames, the bodies in the band move with their figures: a 1–2 px bob.

## 3. Check before delivering

1. `python3 Tools/Art/compose_previews.py`, then `python3 Tools/Art/selftest.py` ends with `art selftest: ok`.
2. **Barrier:** for each sheet, a diff against v3.2 is zero on every barrier pixel, apart from the hands.
3. **No rims:** no `sodium_2` pixel next to transparency, and no vertical run of 3 or more alternating `sodium` pixels, anywhere above each venue's solid barrier row (60, 61, 60, 60, 68, 70, 68).
4. **In the game:** capture each venue (the v3.2 brief, §7). The crowd should run down unbroken to the barrier, with hands on it.

## 4. Delivery

One ZIP, **`IronRing_ArtPack_v3.2.1_CrowdBand.zip`**, in the same layout as v3.2:
- `README.md`, `CONTENTS_SHA256.txt`;
- `Assets/Stages/<Dir>/<id>_crowd.png` and `<id>.png` for all seven;
- `Previews/<id>_band_before_after_4x.png`, rows 30–84 of frames 0 and 4, v3.2 above v3.2.1;
- one in-game capture per venue.

Same licence (zlib). Add a `PROVENANCE.md` only if the method differs from v3.2.
