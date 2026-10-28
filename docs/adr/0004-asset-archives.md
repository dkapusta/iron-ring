# ADR-0004: Asset archives — ZIP containers with Unity-like names, mounted into a VFS

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The player should load content from a few packed files with Unity-flavoured names (ADR-0003), while development should not require repacking after every edit. The format must be simple, debuggable and fast enough for a 2D game whose largest assets are a few MB of PNG.

## Decision

**Container.** Each archive is a standard ZIP file under a custom name (`resources.assets`, `sharedassets0.assets`, `resources.resource`, `globalgamemanagers`). Implemented with miniz.

- First entry `.archive`: JSON header `{"format": 1, "archive": "<name>", "entries": N}`. The runtime rejects archives without it, so a random ZIP renamed to `.assets` fails loudly.
- Entry names are **virtual paths**: relative, `/`-separated, case-sensitive, no `..`, no drive letters (`vfs::normalizePath`).
- Already-compressed formats (PNG, JPG, OGG, FLAC, MP3) are *stored*; everything else is deflated.
- **Deterministic output**: entries sorted by path, fixed timestamp (2000-01-01), timezone pinned to UTC inside the packer. Same inputs → byte-identical archives (tested).

**Pipeline.** `ProjectSettings/AssetPipeline.json` is the single source of truth: each rule names an output archive, a root (virtual paths are relative to it), include folders and optional extension filters. CMake reads it with `string(JSON)` to know the outputs; AssetPacker reads it to fill them. A virtual path claimed by two archives is an error; a file under `Assets/` claimed by none is a warning.

**Runtime.** `vfs::VirtualFileSystem` holds an ordered list of `vfs::Mount`s (`ZipMount`, `DirectoryMount`). Lookup goes newest-first, so a later mount overrides an earlier one. The player mounts every archive in `<name>_Data` (sorted), then `-assets-folder <dir>` if given. Decoding happens in the engine: PNG bytes → `SDL_LoadPNG_IO`, audio bytes → `MIX_LoadAudio_IO`.

## Consequences

- Any ZIP tool can inspect a build; `AssetPacker --list <archive>` does too.
- Overrides give us patches and mods for free later (mount `patch0.assets` last).
- Whole-entry reads into memory: fine for current sizes. Streaming music will read stored entries through an `SDL_IOStream` over the file range (M1); the stored-not-deflated rule for audio keeps that possible.
- miniz readers are not thread-safe; `ZipMount` serialises reads with a mutex. Revisit if background loading becomes a bottleneck (one reader per thread).
- No encryption or obfuscation. Not a goal.

## Alternatives considered

- **Imitating Unity's SerializedFile format**: large effort, zero benefit, and misleading.
- **PhysicsFS**: mature VFS, but another dependency for ~300 lines we control and test.
- **Custom binary pack with an index**: faster to open in theory; ZIP is already indexed and tool-friendly.
- **One archive per asset type** (`.textures`, `.sounds`): doesn't match the Unity look and complicates per-stage streaming.
