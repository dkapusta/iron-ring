# Combined art validation

Result: PASS. 1322 automated assertions; no errors. The full project art self-test returned 0.

## Integration basis

`iron-ring-main(4).zip` + delivered V3A native masters + this correction patch + the recovered venue layers. Asset ownership was extended in memory for validation; no source-code patch is included. All existing frame manifests were retained.

| Fighter | Frames checked | Colours including portraits | Frame-review findings |
| --- | ---: | ---: | ---: |
| sable | 55 | 37 | 0 |
| idris | 55 | 37 | 0 |
| greta | 55 | 37 | 0 |
| jun | 55 | 37 | 0 |
| broker | 49 | 38 | 0 |

## Corrected active-frame geometry

World coordinates are relative to origin (128, 232), with y increasing upward. The measured terminal regions include the foot and a small adjacent shin area; they are not claimed to be anatomical sole-only measurements.

| Active frame | Terminal region world x | Terminal region world y | Pixels inside hitbox |
| --- | --- | --- | ---: |
| idris lk_1 | 71–89 | 59–76 | 218 / 218 |
| idris special_2 | 66–87 | 79–99 | 284 / 284 |
| sable hk_1 | 61–79 | 92–103 | 196 / 196 |

## Preservation and venue checks

- Exactly 54 frame PNGs differ from the V3A baseline; no other frame or manifest changed. One additional asset is the recoloured Broker HUD portrait.
- Sable’s contact-frame upper bodies and feet are pixel-identical to the baseline. All three corrected kick support-foot regions are pixel-identical.
- The Broker’s silhouettes and immutable material ramps are unchanged. Native frame dimensions, hard alpha, ground/canvas bounds and fighter palette limits pass.
- All three venues pass the specified layer dimensions, hard-alpha and master-palette checks. Backs are opaque, fronts exclude the middle 560 world pixels, and every crowd has eight distinct frames with a solid lower barrier edge.
- Stage-select previews exactly match the project compositor. Camera endpoint compositions and crowd animation were visually reviewed.
- The Hold’s two mid frames differ only in the local lamp/light region; 804 pixels change within canvas rectangle (324, 66)–(405, 245).
- The supplied full art self-test also checks the integrated assets, crowd layout and importer round-trips. See `art_selftest.txt`, `frame_review.txt` and `import.txt`.

## Environment and limits

Python 3.12.14; Pillow 12.3.0; NumPy 2.3.5. This is the available runtime, not the project’s prescribed Python 3.13 environment.

CMake, AssetPacker, ctest and live SDL execution were unavailable. No build, runtime screenshot or in-game acceptance claim is made. The newer source containing the adopted V3A state was not attached; validation uses the explicitly identified source snapshot and V3A deliverable. Re-run the project build and gameplay checks after import.

## Final delivery audit

PASS: all 15 native venue layers are byte-identical to their recovered masters; all 54 frame PNGs, one portrait and 18 stage assets are present. The portrait retains its 32 × 32 RGBA canvas and original alpha. Every PNG decodes successfully. No empty file, atlas, manifest, code or JSON file is in the archive.
