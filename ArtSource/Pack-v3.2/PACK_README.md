# Iron Ring Art Pack v3.2 — Crowds

Crowd-facing correction for source **0.4.2 / f2b83eb**, 7 November 2026.

All seven crowds now face the ring. Shadowed heads sit above visible collars,
lapels, coat openings, apron fronts and hands on the rail. Warm light falls on
front and top planes; the orange silhouette rims have been removed. Cheering
forearms bend across the body, with fists, hats and betting slips held forward.

## Install

Copy this package's `Assets/` directory over the matching directory in the
supplied source package. The seven crowd sheets and seven recomposed stage
previews are ready to use. No code or data changes are required. Rebuild the
game's asset packages as usual, or use its `-assets-folder Assets` override.

## Venue changes

| Venue | Changes |
| --- | --- |
| Pier 9 | Reference pass completed first: dockers in pea coats, caps and hard hats, front lapels and coat buttons, overlapping rows and a child on shoulders. Warm ring-side light with sparse cool accents. |
| Lantern Row | Dense two-row market crowd, traders' aprons, shawls, scarves and cooks in muted whites. A shoulder rider and held boards/slips break the silhouettes; overhead lantern warmth, without rear rims. |
| The Foundry | Flat caps, overalls and leather aprons. Front straps, bib pockets and open coat fronts establish direction beneath restrained warm work lights. |
| The Crown | Gamblers and evening spectators in coats, lapels and ties. Cool dawn hat/shoulder planes, small warm front details and sparse one-sided cool accents. |
| Dry Dock 4 | Riggers in hoods, rain coats and work gear. Warm highlights are stronger toward the left floodlight and subdued farther away. |
| The Hold | Crew in caps, pea coats and oilskins. The warm pool peaks beneath the work lamp near x=365 and falls off away from it. |
| The Broker's Club | Suits, ties, fur collars, homburgs and two cigarette embers. Broader hired heavies stand near the rope ends; subdued gold falls from above. |

## Animation and preservation

- Eight native 768×84 frames per sheet: four rest frames, then four cheering frames.
- Stable spectator positions, clothing and palette; one-pixel idle motion and small cheer bobs, with about half the crowd raising an arm.
- Roughly one third of the spectators hold the rail in the rest loop. Hands also remain visible during cheering.
- Dark faces use ink/night shadow and at most two muted brown chin/cheek pixels; no eyes, teeth or white face details.
- Both four-frame loops close without a larger motion at the seam.
- The original pixels from the specified top rail down are retained in every frame, except the explicit hand overlays on the top rail. No new body extends below it.
- No fighter, portrait, floor, wall, effect, font, HUD, JSON or code is changed.

## Review material

`Previews/<id>_crowd_before_after_4x.png` shows the source above the correction:
rest frame 0 at left, cheer frame 4 at right. These are native-pixel crops at
exact 4× nearest-neighbour scale. Pier 9 is the construction reference for all
six following venues.

Each venue also includes actual 960-tick game captures at 1× and 3×, plus
separate rest/cheer crop GIFs at 4× (four frames, 230 ms each). The game images
use Dee and Bram and have not been retouched.

## Validation

The full project art self-test ends with `art selftest: ok`. Local release and
debug builds pass with warnings treated as errors; all three CTest entries pass
in each configuration. All seven venues were captured at both requested sizes
and checked with fighters present. Details and pixel measurements are in
`VALIDATION.md`; file hashes are in `CONTENTS_SHA256.txt`.

All requested crowd corrections and review deliverables are included. The
protected lower-sheet pixels were deliberately retained, including their
existing highlights. No requested item is deferred.

## Licence

Same zlib terms as the source package; see `LICENSE.txt`. This is an altered
artwork package, not the original 0.4.2 artwork. Tool and input provenance is
recorded in `PROVENANCE.md`.
