# Asset pipeline

Source content → `AssetPacker` → archives in `IronRing_Data/` → virtual file system at runtime. Decision record: [ADR-0004](adr/0004-asset-archives.md).

## Where things go

| You add | Put it in | Packed into | Virtual path |
| --- | --- | --- | --- |
| UI art, fonts, effects, HUD data | `Assets/Resources/...` | `resources.assets` | `Resources/...` |
| Sound effects, music (WAV/OGG/FLAC) | `Assets/Resources/Audio/...` | `resources.resource` | `Resources/Audio/...` |
| Fighter atlases + frame data | `Assets/Fighters/<Name>/` | `sharedassets0.assets` | `Fighters/<Name>/...` |
| Stage art | `Assets/Stages/<Name>/` | `sharedassets0.assets` | `Stages/<Name>/...` |
| Content tables (roster, moves, stages) | `Assets/Data/` | `sharedassets0.assets` | `Data/...` |
| Game-wide settings, input bindings | `ProjectSettings/` | `globalgamemanagers` | `ProjectSettings/...` |

A new top-level folder under `Assets/` needs a rule in `ProjectSettings/AssetPipeline.json`; otherwise AssetPacker prints `not packed: ...` for each file in it.

## Rules file

```json
{
  "output": "resources.resource",   // file name in IronRing_Data
  "root": "Assets",                 // virtual paths are relative to this
  "include": ["Resources"],         // folders under root, scanned recursively
  "extensions": [".wav", ".ogg"],   // optional allow-list
  "excludeExtensions": []           // optional deny-list
}
```

CMake reads only the `output` names (to declare build outputs); AssetPacker reads everything. A virtual path claimed by two archives fails the build. Dotfiles, `Thumbs.db`, `desktop.ini` and `*.lnk` are ignored.

## Building and inspecting

The `PlayerData` target runs automatically with the player build whenever anything under `Assets/` or `ProjectSettings/` changes. By hand:

```sh
Build/debug/Tools/Debug/AssetPacker --project . --pipeline ProjectSettings/AssetPipeline.json --output /tmp/IronRing_Data
Build/debug/Tools/Debug/AssetPacker --list Build/debug/Player/Debug/IronRing_Data/sharedassets0.assets
```

Archives are ordinary ZIP files (`unzip -l` works). Output is byte-identical for identical inputs, on any machine and time zone.

## Iterating without repacking

```sh
IronRing.x86_64 -screen-fullscreen 0 -assets-folder ../../../../Assets
```

The loose folder is mounted last, so its files override the packed ones. `ProjectSettings/` is not covered by this override yet — repack after changing settings.

## Formats

- **Images:** PNG (RGBA), decoded with SDL's built-in loader. Stored, not deflated.
- **Audio:** WAV for short effects (decoded to PCM at load), OGG/FLAC for music. Stored.
- **Data:** JSON, deflated. Runtime schema validation arrives with M1; until then malformed data fails at startup with the asset path in the error.
- **Sprite atlases:** JSON next to the PNG with `image`, and either `frames[]` (`source_rect` `[x,y,w,h]`, `pivot_pixels` `[x, footY]`) or a uniform `columns`/`rows` grid.
