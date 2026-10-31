# Iron Ring — Art Pack v2

Production PNG exports for the M2 vertical slice, based on `iron-ring-main(1).zip` and `vertical-slice-art-brief.md` dated 2026-10-30. The supplied source archive has not been modified. This is a separate art delivery contributed under the project's zlib licence.

## Start here

Extract the entire ZIP. Open **Previews/index.html** for an offline stage/camera/animation viewer. **Previews/overview.png**, the fighter contact sheets, animation GIFs and throw-pair sheet are supplementary visual checks. Runtime PNGs are in **Fighters**, **Stages** and **UI**; large generation originals and editable/native sources are in **Source**.

| Delivered | Contents |
| --- | --- |
| Dee Marlowe | 55 registered frames, corrected required poses and P3 polish, 6 champion frames, atlas + JSON, 48×48 select portrait, 640×360 ending picture |
| Bram Hollis | 56 registered frames, corrected required poses and P3 polish, 6 champion frames, atlas + JSON, 48×48 select portrait, 640×360 ending picture |
| Four venues | 25 separate parallax layers; each crowd has 8 frames; 640×360 middle-camera previews |
| Front end | 432×96 brass title wordmark and 640×360 menu backdrop |
| Integration | Per-venue layers.json, combined stages.json, fighter JSON, optional still-ending JSON, generator-ownership patch and notes |
| Sources | Large originals, identity/pose references, inherited base frames, prompts, palettes, registration records, title mask and rebuild scripts |

Fighter PNGs are 256×256 RGBA, face right and use a fixed origin **(128,232)**. Full source canvases are converted at one canvas scale; no frame is fitted to its silhouette bounding box. Integer registration, deliberate waist/breath edits and hand/arm cleanup are recorded in Source. All exported alpha is binary. The shared sprite/portrait palettes contain **47 colours for Dee and 48 for Bram**. Idle heights are Dee 130/129/128/129 and Bram 134/133/132/133; feet stay fixed. All frames receive the dark material-colour outline pass.

Atlas JSON uses the repository's `source_rect`, `pivot_pixels` and optional `partner` fields. Unchanged gameplay frames are included and palette/outline cleaned so each fighter can be replaced as a complete set. No effects, fonts, HUD, icons, locked fighters, new venues or audio are supplied.

## Stage layer contracts

Every venue has an **800 px world**, **640×360 camera**, travel **0–160**, feet at **y316** and floor chain centred at **world (400,326)**. The following lists are in back-to-front order. Animation frames are stacked **vertically**, with the full layer width per frame. Blank offsets are zero. A crowd's frames 0–3 are rest, 4–7 are cheering. `front` layers draw after fighters. Drifting layers repeat horizontally and have transparent seam margins.

### The Foundry — Stages/Foundry

| File | Native PNG | Parallax | x,y | Frames / ticks | Flags |
| --- | --- | --- | --- | --- | --- |
| foundry_wall.png | 680×360 RGB | .25 | 0,0 | 1 | opaque |
| foundry_furnace.png | 728×360 RGBA | .55 | 0,0 | 1 | |
| foundry_steam.png | 640×360 RGBA | .55 | 0,0 | 1 | drift −6 px/s, repeat |
| foundry_crowd.png | 768×672 RGBA | .8 | 0,186 | 8 / 14 | crowd; 84 px frame |
| foundry_floor.png | 800×98 RGBA | 1 | 0,262 | 1 | |
| foundry_front.png | 850×360 RGBA | 1.25 | −10,0 | 1 | front |

### Pier 9 — Stages/PierNine

| File | Native PNG | Parallax | x,y | Frames / ticks | Flags |
| --- | --- | --- | --- | --- | --- |
| pier_nine_sky.png | 672×360 RGB | .2 | 0,0 | 1 | opaque |
| pier_nine_harbour.png | 672×1080 RGBA | .2 | 0,0 | 3 / 24 | shimmer; 360 px frame |
| pier_nine_ship.png | 720×360 RGBA | .5 | 0,0 | 1 | |
| pier_nine_crowd.png | 768×672 RGBA | .8 | 0,186 | 8 / 14 | crowd |
| pier_nine_floor.png | 800×110 RGBA | 1 | 0,250 | 1 | |
| pier_nine_front.png | 850×360 RGBA | 1.25 | −10,0 | 1 | front |

