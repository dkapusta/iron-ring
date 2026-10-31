# Iron Ring Art Pack v3B — Venues + v3A.1 Corrections

Second V3 drop, 31 October 2026. Install over the adopted **V3A character pack**. This combined package follows the V3 venue brief (S1–S3) and the V3A.1 correction brief (K1–K5).

## Contents

| Deliverable | Repository path | Included assets |
| --- | --- | --- |
| S1 · Dry Dock 4 | `Assets/Stages/DryDock/` | Five layers + recomposed 640 × 360 preview |
| S2 · The Hold | `Assets/Stages/ShipHold/` | Five layers + recomposed preview; two-frame swinging lamp |
| S3 · The Broker’s Club | `Assets/Stages/BrokersClub/` | Five layers + recomposed preview |
| K1 · Broker skin | `ArtSource/Fighters/Broker/frames/` and `Assets/Fighters/Broker/` | All 49 frame PNGs + 32 × 32 HUD portrait |
| K2 · Sable walk contacts | `ArtSource/Fighters/Sable/frames/` | `walk_0.png`, `walk_3.png` |
| K3 · Idris Mast Kick | `ArtSource/Fighters/Idris/frames/` | `lk_1.png` |
| K4 · Idris Bowline Heel | `ArtSource/Fighters/Idris/frames/` | `special_2.png` |
| K5 · Sable Wire Kick | `ArtSource/Fighters/Sable/frames/` | `hk_1.png` |

Total: **54 corrected character frames, one corrected portrait, 15 venue layers and three venue previews**. Review images, authoring sources and documentation are separate from the installable assets. No atlases, manifests, code or JSON are supplied. All other V3A character assets remain installed from the first drop.

## Character changes

- **K1:** Gold and burgundy on the Broker’s face, ears and neck are replaced with the existing five-tone skin ramp: `#2a1a14`, `#6e4c3e`, `#9a7060`, `#bf9480`, `#dcb8a2`. The recolour is applied across all 49 frames and the HUD portrait. The alpha silhouette, silver hair, suit, shirt and gloves are retained; the tie and gold accessories are protected. The complete fighter palette remains 38 colours.
- **K2:** Both contact poses have separately shaded legs and an open transparent gap. The contacts differ in near/far leg treatment. Head, arms, torso and feet retain their original pixels and placement. Only the leg area between canvas rows 185 and 215 changes.
- **K3:** The active push kick is lowered to belly height with the sole facing forward inside its existing hitbox. The endpoint moves about 22 pixels down, with a small reach adjustment and local shoe foreshortening. This changes the pose, not the fighter’s scale. The support foot and upper body remain fixed.
- **K4:** The extended heel kick is lowered into its existing head-height box while retaining the torso’s spin and full leg extension. The terminal adjustment is about 28 pixels down. The support foot remains fixed.
- **K5:** The recommended Wire Kick correction is included. Its endpoint is lowered **13 pixels**, slightly more than the suggested 8–10, so the whole terminal foot region fits inside the box. The high-kick silhouette is retained. This is the only numerical departure from the recommended K5 adjustment.

The existing chamber, recovery and neighbouring frames were retained after sequence review; no additional character frames needed changes. All corrected frames remain 256 × 256 RGBA, hard alpha, origin (128, 232), with unchanged standing scale. No hitboxes, timing, throw partner offsets or game data change.

## Venue changes

The earlier V3 venue work was recoverable. These are its finished native layers, revalidated against the supplied project, with freshly recomposed stage-select previews. They are not newly generated substitutes.

Each venue has a 680 × 360 back, 728 × 360 mid, 768 × 672 crowd strip (eight 84-pixel frames), 800 × 98 floor and 850 × 360 front. The Hold’s mid is 728 × 720 for its two 30-tick lamp frames. Rest crowd frames are 0–3; cheering frames are 4–7. The existing stage placements, parallax, chain-ring layout and foreground exclusion area are retained.

## Import

1. Merge the ZIP’s `ArtSource/` and `Assets/` directories into the repository root. **Keep the existing frame manifests**. `Previews/`, `Source/` and these documents are review material; they are not runtime assets.
2. Before procedural generation or self-tests, ensure these three tuples are in the existing `EXTERNAL` set in `Tools/Art/ironart/ownership.py`: `("Stages", "DryDock")`, `("Stages", "ShipHold")`, `("Stages", "BrokersClub")`. Keep the existing V3A fighter ownership entries. No ownership code patch is included in this art-only drop.
3. Repack only the three corrected fighters, using their existing manifests:

```sh
python3 Tools/Art/import_art.py fighter broker ArtSource/Fighters/Broker/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Broker/frames/manifest.json
python3 Tools/Art/import_art.py fighter sable ArtSource/Fighters/Sable/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Sable/frames/manifest.json
python3 Tools/Art/import_art.py fighter idris ArtSource/Fighters/Idris/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Idris/frames/manifest.json
python3 Tools/Art/compose_previews.py
python3 Tools/Art/review_frames.py sable idris broker --sheets review
python3 Tools/Art/selftest.py
```

4. Run the project’s normal AssetPacker/build/tests. In training, inspect Idris’s two kicks with F3 boxes, the Broker’s face and HUD, and Sable’s walk and Wire Kick. Visit each venue at both camera ends and trigger its cheering crowd.

## Validation and previews

The combined art validation passed **1,322 checks**, all five V3 fighters had **zero frame-review findings**, and the full art self-test ended with `art selftest: ok`. The three corrected terminal foot regions fit entirely inside their existing hitboxes. See `Previews/Validation/VALIDATION.md` and the accompanying logs for evidence and limitations.

Validation used `iron-ring-main(4).zip`, the delivered V3A masters and this patch in an isolated integration copy. A newer source archive reflecting the user’s adopted V3A state was not available with the correction brief. CMake, AssetPacker, ctest and live SDL game checks were unavailable here; **this package has not been runtime-tested in the game**. The brief’s report of an earlier successful V3A build is not a runtime test of these corrections.

- `Previews/Corrections/`: 4× before/after comparisons, all Broker faces, a native-size skin comparison and corrected sequence GIFs. Attack GIFs are slowed for inspection; they do not redefine game timing.
- `Previews/Boxes/`: fresh project-generated review sheets for Sable, Idris and the Broker.
- `Previews/Venues/`: 1× camera endpoint compositions, rest/cheer crowd GIFs and crowd sheets. These are offline compositions, not captured game screenshots.
- `Source/`: recovered venue studies, individual spectators, separable people/barrier masters, prompts and correction-method notes.

The artwork is contributed under the project’s zlib licence. See `LICENSE` and `PROVENANCE.md`. `CONTENTS_SHA256.txt` lists every other file in this ZIP.
