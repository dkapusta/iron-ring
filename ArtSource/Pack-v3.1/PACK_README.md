# Iron Ring — Art Pack v3.1 Polish

2026-11-01 · Altered art release · zlib

**Status: P2–P7 implemented and validated. P1 remains pending the exact project backdrop templates. This archive is not the complete final-polish sign-off.**

This is an overlay for the accepted V3A + V3B / V3A.1 art. It contains **126 changed production PNGs**, in repository paths, plus review previews and notes. It does not contain unchanged production files, atlases, manifests, game data, code or JSON.

## Item status

| Item | Result |
| --- | --- |
| P1 — common portrait backdrop | **Pending input.** Neither `docs/art/templates/portrait_backdrop_32.png` nor `portrait_backdrop_48.png` exists in the available source archive. No approximation was substituted. All current backdrops remain as received. Seven portraits whose only requested edit was P1 are omitted because they are unchanged. |
| P2 — Greta | All 55 frames and both portraits received a region-masked apron-to-skin recolour using the five specified skin tones. Hair, shirt and trouser colour ramps remain unchanged. The apron and glove regions were protected through per-pose masks; the champion chain is unchanged. |
| P3 — Jun | All 55 frames and both portraits received skin and black-hair corrections using the specified ramps. Shoe red remains in the high-top regions. Jade jacket, wrist tape and trouser colour ramps are unchanged. |
| P4 — three M3 chains | Rebuilt the native ring motifs using alternating open 7 × 4 links and 5 × 2 bars on the shared metal ramp, around the original centre (400, 64), radius (282, 24), with 1 px contact shadows and venue-dependent highlights. The edits are confined to the ring band. All three previews were recomposed. |
| P5 — Dry Dock | Retained the lamp head/tower, narrowed and dimmed the beam, and moved the dark sky transition below the HUD band with an irregular edge. The resulting beam is substantially fainter than the old cone; the reduction exceeds the brief's approximate half-area target. |
| P6 — The Hold | Two 9 × 12 caged work-lamp frames, hot centres and moving warm dither pools. The pool reaches the netting and crowd area, ending before the separate floor. The moon shaft is unchanged. Only the lamp, cable and illumination differ between the new frames. |
| P7 — cleanup | Dee's actual shoulder speck at (38, 38) now matches adjacent skin. Bram's (20, 6) scalp speck uses his skin highlight. Five dark ankle-edge pixels were removed from Sable `walk_3`: (88, 215), (89, 215), (169, 215), (170, 215), (171, 215). The three right-hand pixels formed one detached stub. Feet at y ≥ 216 are byte-for-byte unchanged. |

## What is included

- 55 Greta frame PNGs and 55 Jun frame PNGs under `ArtSource/Fighters/`.
- Sable's single corrected `walk_3.png`.
- Greta and Jun's HUD/select portraits, plus Dee and Bram's select portraits: 6 changed portrait PNGs.
- 6 changed venue layers and 3 matching composed previews under `Assets/Stages/`.
- `Previews/`: before/after comparisons, all 55 poses for each recoloured fighter, roster comparisons at 1×/4×, the six select portraits, all seven floors, camera endpoints, and a lamp-swing GIF.
- `Checks/`: importer, frame-review and self-test results, and per-frame mask counts.

The portrait-row previews deliberately show **P1 pending**. They do not demonstrate a common backdrop yet.

## Verification

The integrated test copy was assembled from the available `iron-ring-main(4).zip` source, the accepted V3A frames and V3B/V3A.1 corrections, then this patch. The newer source package described by the final brief was not attached.

- **1,384 assertions passed; no errors.** Native dimensions, RGBA depth, binary alpha, palette membership, changed-file scope, ring bounds, illumination bounds and preview composition were checked.
- Greta, Jun and Sable imported at origin **(128, 232)**, scale **1**, without quantisation. All three frame reviews report **0 findings**.
- Full project art self-test: **`art selftest: ok`**.
- Greta, Jun and Sable each use **37 colours including portraits**, below the 48-colour limit.
- Greta and Jun preserve every alpha pixel. Source manifests / gameplay geometry are unchanged. Sable's five explicitly requested removals are the sole fighter-alpha exception.
- `Checks/recolour_counts.csv` records all 110 frames. Forbidden apron/shoe colours are zero **within the reviewed skin/hair masks**; protected-pixel changes and alpha changes are zero. These counts validate the selected masks, not an independent automatic semantic segmentation of the figures. The before/after sets were also inspected visually.
- **Not run:** the exact template-background diff (templates absent), CMake, AssetPacker, ctest or a live SDL/game check (not available in this environment). This is an offline art validation, not a runtime sign-off.

## Apply

Copy the archive's `Assets/` and `ArtSource/` contents into the current project, preserving the paths. Repack Greta, Jun and Sable through the project's normal importer and existing manifests, then recompose previews and run the project checks. Generated atlases are intentionally excluded from the archive.

To finish P1, supply the exact two template PNGs above, or the current source ZIP containing them. Apply their pixels outside each portrait's figure outline, preserve the select colour bands, then rerun the backdrop diff and portrait review. No further venue work is needed for P1.
