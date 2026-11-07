# Validation — Art Pack v3.2 Crowds

## Asset checks

All seven PNGs are RGBA, 768×672, with eight vertical 84-pixel frames. Alpha is strictly 0 or 255. All opaque colours belong to the supplied palette (including the existing metal ramp). The limit is 28 colours; the delivered range is 14–20.

| Stage | Colours | People | Rail holders (rest) | Raised (cheer) | Orange edges before → after |
| --- | ---: | ---: | ---: | ---: | ---: |
| pier_nine | 19 | 69 | 23 | 34 | 775 → 0 |
| lantern_row | 19 | 82 | 23 | 41 | 1335 → 0 |
| foundry | 15 | 67 | 23 | 34 | 887 → 0 |
| the_crown | 14 | 67 | 23 | 34 | 1946 → 0 |
| dry_dock | 18 | 64 | 23 | 32 | 1454 → 0 |
| ship_hold | 20 | 73 | 24 | 37 | 1400 → 0 |
| brokers_club | 19 | 78 | 24 | 39 | 1635 → 0 |

**Orange-edge measurement:** count of `sodium_2` pixels adjacent to transparency in the four cardinal directions, summed across all eight frames, above the venue’s protected top-rail row. It is zero in all seven delivered sheets. Existing bright pixels at/below the rail belong to the immutable source region and are not changed to force a whole-sheet zero.

Barrier comparison is byte-exact at every protected pixel outside the authored hand masks. Changes stop no later than one row below the top rail. The rail start rows are 49, 49, 48, 48, 50, 49 and 54 in the table’s order.

Every 128-pixel horizontal strip has opaque head/shoulder pixels above row 37 in all eight frames. The earliest occupied row is 3 in every sheet. No head is clipped at x=0 or x=767. All eight frames are distinct. Stable identities and material choices are shared across frames; idle head displacement between adjacent frames, including the wrap, is at most 1 pixel and cheer displacement is at most 2 pixels.

Face construction was inspected in enlarged rest/cheer views: the face planes are ink/night_0, with one or two wood_1/wood_2 chin or cheek pixels. Front shirt, collar, scarf and hand pixels are separate from the face. No eye or tooth features were drawn. Warm highlights sit on internal/front planes, not opposing silhouette edges.

## Loop transitions

Changed-pixel counts for each transition, including 3→0 and 7→4. Alternating quieter/busier transitions come from different spectators moving on different phases; the wrap has no extra jump.

| Stage | Rest: 0→1, 1→2, 2→3, 3→0 | Cheer: 4→5, 5→6, 6→7, 7→4 |
| --- | --- | --- |
| pier_nine | 2843, 2284, 2836, 2284 | 6571, 6242, 6501, 6274 |
| lantern_row | 2436, 2909, 2437, 2902 | 6822, 6880, 6803, 6914 |
| foundry | 3390, 1910, 3398, 1915 | 6859, 6387, 6772, 6443 |
| the_crown | 1982, 3180, 1974, 3171 | 6040, 6937, 5973, 6847 |
| dry_dock | 2910, 1699, 2893, 1715 | 6801, 5711, 6842, 5740 |
| ship_hold | 2805, 2248, 2802, 2243 | 6386, 6750, 6415, 6756 |
| brokers_club | 3517, 1791, 3481, 1793 | 7454, 7122, 7561, 7006 |

## Project checks

- Compared every file from the supplied source ZIP against the integrated working copy: exactly the requested 14 production PNGs changed.
- `python3 Tools/Art/compose_previews.py`: all seven shipped previews match their layers.
- `python3 Tools/Art/selftest.py`: `art selftest: ok`.
- Release and debug: configured with `IR_WARNINGS_AS_ERRORS=ON`; both builds pass.
- `ctest --preset release` and `ctest --preset debug`: 3/3 pass in each (IronRingTests, AssetPackerBakesProject, IronRingServerCheck).
- Debug player smoke: 420 frames at 1920×1080, exit 0, screenshot inspected.
- Release player: all seven stages, 960 frames, Dee/Bram, 640×360 and 1920×1080, all 14 runs exit 0. Captures were made from the delivered loose assets.

The local build used `SDL_UNIX_CONSOLE_BUILD=ON`, SDL’s dummy video/audio drivers and the existing software batch renderer. These are build/runtime environment settings; no project source or gameplay data was modified. Results above are local checks, not a claim about remote CI.

Capture command (repeat per stage and at 640×360):

```sh
./IronRing.x86_64 -batchmode -demo -stage <id> -fighters dee,bram -frames 960 \
  -screen-fullscreen 0 -screen-width 1920 -screen-height 1080 \
  -assets-folder <source-root>/Assets -screenshot <id>_ingame_3x.png
```

At native size, collars and front openings read toward the ring; enlarged views confirm rail hands and bent forearms. Fighter skin, clothing highlights and hit effects remain brighter and more prominent than spectators. The unchanged rail structure remains in the same place.
