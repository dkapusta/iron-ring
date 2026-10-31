# ArtSource

Editable sources of externally authored art. The game never reads this folder; `Assets/` holds the packed, game-ready files. Which folders under `Assets/` are external is listed in `Tools/Art/ironart/ownership.py`; everything else in `Assets/` is drawn by `Tools/Art/generate.py`.

| Folder | Content |
| --- | --- |
| `Fighters/<Name>/frames/` | One PNG per frame: 256 × 256 RGBA, character facing right, origin (feet on the ground) at **(128, 232)**, hard alpha, 48-colour palette per fighter. `manifest.json` holds the throw partner offsets. |
| `Pack-v1/` | Provenance of Iron Ring Art Pack v1: production notes, art review, the generation prompts and the original per-frame registration. |
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
