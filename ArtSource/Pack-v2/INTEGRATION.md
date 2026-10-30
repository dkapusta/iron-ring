# Integration into the attached M2 snapshot

This document describes changes to make in your own checkout. No files have been inserted into the supplied source archive.

1. Copy `Fighters/Dee/frames` and `Fighters/Bram/frames` into the matching `ArtSource/Fighters/<Name>/frames/` directories, replacing the complete sets. Keep `manifest.json` beside the frames.
2. Copy the two portraits and ending pictures from each pack `Fighters/<Name>/` folder into `Assets/Fighters/<Name>/`. Keep the existing HUD portraits.
3. Copy each pack `Stages/<Venue>/*.png` into the matching `Assets/Stages/<Venue>/`. The combined `Integration/Assets/Data/stages.json` supplies all four entries with 800 px world width and vertical-strip animation metadata. It preserves the existing stage IDs, music IDs and descriptions.
4. Copy `UI/title_logo.png` and `UI/menu_backdrop.png` into `Assets/Resources/UI/`.
5. Use the matching `Integration/Assets/Fighters/<Name>/<id>.fighter.json`. Gameplay bodies, hurtboxes, hitboxes, move timings and audio references come from the attached snapshot. Changes are the six-frame champion intro/loop and the 48×48 select portrait reference.
6. Apply the small `ownership-v2.patch` from the checkout root (`git apply <path-to-pack>/Integration/ownership-v2.patch`). It protects Foundry and the two UI pictures from procedural regeneration/comparison. It also marks generator parts `foundry` and `ui` external. **Fonts, HUD and other Resources/UI files remain procedural.** The patch targets this exact source snapshot; inspect/rebase it if those tools have changed.
7. Repack with the repository importer, then run the repository checks below.

```sh
python3 Tools/Art/import_art.py fighter dee ArtSource/Fighters/Dee/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Dee/frames/manifest.json
python3 Tools/Art/import_art.py fighter bram ArtSource/Fighters/Bram/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Bram/frames/manifest.json
python3 Tools/Art/review_frames.py --sheets build/art-v2-review
python3 Tools/Art/selftest.py
cmake --preset release
cmake --build --preset release
```

The pack's prebuilt atlases can also be copied directly to `Assets/Fighters/<Name>/`; their JSON uses the repository format. Repacking can change atlas layout without changing decoded pixels or origins. Use source frames as the editing authority.

## Champion animation and still endings

`champion_intro`: champion_0/20 ticks → champion_1/16 → champion_2/14, once. `champion`: champion_3/18 → champion_4/18 → champion_5/18, looping. At 60 Hz, the intro is 50 ticks and the loop is 54 ticks.

`Source/Runtime/Game/ArcadeScene.cpp::drawEnding` checks `ending()` first. If an ending picture exists, it draws that picture as the whole scene and skips `drawChampion`. For that reason the default supplied fighter JSON **omits `ending`**, so the required six-frame sequence is visible. The ending PNGs are still delivered and installed. To show a still ending instead, use the corresponding JSON in `Integration/StillEndings/`, which adds `"ending": "<id>_ending.png"`. Animating the champion over a bespoke ending background would require a renderer change outside this art-only delivery.

## Registration and partner points

All source frames: 256×256 RGBA, origin (128,232), right-facing. Import with scale 1; do not use `--height` and do not resize individual bounds. The actual repository crouch boxes are height 92 for Dee and 100 for Bram, rather than the older values in the brief. These definitions are retained.

| Fighter/frame | Partner [dx,dy] | Reason |
| --- | --- | --- |
| Dee throw_hold_0 | [22,2] | New close collar/upper-arm clinch |
| Dee throw_hold_1 | [34,34] | Retained release point |
| Bram throw_hold_0 | [28,12] | Knees bent, victim raised to chest |
| Bram throw_hold_1 | [34,34] | Retained release point |
| Bram hoist_0 | [30,0] | Retained waist hold |
| Bram hoist_1 | [4,126] | Retained overhead victim placement, thicker arms |
| Bram hoist_2 | [44,6] | Retained release point |

`Previews/throw_pairs.png` composes the same mirrored victim frames and these offsets at native resolution. Rendering order and live transition timing should receive the final SDL pass.

## Checks completed / pending

Completed: repository art self-test (including 4× round trip and source/atlas match), repository Python frame review (zero findings), importer reconstruction at scale 1/no quantization (111 frames, exact pixels/pivots/partner values), native PNG/palette/alpha and stage coverage checks, offline visual review, archive integrity and readback verification. Machine-readable findings: `frame_review.json` and `validation.json`.

Not executed: CMake/AssetPacker, SDL training/demo, arcade autoplay and actual menu captures. `cmake` is absent in the delivery environment. Run the brief's F3 hitbox check, camera-end demos and both arcade ending commands after integrating.

Keep `Source/`, large originals, optional review GIFs and tools outside runtime Assets. Keep the existing fonts, effects, HUD, icon, locked fighters and audio.
