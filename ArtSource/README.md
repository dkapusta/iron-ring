# ArtSource

Editable sources of externally authored art. The game never reads this folder; `Assets/` holds the packed, game-ready files. Which folders under `Assets/` are external is listed in `Tools/Art/ironart/ownership.py`; everything else in `Assets/` is drawn by `Tools/Art/generate.py`.

| Folder | Content |
| --- | --- |
| `Fighters/<Name>/frames/` | One PNG per frame: 256 × 256 RGBA, character facing right, origin (feet on the ground) at **(128, 232)**, hard alpha, 48-colour palette per fighter. `manifest.json` holds the throw partner offsets. |
| `Pack-v1/` | Provenance of Iron Ring Art Pack v1: production notes, art review, the generation prompts and the original per-frame registration. |
| `Pack-v2.1/` | Provenance of Iron Ring Art Pack v2.1 (redrawn crowds, Foundry sign): the pack's README, the review, its registration, motion and validation records (`Source/`). |
| `Pack-v3.1/` | Provenance of Iron Ring Art Pack v3.1 (final polish): the pack's README, provenance, licence, checks and the review. |
| `Pack-v3B/` | Provenance of Iron Ring Art Pack v3B (Dry Dock 4, The Hold, The Broker's Club) and the v3A.1 corrections: the pack's README, provenance, review, validation, authoring notes and prompts, and the separable people, barrier and spectator masters (`Source/`). |
| `Pack-v3A/` | Provenance of Iron Ring Art Pack v3A (Sable, Idris, Greta, Jun, the Broker): the pack's README and provenance, the review, validation reports, palettes and source records (`Source/`). |
| `Pack-v2/` | Provenance of Iron Ring Art Pack v2: the pack's README, art review, integration notes and validation reports; its palettes, registration and generation records (`Source/`); the still ending pictures, not used by the game (`Endings/`). |

## Editing a fighter

1. Edit frames in any pixel editor (Aseprite, LibreSprite, GIMP). Keep the canvas, the origin, hard alpha and the fighter's existing colours.
2. Repack:

   ```sh
   python3 Tools/Art/import_art.py fighter dee ArtSource/Fighters/Dee/frames --origin 128 232 --scale 1 \
           --no-quantize --manifest ArtSource/Fighters/Dee/frames/manifest.json
   ```

3. Build, then check in `IronRing -training -stage <id>` (F3 shows boxes). `python3 Tools/Art/selftest.py` fails if `Assets/` and these frames disagree.

Stage layers, previews, portraits (32 × 32 HUD, 48 × 48 select), the title logo and the menu backdrop are final images; edit them directly in `Assets/`. Stage layer sizes follow the width rule in [asset spec §3.1](../docs/art/asset-spec.md#31-layered-stages); the build checks it. A stage's 640 × 360 preview is composed from its layers (`Tools/Art/ironart/preview.py`); recompose it after editing a layer (`python3 Tools/Art/compose_previews.py`), or the art self-test fails.

## Art Pack v1

Delivered 2026-10-30, generated with an image model from [docs/art/generation-prompts.md](../docs/art/generation-prompts.md) and the procedural pose references, under the project's zlib licence ([Pack-v1/PROVENANCE.md](Pack-v1/PROVENANCE.md)). Adopted with one change to its import, recorded in [ADR-0012](../docs/adr/0012-adopting-external-art.md): the pack's raw sprites were re-registered at one scale per fighter (`Tools/Art/adopt_art_pack.py`) because the pack had fitted each frame to its procedural bounding box, which changed the characters' size from frame to frame. Large originals (generated sheets, raw sprites, about 30 MB) stay outside the repository ([ADR-0008](../docs/adr/0008-repository-and-binary-assets.md)).

## Art Pack v2

Delivered 2026-10-31 against the [vertical-slice art brief](../docs/art/vertical-slice-art-brief.md), under the project's zlib licence. It replaces both fighters' frame sets (the brief's corrections, a dark outline pass, six champion frames each), adds select portraits, makes all four venues layered and final (The Foundry replaces its procedural placeholder), and supplies the title logo and menu backdrop. Adopted as delivered ([ADR-0020](../docs/adr/0020-adopting-art-pack-v2.md)), with one later correction: in every crowd layer, three of the six spectator groups were cut off at the head; `Tools/Art/repair_crowds.py` rebuilt them from the intact groups of the same frame and recomposed the previews ([Pack-v2/CORRECTIONS.md](Pack-v2/CORRECTIONS.md)); the arcade ending animates the champion, so the pack's still ending pictures are kept in `Pack-v2/Endings/` only. Generation originals (about 44 MB) and the pack's previews and rebuild scripts stay outside the repository.

## Art Pack v2.1

Delivered 2026-10-31 against the [v2.1 correction brief](../docs/art/art-pack-v2.1-brief.md): the four crowd layers redrawn (42 distinct spectators per venue, heads in every frame, cheering arms with hands, the barrier unchanged), a legible VOLL WORKS on the Foundry wall, and the stage previews recomposed. Adopted as delivered; review and notes for a later round in [Pack-v2.1/REVIEW.md](Pack-v2.1/REVIEW.md). It retires the v2 crowd repair.

## Art Pack v3A

Delivered 2026-10-31 against the [Art Pack v3 brief](../docs/art/art-pack-v3-brief.md): the full frame sets of Sable, Idris, Greta, Jun (55 each, with champion poses) and the Broker (49), HUD portraits for all five and select portraits for the four playable fighters, under the project's zlib licence. Adopted as delivered ([ADR-0024](../docs/adr/0024-adopting-art-pack-v3a.md)); four corrections go back as the [v3A.1 brief](../docs/art/art-pack-v3a.1-brief.md) ([Pack-v3A/REVIEW.md](Pack-v3A/REVIEW.md)). The pack's 34 MB of generated studies and its previews stay outside the repository. The venues follow as v3B.

## Art Pack v3B and v3A.1

Delivered 2026-11-01. It holds final layered art for Dry Dock 4, The Hold and The Broker's Club (the [v3 brief](../docs/art/art-pack-v3-brief.md)'s S1–S3), and the [v3A.1 corrections](../docs/art/art-pack-v3a.1-brief.md):
- the Broker's skin recoloured in all 49 frames and his portrait;
- Sable's walk contact frames redrawn and her Wire Kick lowered;
- Idris's Mast Kick and Bowline Heel brought into their hitboxes.

Adopted as delivered ([ADR-0025](../docs/adr/0025-adopting-art-pack-v3b.md), [Pack-v3B/REVIEW.md](Pack-v3B/REVIEW.md)). With it, no procedural placeholder art is left in `Assets/`. The pack's 20 MB of generated studies and its previews stay outside the repository.

## Art Pack v3.1

Delivered 2026-11-01 against the [final polish brief](../docs/art/final-polish-brief.md):
- Greta's and Jun's skin (and Jun's hair) recoloured in all their frames and portraits;
- the chain ring redrawn on the three M3 floors;
- Dry Dock 4's beam and sky softened;
- a caged lamp and light pool for The Hold;
- three specks removed.

Adopted, with two project-side edits: the shared portrait backdrop (P1) applied to all thirteen portraits with `Tools/Art/portrait_backdrop.py`, and a radial falloff on The Hold's light pool ([Pack-v3.1/REVIEW.md](Pack-v3.1/REVIEW.md)).

## Art Pack v3.2

Delivered 2026-11-07 against the [crowd brief](../docs/art/art-pack-v3.2-crowds-brief.md): the seven crowd sheets redrawn so the spectators face the ring, with shadowed faces, lit fronts, hands on the rail and cheering forearms, and their previews recomposed.

Adopted as delivered ([Pack-v3.2/REVIEW.md](Pack-v3.2/REVIEW.md)). The brief gave the wrong protected rows, so a band of the old spectators survives above the barrier. The [v3.2.1 brief](../docs/art/art-pack-v3.2.1-crowd-band-brief.md) asks for it to be repainted. The pack's game captures and GIFs (3.6 MB) stay outside the repository; its Pier 9 before/after is in `Pack-v3.2/Checks/`.

## Art Pack v3.2.1

Delivered 2026-11-07 against the [crowd band brief](../docs/art/art-pack-v3.2.1-crowd-band-brief.md). The band of old spectators above each barrier is repainted as a continuation of the v3.2 figures, behind ropes, chains and mesh where the barrier is see-through, and the rail hands are moved onto the real barriers. Adopted as delivered ([Pack-v3.2.1/REVIEW.md](Pack-v3.2.1/REVIEW.md)). The pack's game captures and GIFs stay outside the repository.

