# Art Pack v3.1 (final polish): review

**Reviewed:** 2026-11-01, against the [final polish brief](../../docs/art/final-polish-brief.md), in the game and with the project tools.
**Verdict:** adopted. P2–P7 are delivered and meet the brief. The project made two edits, both mechanical:
- **P1, done here.** The pack left it pending because its source package predated the templates.
- **P6's light pool** given a falloff.

With this, the polish round is closed.

## Item by item

| # | Item | Result |
| --- | --- | --- |
| P1 | One portrait backdrop | **Applied by the project** with the brief's templates; details below. |
| P2 | Greta's skin | **Fixed.** Her face, neck and arms use her skin ramp, and skin and apron now read as two materials. Apron tones remain only on the apron, gloves and boots: in their 20 head rows, the median frame has 16 such pixels, all on apron straps; `down_1` and the champion frames have more, from the apron itself. The alpha of all 55 frames is unchanged. |
| P3 | Jun's skin and hair | **Fixed.** His skin and hair use his skin ramp and black hair ramp. Shoe red is left only on the high-tops: the median frame has 0 red pixels above y 200, and the kick and jump frames count the raised shoe. The alpha is unchanged. |
| P4 | Chain ring on the M3 floors | **Fixed.** Dry Dock 4, The Hold and The Broker's Club now draw lit, countable links. They match Pier 9's ring in the stacked floors and in the game. |
| P5 | Dry Dock 4 | **Done.** The beam is a faint dithered wedge from the lamp, and the sky's transition band is out of the HUD band. |
| P6 | The Hold's lamp | **Delivered, then adjusted.** The caged lamp (9 × 12) swings with its pool across both frames. The pool's outer tone was a uniform 4 px dot lattice filling a rectangle about x 248–460, y 128–252, with hard vertical edges, so it read as a lit panel. The project thinned that outer tone with an ordered falloff from the pool's centre (790 and 986 pixels removed in the two frames) and kept the inner tone and the lamp as delivered. The pool now fades radially. |
| P7 | Specks and stubs | **Fixed.** Dee's shoulder speck and Bram's scalp speck are shaded in, and five stray pixels at Sable's ankles in `walk_3` are removed: her only alpha change. |

## P1, applied by the project

`Tools/Art/portrait_backdrop.py` (new) puts each portrait on `docs/art/templates/portrait_backdrop_<size>.png`. Every pixel outside the figure becomes the template's pixel. The figure is whatever the backdrop doesn't reach, and the backdrop is made up of:
- the `night_1` area connected to the top and side edges;
- the old decoration inside it: v3A's dot grid, and Dee's and Bram's corner smudge;
- the few decoration dots left against hair outlines (37 pixels across four portraits).

The select portraits' side-colour bands are kept. All 13 portraits were reviewed at 4× before and after: no figure pixel was lost, and the six select cells and every HUD pairing now share the same corner.

The art self-test now checks each portrait's top-left corner against the template (4 px on HUD portraits, 6 px on select portraits), so a portrait on a different backdrop fails it. Re-running the tool on the result changes nothing.

## Checks

- **Tools:** `review_frames.py` reports 0 findings for Greta, Jun and Sable, and `selftest.py` passes, including the new portrait check. `compose_previews.py` reproduces the delivered previews; the Hold's preview is recomposed after the P6 adjustment.
- **Build:** AssetPacker and `ctest` pass, and the simulation is unchanged.
- **In the game:**
  - the select screen;
  - a Jun vs Greta demo in The Hold;
  - a Sable vs the Broker demo at Dry Dock 4.

## Kept

- **This folder:** the pack's README, provenance, licence, checksums and its checks, including the per-frame recolour counts.
- **Outside the repository:** the previews (before/after sheets).
