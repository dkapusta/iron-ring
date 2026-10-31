# Iron Ring — Art Pack v3A: Characters

The complete character portion of Art Pack v3, recovered from the earlier completed V3 archive and revalidated against `iron-ring-main(4).zip` and `art-pack-v3-brief(1).md`. The recovered production artwork was usable; this delivery preserves it instead of generating another inconsistent set.

This drop follows the requested **characters / venues** split: it includes all five new fighters, including the Broker. Venues are deferred to the second drop. This differs deliberately from the brief's optional four-fighter Drop A / Broker-and-venues Drop B division.

| Fighter | Native frames | HUD portrait | Select portrait | Standing height |
| --- | ---: | --- | --- | ---: |
| Sable Reyes | 55 | 32 × 32 | 48 × 48 | 132 px |
| Idris Okafor | 55 | 32 × 32 | 48 × 48 | 139 px |
| Greta Voll | 55 | 32 × 32 | 48 × 48 | 121 px |
| Jun Arata | 55 | 32 × 32 | 48 × 48 | 118 px |
| The Broker | 49 | 32 × 32 | — | 140 px |
| **Total** | **269** | **5** | **4** | |

## What is included

- `Fighters/<Name>/frames/`: individual 256 × 256 RGBA production masters and the throw-partner manifest. Fixed origin **(128, 232)**; facing right; binary alpha; one shared palette per fighter, including portraits, within 48 colours.
- `Fighters/<Name>/`: native portraits and convenient atlas PNG/JSON pairs rebuilt with the latest supplied importer. The atlas JSON is rendering metadata, not fighter gameplay data.
- `Previews/Fighters/`: complete 2× contact sheets, animated move previews, paired throw reviews, and a scale/portrait comparison against the unchanged Dee and Bram references.
- `Previews/Boxes/`: fresh hitbox/hurtbox review sheets produced by the current project tools.
- `Previews/Validation/`: current checks, import log, self-test log, and expected ownership-adoption differences.
- `Source/`: five identity sheets, portrait originals, selected large idle/special/champion studies, generation prompts and native registration/repair records. The final native PNGs are the authoritative editable art; large studies are reference material.
- `LICENSE`, `PROVENANCE.md` and `CONTENTS_SHA256.txt`.

Broker intentionally has no champion frames or select portrait. The other four have all six champion frames. No venue art, Dee/Bram replacement art, frontend, FX, fonts, HUD, audio, still ending pictures, gameplay JSON or code patches are included. No character deliverable is missing. No hitbox-fit exception was identified by the frame reviewer.

## Integrating the art

Work from the project root. For each fighter:

1. Copy `Fighters/<Name>/frames/` into `ArtSource/Fighters/<Name>/frames/`.
2. Copy `<id>_portrait.png` and, where supplied, `<id>_select.png` into `Assets/Fighters/<Name>/`.
3. Rebuild the atlas with the commands below. Alternatively, copy the supplied `<id>_atlas.png` and `<id>_atlas.json` pair into that same Assets folder; they are an exact round trip of these masters.

```sh
python3 Tools/Art/import_art.py fighter sable ArtSource/Fighters/Sable/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Sable/frames/manifest.json
python3 Tools/Art/import_art.py fighter idris ArtSource/Fighters/Idris/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Idris/frames/manifest.json
python3 Tools/Art/import_art.py fighter greta ArtSource/Fighters/Greta/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Greta/frames/manifest.json
python3 Tools/Art/import_art.py fighter jun ArtSource/Fighters/Jun/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Jun/frames/manifest.json
python3 Tools/Art/import_art.py fighter broker ArtSource/Fighters/Broker/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Broker/frames/manifest.json
```

**Adopt these five fighter folders as external art** in the existing `EXTERNAL` set in `Tools/Art/ironart/ownership.py`, using the same convention as Dee and Bram: `("Fighters", "Sable")`, `("Fighters", "Idris")`, `("Fighters", "Greta")`, `("Fighters", "Jun")`, `("Fighters", "Broker")`. The current importer does not perform that ownership change. Without it, a later generator run can replace these atlases, and the self-test reports expected placeholder differences. No ownership patch is bundled because this delivery is art only. Venue ownership stays as supplied.

Preserve the manifests. The raised throw artwork uses these rendering attachment offsets, measured from the thrower's origin in forward/up coordinates:

| Fighter | `throw_hold_0` | `throw_hold_1` |
| --- | --- | --- |
| Sable | [30, 0] | [40, 86] |
| Idris | [30, 0] | [54, 91] |
| Greta | [30, 0] | [35, 77] |
| Jun | [30, 0] | [41, 65] |
| Broker | [30, 0] | [42, 88] |

These are the earlier artwork's adjusted partner offsets, permitted by brief §3.4; gameplay boxes and timing are unchanged. The supplied paired previews support offline review. Live throws against each opponent still require runtime acceptance.

Do not install `Previews/` or `Source/` under `Assets/`. Do not apply the historical scale calibration in the Source records again: the native masters are already registered. The reference exporter may synthesize unused Broker champion guides; keep the delivered Broker frame set at 49.

## Validation and remaining runtime acceptance

Fresh validation against the latest source includes exact frame names/counts, native PNG mode and dimensions, binary alpha, shared palette limits, standing heights, 0/1/2/1 idle breathing, fixed idle/champion-loop feet, canvas/ground bounds, current importer round trips, animation frame references, and the supplied frame reviewer. All five fighters have **zero frame-review findings**.

The full art self-test is run in isolated staging with the five new fighter folders classified as external **in memory only**. No project code or original source assets are modified. The test passes. The final validation records 2,781 checks and confirms all 552 original source archive entries are unchanged. The report records tool versions, input hashes and expected pre-adoption differences. Existing venues and gameplay files are unchanged.

**Not run here:** CMake build, AssetPacker, ctest, and live SDL training/select/arcade-ending playback. CMake/ctest and a built player are unavailable in this environment. Installed Python library versions also differ from the project's pinned requirements; current versions and test results are recorded in `Previews/Validation/report.json`. Offline previews are not game captures.

After adoption, complete the project's normal release build and acceptance checks:

```sh
python3 Tools/Art/review_frames.py sable idris greta jun broker --sheets review
python3 Tools/Art/selftest.py
cmake --preset release -DIR_WARNINGS_AS_ERRORS=ON
cmake --build --preset release
ctest --preset release
```

Use training with F3 boxes to check each active limb and throw attachment; review the select portraits and the four champion endings. The original brief gives platform-specific player paths and autoplay commands. No stage recomposition is needed for this character-only drop.
