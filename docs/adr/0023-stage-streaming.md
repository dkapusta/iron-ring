# ADR-0023: Stream each venue from archives of its own; keep two stages loaded

- **Status:** Accepted
- **Date:** 2026-10-31

## Context

With seven layered venues, the player decoded every stage at start-up: 40 layer textures, about 48 MiB of RGBA, plus seven encoded themes (3.5 MB), though a fight shows one venue and the menus only show 640 × 360 previews. Each new venue adds 5–8 MiB. The roadmap planned per-stage `sharedassetsN.assets` and `levelN` archives, the layout Unity players use for scene data.

## Decision

- **Packing.** `AssetPipeline.json` gives each venue, in `stages.json` order, a `sharedassetsN.assets` (its `Stages/<Name>` folder without the preview) and a `levelN` (its theme). Pipeline rules gain `files` (single files) and `exclude` (files or folders skipped while scanning) to express this. Previews move into `sharedassets0.assets`, which the menus need; the menu theme stays in `resources.resource`.
- **Mounting.** The player mounts the base archives at start and indexes the streamed ones: each is opened once, its file list kept, then closed. `vfs::StreamingArchives` mounts an archive while at least one key (a stage id) holds a path in it, and unmounts it when none does. It inserts streamed mounts below the loose `-assets-folder` folder (`VirtualFileSystem::insert`), so dev overrides keep working. Two streamed archives claiming one path is an error at start.
- **Loading.** Stage layers and stage themes are no longer loaded by `GameData`'s constructor. `GameData::useStage(index)`, called by `FightScene` before it builds the `StageView`, mounts the stage's archives, decodes its layers and reads its theme bytes. The two most recently used stages stay resident (`kResidentStages`); the next one evicts the oldest: `AssetLibrary::unload` frees its textures, its theme bytes are dropped, its archives are unmounted. Loading is synchronous; a stage takes 10–45 ms in a release build, inside the fade into the fight.
- **Checks.** A content test asserts that every stage's layers sit in one `sharedassetsN` (N ≥ 1) and its theme in one `levelN`, none shared, and the previews in `sharedassets0`. The player itself does not require the split: a path outside the streamed archives is read from wherever it is mounted.

## Consequences

- Stage textures held at once drop from 48 MiB to at most about 17 MiB, and start-up decodes 40 fewer PNGs. At most four of the fourteen streamed archives are open at once.
- Two resident stages cover what the game does in practice: a rematch, F2 cycling back, the arcade's previous venue during the versus card. The current fight's stage is always the most recently used, so it is never evicted under its `StageView`.
- Renderer recreation (HDR) still works: it re-decodes the textures in the cache, which are only those of resident stages, whose archives are mounted.
- Playing a theme copies its bytes into the audio system, so evicting a stage while its theme fades out is safe.
- A new venue needs two pipeline rules and two list entries (docs/asset-pipeline.md); the content test names what is missing.
- The simulation is untouched; the golden checksum holds.

## Alternatives considered

- **Load everything at start (as before):** simple, but memory grows with every venue, and so does start-up time.
- **One resident stage:** frees more, but a rematch or a quick F2 back would decode again; two cost about 8 MiB more.
- **Background loading on a thread:** SDL textures must be created on the render thread, and loads are short enough to hide in the fade. Revisit if final art makes stages much larger.
- **Keep the archives always mounted, unload textures only:** saves the same texture memory, but the file layout would not reflect streaming and a later move to compressed or downloadable stage packs would need it anyway.
