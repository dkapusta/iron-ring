# ArtSource

Editable sources of externally authored art. The game never reads this folder; `Assets/` holds the packed, game-ready files. Which folders under `Assets/` are external is listed in `Tools/Art/ironart/ownership.py`; everything else in `Assets/` is drawn by `Tools/Art/generate.py`.

| Folder | Content |
| --- | --- |
| `Fighters/<Name>/frames/` | One PNG per frame: 256 × 256 RGBA, character facing right, origin (feet on the ground) at **(128, 232)**, hard alpha, 48-colour palette per fighter. `manifest.json` holds the throw partner offsets. |
| `Pack-v1/` | Provenance of Iron Ring Art Pack v1: production notes, art review, the generation prompts and the original per-frame registration. |

## Editing a fighter

1. Edit frames in any pixel editor (Aseprite, LibreSprite, GIMP). Keep the canvas, the origin, hard alpha and the fighter's existing colours.
2. Repack:

   ```sh
   python3 Tools/Art/import_art.py fighter dee ArtSource/Fighters/Dee/frames --origin 128 232 --scale 1 \
           --no-quantize --manifest ArtSource/Fighters/Dee/frames/manifest.json
   ```

3. Build, then check in `IronRing -training -stage <id>` (F3 shows boxes). `python3 Tools/Art/selftest.py` fails if `Assets/` and these frames disagree.

Stages and portraits from the pack are final 640 × 360 and 32 × 32 images; edit them directly in `Assets/`.

## Art Pack v1

Delivered 2026-10-30, generated with an image model from [docs/art/generation-prompts.md](../docs/art/generation-prompts.md) and the procedural pose references, under the project's zlib licence ([Pack-v1/PROVENANCE.md](Pack-v1/PROVENANCE.md)). Adopted with one change to its import, recorded in [ADR-0012](../docs/adr/0012-adopting-external-art.md): the pack's raw sprites were re-registered at one scale per fighter (`Tools/Art/adopt_art_pack.py`) because the pack had fitted each frame to its procedural bounding box, which changed the characters' size from frame to frame. Large originals (generated sheets, raw sprites, about 30 MB) stay outside the repository ([ADR-0008](../docs/adr/0008-repository-and-binary-assets.md)).
