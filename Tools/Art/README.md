# Tools/Art

Python tools that produce Iron Ring's art. The game never runs them: their output is committed under `Assets/` (and the Windows icon under `Source/Launcher/Resources/`), and AssetPacker packs and validates it at build time.

```sh
pip install -r Tools/Art/requirements.txt        # pinned Pillow, numpy, scipy (Python 3.13)
```

| Script | Purpose |
| --- | --- |
| `generate.py` | Draws the interim procedural art: palette, fighters, stages, effects, fonts, icon. Deterministic. |
| `import_art.py` | Brings in art made elsewhere: `refs` exports pose references, `fighter` imports frames into an atlas, `image` fits a stage or portrait. |
| `selftest.py` | Checks that procedural assets match `generate.py`, external atlases match their `ArtSource/` frames, and export→import round-trips losslessly. Runs in CI. |
| `review_frames.py` | Measures frames against the animation rules (asset spec §2.6) and writes review sheets with boxes. Findings are notes, not errors. |
| `compose_previews.py` | Recomposes each layered stage's 640 × 360 preview from its layers; run it after editing a layer. |
| `adopt_art_pack.py` | One-off: re-registers Art Pack v1's raw sprites into `ArtSource/` at one scale per fighter ([ADR-0012](../../docs/adr/0012-adopting-external-art.md)). |

The contract every asset follows (sizes, frame names, pivots, timing, layout) is [docs/art/asset-spec.md](../../docs/art/asset-spec.md). Prompts for image generators are in [docs/art/generation-prompts.md](../../docs/art/generation-prompts.md). Background: [ADR-0011](../../docs/adr/0011-procedural-interim-art.md).

## Procedural art

```sh
python3 Tools/Art/generate.py                       # every part it still owns: palette, placeholder venues, effects, font, icon
python3 Tools/Art/generate.py fighters stages --assets build/procedural --sheet build/sheet.png
```

Dee, Bram, the stages (The Foundry included) and the title logo and menu backdrop are external now (`ironart/ownership.py`); their procedural versions remain as references and can only be written outside `Assets/` (or with `--overwrite-external`). New fighters start procedural (Sable): `generate.py fighters` draws every fighter that isn't external into `Assets/` and leaves the external ones alone. After changing anything under `ironart/`, regenerate and commit the changed assets together with the code; `selftest.py` fails otherwise.

| Module | Content |
| --- | --- |
| `palette.py` | Named colours and 5-colour material ramps (line + 4 tones); writes `Assets/Data/palette.json/.gpl`. |
| `figure.py` | Skeleton + primitives renderer: capsules and ellipses with normals, 2-bone IK, toon shading with Bayer dithering, coloured outlines and inner contour lines. |
| `characters.py` | Builds (proportions) and costumes for Dee and Bram. |
| `poses.py` | Every frame as a pose; strike points the hitboxes rely on. |
| `fighters.py` | Renders poses, trims, packs atlases, draws portraits; warns about frames sinking below the floor. |
| `stages.py`, `paint.py` | The three 640 × 360 venues and the painting helpers (glows, dithered gradients, chain ring). |
| `effects.py`, `font.py`, `icon.py` | Hit effects, the two bitmap fonts, the app icon. |
| `atlas.py` | Shared trimming, shelf packing and atlas JSON writing (also used by the importer). |
| `importer.py` | Background keying, consistent downscaling, palette snapping, fringe cleanup, reference export. |
| `preview.py` | Composes a layered stage's 640 × 360 preview from its layers, as the game draws them; the self-test compares it with the committed preview. |
| `venues.py` | The M3 venues' placeholder layers (Dry Dock 4, The Hold, the Broker's Club): back, structure, crowd, floor, foreground; previews composed by `preview.py`. |
| `ownership.py` | Which `Assets/` folders and files are externally authored, and where their sources live. |

## Replacing art with outside art

```sh
python3 Tools/Art/import_art.py refs dee build/refs/dee                     # 1. pose references
# 2. generate or draw frames from the references (same canvas, same names)
python3 Tools/Art/import_art.py fighter dee path/to/frames --origin 512 928 --scale 0.25 \
        --manifest build/refs/dee/manifest.json                             # 3. import
python3 Tools/Art/import_art.py image pier.png Assets/Stages/PierNine/pier_nine.png --size 640 360
```

Once a fighter's art comes from outside, add its folder to `ironart/ownership.py`, keep its frames in `ArtSource/Fighters/<Name>/frames/` (256 × 256, origin 128,232) and pack from there ([ArtSource/README.md](../../ArtSource/README.md)).
