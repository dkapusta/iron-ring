# Validation — Crowd Band v3.2.1

Source: IronRing 0.4.3 / fee247c. Baseline: its integrated v3.2 crowd sheets.

## Pixel results

| Venue | Band | Colours | Orange edges, before → after | Dotted runs, before → after | Barrier differences outside hands |
| --- | --- | ---: | ---: | ---: | ---: |
| pier_nine | 49–59 | 17 | 180 → 0 | 245 → 0 | 0 |
| lantern_row | 49–60 | 19 | 194 → 0 | 315 → 0 | 0 |
| foundry | 48–59 | 15 | 229 → 0 | 364 → 0 | 0 |
| the_crown | 48–59 | 12 | 416 → 0 | 761 → 0 | 0 |
| dry_dock | 50–67 | 15 | 336 → 0 | 613 → 0 | 0 |
| ship_hold | 49–69 | 15 | 284 → 0 | 1051 → 0 | 0 |
| brokers_club | 54–67 | 16 | 319 → 0 | 314 → 0 | 0 |

Counts are summed over all eight frames. Orange edges are `sodium_2` pixels adjacent to transparent pixels in the four cardinal directions. Dotted runs are three consecutive warm pixel groups (`sodium_1` or `sodium_2`) in one column, separated by 1–3 non-warm pixels. Both checks cover every row above the specified solid-barrier row: 60, 61, 60, 60, 68, 70 and 68 respectively.

Hardware was identified using the existing Lantern Row rope/post mask and the three Pack-v3B barrier masters, plus the fixed rail/parapet and protected lower regions. The original source pixels were retained. The comparison excludes only the old and relocated hand footprints, including restoration of hardware previously covered by a hand.

Upper artwork and protected lower rows also compare byte-exact outside those hand footprints. Lantern Row grips follow the exact rope/post mask rather than a guessed constant row; its rope contour includes row 62. The hand overlap is the only change below the specified band there.

All sheets: 768×672 RGBA; eight 84-pixel frames; alpha only 0/255; all opaque RGB values from the source palette; 12–19 colours; head/shoulder coverage above row 37 in every 128-pixel strip; no new pixels above row 2. The upper spectator shapes and clothes remain the accepted v3.2 pixels.

## Animation

Changed pixels within the band for each adjacent frame, including the loop seam:

| Venue | Rest: 0→1, 1→2, 2→3, 3→0 | Cheer: 4→5, 5→6, 6→7, 7→4 |
| --- | --- | --- |
| pier_nine | 279, 143, 279, 143 | 470, 472, 473, 471 |
| lantern_row | 209, 154, 209, 154 | 517, 455, 526, 448 |
| foundry | 217, 106, 217, 106 | 423, 401, 432, 392 |
| the_crown | 136, 251, 136, 251 | 340, 433, 332, 445 |
| dry_dock | 263, 85, 263, 85 | 524, 391, 523, 406 |
| ship_hold | 331, 169, 331, 169 | 610, 600, 611, 599 |
| brokers_club | 237, 105, 237, 105 | 433, 380, 427, 368 |

The lower-body construction uses each original spectator’s phase and bob, with fixed world positions and costume colours. Lower folds and buttons move with the torso. Hands remain fixed on the barrier. No wrap transition exceeds the other transitions by a material jump.

## Project and runtime checks

- Source ZIP comparison: exactly seven crowd PNGs and seven stage-preview PNGs changed; all other supplied files match.
- `python3 Tools/Art/compose_previews.py`: seven previews recomposed.
- `python3 Tools/Art/selftest.py`: `art selftest: ok`.
- Release and debug builds: pass; `IR_WARNINGS_AS_ERRORS=ON` in both configurations.
- Release CTest: 3/3 pass; debug CTest: 3/3 pass. Entries: IronRingTests, AssetPackerBakesProject and IronRingServerCheck.
- Debug smoke: 420 frames at 1920×1080, exit 0, image inspected.
- Release captures: all seven stages, 960 frames, Dee/Bram, at 640×360 and 1920×1080. All 14 runs exit 0. The player reports version 0.4.3 and loads the delivered loose assets.

The local Linux build used `SDL_UNIX_CONSOLE_BUILD=ON` and the existing software batch renderer with dummy video/audio drivers. No engine code or game data was changed. These are local build/test results, not remote CI results.

Capture command, repeated per stage and resolution:

```sh
./IronRing.x86_64 -batchmode -demo -stage <id> -fighters dee,bram -frames 960 \
  -screen-fullscreen 0 -screen-width 1920 -screen-height 1080 \
  -assets-folder <source>/Assets -screenshot <id>_ingame_3x.png
```

Visual review confirms continuous clothing down to the barrier, relocated grips, dark lower bodies visible through the open barriers, and fighters remaining brighter than the spectators.
