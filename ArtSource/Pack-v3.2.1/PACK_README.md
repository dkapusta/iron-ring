# Iron Ring Art Pack v3.2.1 — Crowd Band

Correction for the supplied **0.4.3 / fee247c** source snapshot, 7 November 2026.

The legacy lower-body band has been replaced in all seven crowd sheets.
The accepted v3.2 figures now continue down to the actual barriers: coat and
apron fronts become darker toward the rail, and bodies remain visible behind
ropes, chains and mesh. Hands grip the real hardware; the old waist-height
hand placements are removed and the forearms extend to the new grips.

## Install

Copy `Assets/` over the matching directory in the supplied source project.
The package contains exactly the seven corrected crowd sheets and seven
recomposed stage previews. Rebuild the asset packages, or use the player's
`-assets-folder Assets` option to inspect the loose assets.

## Corrections by venue

| Venue | Repainted band, inclusive | Result and preserved barrier |
| --- | --- | --- |
| Pier 9 | 49–59 | Pea coats and workwear extend to the steel rail; hands rest on row 60. Rail, posts and all lower pixels are preserved outside the hand footprints. |
| Lantern Row | 49–60 | Aprons, shawls and coats continue to the rope. Grips follow its actual contour; brass posts, caps, highlights and rope pixels are preserved. |
| The Foundry | 48–59 | Overalls and leather aprons continue to the steel rail at row 60, with lowered grips and dark lower folds. |
| The Crown | 48–59 | Coats and evening wear continue to the parapet at row 60; hands rest on its coping. The old orange dotted torsos are removed. |
| Dry Dock 4 | 50–67 | Rain gear continues behind the slack chain. Hands meet the chain or post, with the stanchions, links and wall from row 68 preserved. |
| The Hold | 49–69 | Crew coats and oilskins continue behind the mesh. Grips meet the fence's top edge; the mesh, posts and all pixels from row 70 are preserved. |
| The Broker's Club | 54–67 | Suit fronts and coats continue behind the red rope. Grips follow the rope; brass posts, rope and base from row 68 are preserved. |

The whole band was reconstructed from the existing v3.2 spectator designs,
not recoloured from the legacy pixels. Original barrier ownership masks kept
hardware separate from clothing. The upper figures are byte-exact to v3.2
apart from the explicitly requested hand removal/relocation. No other artwork,
game data or code is included or changed.

The same eight-frame layout and spectator identities are retained. Lower
folds, openings and buttons follow the existing 1–2 pixel cheer bob; rail
hands remain anchored. Both four-frame loops close without a seam jump.

## Review files

- `<id>_band_before_after_4x.png`: the entire 768-pixel width, rows 30–83, at exact 4×. Frame 0 is left, frame 4 is right; v3.2 is above v3.2.1.
- `<id>_band_detail_before_after_4x.png`: the same comparison with a 200-pixel detail crop for easier inspection.
- `<id>_ingame_1x.png` and `<id>_ingame_3x.png`: actual unretouched 960-frame game captures with Dee and Bram.
- `<id>_rest_band_4x.gif` and `<id>_cheer_band_4x.gif`: separate four-frame loops, 230 ms per frame.

## Validation

All seven sheets pass the band audit: zero orange silhouette-edge pixels,
zero dotted sodium runs above the specified solid barrier row, and zero
hardware differences outside the old/new hand footprints. Each remains
768×672 RGBA with binary alpha and at most 19 colours. Only the requested 14
production PNGs differ from the supplied source ZIP.

The project art self-test passes. Local release and debug builds pass with
warnings treated as errors, and all three CTest entries pass in each.
All seven venues were checked in-game at 1× and 3×. See `VALIDATION.md` for
measurements and `CONTENTS_SHA256.txt` for the file hashes.

All requested corrections are complete; no item is deferred.

## Method and licence

The native-pixel Python/Pillow/NumPy authoring method is unchanged from v3.2.
No new generated reference, stock artwork or outside image was used, so no
new provenance document is required. The source package retains v3.2's
provenance. This is an altered artwork package under the same zlib terms;
see `LICENSE.txt`.
