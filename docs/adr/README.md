# Architecture decision records

One file per decision, numbered, never renumbered. Supersede rather than rewrite: mark the old record "Superseded by ADR-XXXX" and link forward. Template: [0000-template.md](0000-template.md).

| # | Decision | Status |
| --- | --- | --- |
| [0001](0001-engine-stack.md) | Own small engine on C++20 + SDL3 + SDL3_mixer | Accepted |
| [0002](0002-dependencies-via-fetchcontent.md) | Dependencies via CMake FetchContent, pinned tags, static linking | Accepted |
| [0003](0003-unity-style-player-layout.md) | Unity-style player layout (UnityPlayer module, `<name>_Data`) | Accepted |
| [0004](0004-asset-archives.md) | Asset archives: ZIP containers with Unity-like names, mounted into a VFS | Accepted |
| [0005](0005-display-hdr-and-presentation.md) | Native-resolution fullscreen, 640×360 logical canvas, scRGB HDR | Accepted |
| [0006](0006-deterministic-simulation.md) | Deterministic integer simulation ported from the concept kit | Accepted |
| [0007](0007-concept-kit-as-placeholder-content.md) | Concept kit as visual reference and placeholder content; Iron Ring setting | Accepted; placeholder art superseded by [0011](0011-procedural-interim-art.md) |
| [0008](0008-repository-and-binary-assets.md) | Binary assets in plain Git until a size threshold, then LFS | Accepted |
| [0009](0009-mingw-static-runtime.md) | MinGW runtime: static when the toolchain allows, runtime DLLs copied otherwise | Accepted |
| [0010](0010-fighter-content-format.md) | Fighters are data: one definition file per fighter, with an animation timing contract | Accepted |
| [0011](0011-procedural-interim-art.md) | Procedural interim art, replaceable through a written spec and an importer | Accepted; fighters and stages replaced per [0012](0012-adopting-external-art.md) |
| [0012](0012-adopting-external-art.md) | Adopt Art Pack v1, re-registered at one scale per fighter, with sources in ArtSource/ | Accepted |
| [0013](0013-nes-style-generated-audio.md) | Generate all audio from an NES sound-chip model; music as looping Vorbis | Accepted |
| [0014](0014-venue-themes-and-announcer.md) | A theme per venue, and an announcer through the DMC channel | Accepted |
| [0015](0015-front-end-scene-stack.md) | Build the front end on a scene stack, with saved options in prefs.cfg | Accepted |
| [0016](0016-remapping-and-renderer-recreation.md) | Remap controls per player and per pad; recreate the renderer for HDR | Accepted |
| [0017](0017-layered-stages-and-camera.md) | Layered stages wider than the screen, a follow camera, no zoom | Accepted |
| [0018](0018-hit-feel-and-heat.md) | Counter hits, combos and cancels; the Breakaway as a guard cancel; Heat for the match | Accepted |
| [0019](0019-cpu-levels-and-arcade.md) | A stateful CPU with five levels; an arcade ladder as data | Accepted |
| [0020](0020-adopting-art-pack-v2.md) | Adopt Art Pack v2 as delivered, with animated champion endings | Accepted |
| [0021](0021-adding-fighters.md) | Add fighters as data with procedural placeholder art; resolve the arcade ladder per run | Accepted |
| [0022](0022-boss-and-story.md) | A boss bout after the title, and the arcade story as fighter data | Accepted |
| [0023](0023-stage-streaming.md) | Stream each venue's layers and theme from archives of its own; keep two stages loaded | Accepted |
| [0024](0024-adopting-art-pack-v3a.md) | Adopt Art Pack v3A (the M3 fighters) as delivered, with a correction round | Accepted |
| [0025](0025-adopting-art-pack-v3b.md) | Adopt Art Pack v3B (the M3 venues) and the v3A.1 corrections | Accepted |
| [0026](0026-online-play.md) | Online play through a dedicated server, with rollback and room codes | Accepted |
