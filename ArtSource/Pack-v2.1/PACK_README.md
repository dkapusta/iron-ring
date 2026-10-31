# Iron Ring — Art Pack v2.1

Correction-only art delivery for `iron-ring-main(2).zip` and the 2026-10-31 v2.1 brief. All requested deliverables, including the optional sign repair, are included. The supplied source archive is unchanged.

## Changes

| Deliverable | Result |
| --- | --- |
| C1 — Foundry crowd | New continuous cast of 42 distinct workers; furnace-lit silhouettes; four rest and four cheer frames. |
| C2 — Pier 9 crowd | New continuous cast of 42 distinct dockers/bookmakers; right-hand sodium rim; rest/cheer animation. |
| C3 — Lantern Row crowd | New continuous cast of 42 distinct traders/strangers; warm overhead light and muted coats; rest/cheer animation. |
| C4 — Crown crowd | New continuous cast of 42 distinct rooftop spectators behind the existing bench/wall; dawn rim; rest/cheer animation. |
| C5 — Foundry sign | Restored the K's diagonals and S's painted stroke. Exactly 191 letter pixels changed, all within rows 52–72; all other wall pixels preserved. |
| P — four stage previews | Recomposed with the supplied `Tools/Art/compose_previews.py`; exact frame-0, middle-camera, no-drift composites. |

Only **nine changed runtime PNGs** are supplied under `Assets/Stages/`: four crowds, four stage previews and the Foundry wall. No fighter art, other stage layers, UI, effects, fonts, HUD, code patches or `stages.json` are included.

## Crowd contract

Every crowd is **768×672 RGBA**, eight vertical frames of **768×84**. Alpha is strictly 0/255. Existing placement, parallax and the **14-tick** frame timing remain unchanged.

Each venue has 42 individually registered spectators, with no copied or mirrored people and no repeated 128/192/256 px donor tiles. All resting head tops stay in rows **26–36**. Fourteen people move in the rest loop, by at most 1 px. Twenty-one raise one or both sleeved arms in the cheer loop; fists, open hands, hats, slips and bottles remain attached. Those bodies rise 1–3 px. The last frame of each four-frame loop leads back into its first.

The protected barriers are pixel-identical to the supplied source, in all eight frames: Foundry/Pier/Crown rows 60–83; Lantern Row's rope/post pixels and rows 65–83. The original crowd palettes are retained, with only **four additional hand colours** per venue, within the allowance of eight.

## Import

Copy the contents of this pack's `Assets/` folder over the matching paths in the current checkout's `Assets/`. Leave the existing `Assets/Data/stages.json` in place. From the checkout root:

```sh
python3 Tools/Art/compose_previews.py
python3 Tools/Art/repair_crowds.py --check
python3 Tools/Art/selftest.py
```

Expected: no preview changes, **0 clipped group frames** for all four venues, and **art selftest: ok**. Do not run `repair_crowds.py` without `--check`: it is the old donor-copy repair tool.

## Review and validation

`Previews/` contains each venue's eight-frame contact sheets at 1×, the requested 2×, and 4×, on mid-grey; a rest-then-cheer GIF; scene GIFs; and camera-end context PNGs. The runtime stage previews in `Assets/` are tool-composed, not redrawn. Review scene images are offline composites, not player captures.

Completed on a separate staged checkout:

- Supplied preview compositor, clipped-crowd check and complete art self-test pass.
- Pixel checks confirm exact protected barriers, strict alpha, sizes, palette allowance, head bands, individual sprite connectivity, and no copied/mirrored spectators.
- Scope audit confirms the nine authorised runtime files changed and **every other runtime asset, including stages.json, is byte-for-byte unchanged**.
- All eight frames were inspected at native size and 4×. Source/validation.json records the detailed results; Source/Validation contains the repository check logs.

The optional SDL demo/K.O. check was not run: this environment has no CMake/player executable. No requested art item is deferred.

## Editable sources and provenance

`Source/` contains the twelve newly generated authoring originals, pose guides, palette references, 168 individual native spectator masters, the people-only animation layers, registration/motion records, sign pixel edits and export scripts. These files stay outside the game repository. The final runtime PNGs are the production exports.

The built-in OpenAI image-generation service produced new spectator drawings from project-owned position/palette references and newly authored pose guides only. Its exact model revision was not exposed. Native pixel cleanup, individual registration, sleeve/hand/prop animation, venue rim light, barrier preservation and sign-letter edits use Python/Pillow/NumPy/SciPy. The prompt set is `Source/generation_jobs.json`; export versions are in `Source/requirements.txt`. No franchise imagery, real brands, real people or outside image sources were used.

All new and adapted artwork and authoring scripts are contributed under the project's **zlib licence**, reproduced in LICENSE. This correction delivery is identified as an altered art version, not the original software release.

Optional art rebuild, from this pack's root with an unmodified source checkout available:

```sh
python3 Source/build_crowds.py --project /path/to/current/iron-ring
python3 Source/repair_sign.py --project /path/to/current/iron-ring
```

These authoring commands write into this separate art pack, not the referenced project. After copying the resulting runtime PNGs to a staged checkout, recompose previews with the repository tool. `Source/make_previews.py --project /path/to/staged/iron-ring` copies those recomposed previews back and builds review images; `Source/validate_delivery.py` can compare that staged checkout with the unmodified one.
