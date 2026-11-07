# Provenance — v3.2 Crowd Correction

## Inputs

- `IronRing-source-0.4.2-f2b83eb.zip`: supplied game source, current crowd sheets, barrier masters, palette, setting and composition tools.
- `art-pack-v3.2-crowds-brief.md` and `IronRing_CrowdPass_Brief.zip`: supplied correction requirements and the `crowd_now.png` / `crowd_reference_spectator.png` references.

## Process and tools

An OpenAI image-generation edit produced a Pier 9 reference study from an
enlarged crop of the existing crowd and the supplied schematic. It was used to
resolve the frontal lighting and collar/hand treatment. The study is not a
production sprite sheet and is not included among the game assets.

The delivered art was then constructed at native pixel resolution using
authored sprite shapes and Python/Pillow/NumPy composition, with deterministic
per-spectator costumes and four-frame motion cycles. No smoothing, tracing of
unrelated artwork or outside stock assets were used. The hand-cleanup and native
pixel method is permitted by brief §6. The generator output was a design
reference, not an unreviewed raster replacement.

Original protected barrier regions were copied exactly, with explicit hand
overlays. The supplied `Tools/Art/compose_previews.py` recomposed the stage
previews. The source project's CMake/Ninja builds, AssetPacker, CTest and art
self-test provided validation; the supplied player rendered the game captures.

All final colours come from the supplied palette. The original copyright and
zlib licence notice is retained in `LICENSE.txt`.

## Exact reference-study prompt

Use case: precise-object-edit. Asset type: native pixel-art spectator strip for Iron Ring, an original gritty 1990s arcade fighter. Image 1 is the existing Pier 9 crowd strip at 4x nearest-neighbour, edit target; image 2 is the project's schematic explaining the wanted front-facing cues, not final style. Redraw only the spectators ABOVE the steel barrier so they unmistakably FACE THE VIEWER/RING, maintaining approximately the same small native pixel scale, densely crowded busts, about 16 people in this crop. The barrier must remain untouched. Bodies dark navy pea coats, weathered brown work coats, muted teal, dockers' caps and a few hard hats. Every person needs a clear front collar or lapel V, plus coat opening/buttons or scarf. About a third lean forward with two hands resting ON the top rail; some three-quarter heads turn toward the middle. Add overlapping shoulders and mixed heights, including a second row peeking through. Faces stay featureless in deep navy shadow under brims, ONLY one or two muted brown skin pixels for a chin or nose: no eyes, teeth, white face patches. Warm light falls from IN FRONT on hat top planes, shoulder tops, collars and hands, never an all-round orange outline. Use almost-black outlines with sparse muted warm interior highlights. Hottest orange just a few inset pixels. Retain hard square pixel edges, no smoothing or antialiasing. Transparent background. No scenery, text or logos. This is the Pier 9 reference pass: solve the frontal read and dark cinematic palette.