### Lantern Row — Stages/LanternRow

| File | Native PNG | Parallax | x,y | Frames / ticks | Flags |
| --- | --- | --- | --- | --- | --- |
| lantern_row_facade.png | 688×360 RGB | .3 | 0,0 | 1 | opaque |
| lantern_row_lanterns.png | 712×1080 RGBA | .45 | 0,0 | 3 / 24 | slow flicker |
| lantern_row_stalls.png | 736×360 RGBA | .6 | 0,0 | 1 | |
| lantern_row_crowd.png | 768×672 RGBA | .8 | 0,186 | 8 / 14 | crowd, red rope |
| lantern_row_floor.png | 800×90 RGBA | 1 | 0,270 | 1 | |
| lantern_row_front.png | 850×360 RGBA | 1.25 | −10,0 | 1 | front |

### The Crown — Stages/TheCrown

| File | Native PNG | Parallax | x,y | Frames / ticks | Flags |
| --- | --- | --- | --- | --- | --- |
| the_crown_sky.png | 640×360 RGB | 0 | 0,0 | 1 | opaque |
| the_crown_clouds.png | 640×360 RGBA | .1 | 0,0 | 1 | drift +3 px/s, repeat |
| the_crown_city.png | 680×360 RGBA | .25 | 0,0 | 1 | |
| the_crown_sign.png | 720×720 RGBA | .5 | 0,0 | 2 / 24 | blinking bulbs |
| the_crown_crowd.png | 768×672 RGBA | .8 | 0,174 | 8 / 14 | crowd and parapet |
| the_crown_floor.png | 800×108 RGBA | 1 | 0,252 | 1 | |
| the_crown_front.png | 850×360 RGBA | 1.25 | −10,0 | 1 | front |

Each venue also contains `<venue>.png`: RGB 640×360, camera x80, crowd resting, first animation frame. `layers.json` is authoritative for offsets and timing.

## Validation and remaining integration check

The repository's `Tools/Art/selftest.py` also passes, including the 4× export/import round trip. The supplied repository's `review_frames.py` reports **zero findings for both fighters** on this delivery. The repository importer accepted 55 + 56 frames using scale 1 and no quantization; reconstructed imported pixels, pivots and partner metadata match the delivered frames. `Integration/validation.json` records native checks including palette limits, hard alpha, dimensions, timing, layer width, clear foreground centre and transparent drift seams.

Visual checks cover camera x0/x80/x160, rest/cheer crowds, the corrected key poses, champion loop, throw pairs, menu, logo and endings. These are offline composites, not game captures.

**C++/SDL build and in-game autoplay checks remain to be run in the project environment.** This environment has no `cmake` executable, so AssetPacker and the SDL player could not be built or executed here. The actual source fighter definitions take precedence over the brief's older crouch heights: Dee 92 and Bram 100; gameplay boxes and move timing are preserved.

**Ending choice:** default fighter metadata enables the champion animation. In this source snapshot an `ending` picture replaces the animated winner scene. Optional `Integration/StillEndings/` metadata enables the supplied still pictures instead. See **Integration/INTEGRATION.md** before copying metadata.

## Rebuild the art exports

Requires Python and the versions in Tools/requirements.txt. From the extracted pack root:

```sh
python3 Tools/build_fighters.py --reimport-generated
python3 Tools/polish_native.py
python3 Tools/build_stages.py
python3 Tools/build_ui.py
python3 Tools/build_previews.py
python3 Tools/validate_pack.py
```

Run the full sequence in order: `polish_native.py` applies edits to newly imported frames and is not intended to be run repeatedly on its own. Final PNGs themselves are the authoritative editable pixel assets; generation originals are not game-ready replacements. No image-generation service is needed to rebuild these exports.
