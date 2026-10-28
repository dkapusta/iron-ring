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
| [0007](0007-concept-kit-as-placeholder-content.md) | Concept kit as visual reference and placeholder content; Iron Ring setting | Accepted |
| [0008](0008-repository-and-binary-assets.md) | Binary assets in plain Git until a size threshold, then LFS | Accepted |
| [0009](0009-mingw-static-runtime.md) | Link the MinGW runtime statically (no runtime DLLs next to binaries) | Accepted |
