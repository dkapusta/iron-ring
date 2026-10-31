# Editable source assets

- `BaseFrames/`: project-adopted v1 frames and original partner manifests, before this pass.
- `References/`: registered pose guides, identity references and the hook correction guide.
- `Generated/`: accepted high-resolution raster originals. Preserve the whole canvas when importing fighter sources.
- `Studies/`: superseded first-pass poses retained as correction-guide inputs, not for runtime use.
- `Contracts/`: source snapshot fighter definitions and stage metadata.
- Prompt JSON: full generation/edit instructions and reference paths; the referenced project files came from the attached source or earlier project-owned art.
- Fighter palette/registration JSON: final palette selections and whole-canvas scale plus integer offsets. `native_polish.json` records additional explicit pixel edits.
- `title_glyph_mask.png`: editable native title-letter mask. Glyph construction and plate/rivet styling are in `Tools/build_ui.py`.

The final 256×256 frames in `Fighters/*/frames`, each separate stage layer and `UI/*.png` are authoritative editable pixel masters. Rebuild all exports with the ordered commands in the root README. Aseprite/GIMP or another pixel editor can edit these PNGs directly; no proprietary source format is required. Runtime transparency is hard alpha, and pixel scaling must use nearest-neighbour.
