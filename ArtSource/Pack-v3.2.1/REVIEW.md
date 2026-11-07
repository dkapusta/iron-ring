# Art Pack v3.2.1 (crowd band): review

**Reviewed:** 2026-11-07, against the [crowd band brief](../../docs/art/art-pack-v3.2.1-crowd-band-brief.md), with the project tools and in the game.

**Verdict:** adopted as delivered. The band of old spectators above each barrier, left by the v3.2 brief's wrong rows, is gone in all seven venues. With this, the crowd pass is complete.

## Item by item

| Venue | Result |
| --- | --- |
| Pier 9, The Foundry, The Crown | Coats and workwear run down to the rail or parapet at row 60, darkening toward it. Hands rest on the rail's top edge. The rail, posts and parapet are unchanged apart from those hands, rows 60–61. |
| Lantern Row | Aprons, shawls and coats continue to the rope, and the grips follow its sag (rows 61–63). The brass posts, caps and highlights are untouched. |
| Dry Dock 4 | Rain gear continues behind the slack chain, which stays drawn in front. The posts, links and the wall from row 68 are unchanged. |
| The Hold | Coats and oilskins show through the chain-link mesh. The mesh, posts and everything from row 70 are unchanged. |
| The Broker's Club | Suits and coats continue behind the red rope. The posts, rope and base from row 68 are unchanged. |

**Across all seven:**
- No `sodium_2` silhouette-edge pixels, and no dotted vertical `sodium` runs, above the solid barrier row. Before, these were the old rims.
- The figures above the band are as in v3.2, except where the old waist-height hands were removed and the forearms extended: 736–1,120 px over the 8 frames.
- 12–19 colours per sheet, all on the master palette.
- Binary alpha, 768 × 672, 8 frames.
- Heads in every 128 px strip of every frame.
- The rest and cheer loops close cleanly.
- The bodies in the band follow the figures' cheer bob, and the grips stay put.

## Checks

- `Checks/project_checks.txt`: diffs against v3.2 above, in and below the band; rims, dotted runs, palette and heads.
- `Checks/art_selftest.txt`: `art selftest: ok`. `compose_previews.py` reproduces the delivered previews unchanged.
- `Checks/*_band_detail_before_after_4x.png`: the pack's own before/after for The Crown (the worst case before) and The Hold (a see-through barrier).
- **In the game:** all seven venues at 1× (Dee vs Bram, 960 ticks). The crowds run down unbroken to their barriers, with no dots. The player smoke run passes, and so does `ctest`.
- **Provenance:** the method is unchanged from v3.2, with no new generated references. zlib licence.
