# Iron Ring: art correction brief (Art Pack v2.1)

**For:** whoever produces the artwork (an artist, an image-generation pipeline, or both).
**From:** the Iron Ring project, 2026-10-31.
**With:** the current Iron Ring source package. Every path below is relative to its root.

Art Pack v2 is in the game. The owner has played the whole arcade ladder with it and found no issues in play. This is a small correction round: it replaces the **crowd layers of the four venues**, and optionally touches up one **painted sign**. Nothing else changes.

This brief is self-contained. If anything disagrees, these repository documents are the authority:

- [`docs/art/asset-spec.md`](asset-spec.md) §3.1, layered stages: the technical contract.
- [`ArtSource/Pack-v2/CORRECTIONS.md`](../../ArtSource/Pack-v2/CORRECTIONS.md): what was wrong with the v2 crowds and how it was patched.
- [`docs/art/vertical-slice-art-brief.md`](vertical-slice-art-brief.md): the v2 brief, including the master palette (Appendix B).
- [`docs/design/setting.md`](../design/setting.md): world, cast and tone.

---

## 1. What to deliver

| # | Deliverable | File | Priority |
| --- | --- | --- | --- |
| C1 | **The Foundry crowd** (§3) | `Assets/Stages/Foundry/foundry_crowd.png` | Required |
| C2 | **Pier 9 crowd** (§3) | `Assets/Stages/PierNine/pier_nine_crowd.png` | Required |
| C3 | **Lantern Row crowd** (§3) | `Assets/Stages/LanternRow/lantern_row_crowd.png` | Required |
| C4 | **The Crown crowd** (§3) | `Assets/Stages/TheCrown/the_crown_crowd.png` | Required |
| C5 | The Foundry's painted sign, legible (§4) | `Assets/Stages/Foundry/foundry_wall.png` | Recommended |
| P | The four stage previews, recomposed with the tool, not drawn (§5) | `Assets/Stages/<Venue>/<venue>.png` | Required |

**Not wanted:** keep everything else exactly as it is.

- Fighter frames, atlases, portraits, champion frames, ending pictures.
- Every other stage layer.
- The title logo, the menu backdrop, effects, fonts and the HUD.
- `Assets/Data/stages.json`: the layer sizes, positions and timings stay the same, so the JSON does not change.

---

## 2. Why: the v2 crowds

Each crowd layer is a row of spectators behind the venue's barrier (rail, rope or bench). It plays a slow resting bob, and cheers when the fight gets exciting. In Art Pack v2:

1. **Heads were cut off.** The 768 px row is built from six 128 px groups. In every venue and every frame, groups 2, 3 and 5 (counting from 0) were a flat band of shoulders with no heads. The Crown's were an empty bench. The repository patched this by copying the intact groups over the broken ones (`Tools/Art/repair_crowds.py`). That copying is now the second problem.
2. **Only three distinct groups.** After the patch, each venue repeats three groups twice: 0 1 4 0 4 1. At 0.8 parallax the camera travels 128 px across the crowd, so the repeat is visible. The crowd should be one continuous row in which no spectator appears twice.
3. **The cheering arms read as bent sticks.** In frames 4–7 each group lifts exactly two arms. They are flat, dark bars with no hand or fist, no sleeve and no rim light, and the bodies under them barely move. A cheer should read as a crowd coming alive.

The patched v2 layers in the repository are the **reference for position and barrier only** (§3.2). Redraw the people.

---

## 3. Crowd layers (C1–C4)

### 3.1 File format (unchanged from v2)

- **768 × 672 px RGBA PNG**, hard alpha (every pixel fully opaque or fully transparent).
- **8 frames stacked vertically, 84 px each.** Frame *n* occupies rows 84·*n* to 84·*n* + 83. Each frame is the full 768 px width.
  - **Frames 0–3: at rest.** A loop, shown at 14 ticks per frame (about 4.3 frames a second) and faster as the crowd gets restless.
  - **Frames 4–7: cheering.** A loop, played when excitement runs high (after big hits, throws and a K.O.), then the crowd calms back to rest.
- Placement in `stages.json` stays the same: parallax 0.8, x 0, y 186 (y 174 at The Crown). Screen y = layer y + row, so a head at row 32 sits at screen y 218 (y 206 at The Crown).
- Colours: the venue's existing crowd and barrier colours. Within a venue, stay within the colours the v2 layer already uses, plus up to 8 new ones for hands and rim light.

### 3.2 Keep the barrier exactly as it is

Start from the current file in the source package and redraw only the spectators. From the first barrier row down, every pixel stays as delivered, because the floor layer and the camera framing line up with it:

| Venue | Barrier | Rows kept exactly (each frame) | Spectators drawn in rows |
| --- | --- | --- | --- |
| The Foundry | steel rail, rail bars at rows 60–63 and 74–77, posts between | 60–83 | 4–65, behind the rail |
| Pier 9 | dock rail, same layout as the Foundry | 60–83 | 4–65, behind the rail |
| Lantern Row | rope on brass posts; the rope sags across rows 54–64 | the rope pixels, and rows 65–83 | 4–64, behind the rope |
| The Crown | bench back and the terrace wall below it | 60–83 | 4–59, seated behind the bench |

"Behind" means the barrier pixels are drawn over the people. Bodies may continue down behind the rail and the rope, and show through the gaps between bars and posts as they do now.

### 3.3 The spectators

- **One continuous row, all different.** About 40–48 spectators across the 768 px, as now. No figure may be a copy, or a mirrored copy, of another figure in the same layer. Keep some variety of height, build, hats and caps, coats, a raised bottle or a betting slip.
- **Heads and shoulders always show.** In every frame, and along the full width, every spectator's head and shoulders are above the barrier.
  - **Head tops at rows 26–36 at rest.** The v2 rows were 30–35.
  - **Every 128 px column strip must contain a head reaching above row 37** in each of frames 0–3. The art self-test checks this strip by strip; a strip with no head counts as cut off.
  - An empty bench at The Crown is fine only where seated spectators fill the rest of that strip.
- **Silhouettes with rim light, no faces.** The same reading as v2 (asset spec §3), lit from the venue's key light:
  - The Foundry: furnace orange from below and behind.
  - Pier 9: sodium floodlight from the right.
  - Lantern Row: warm lantern light from above; its crowd may keep the muted coat colours it has now.
  - The Crown: dawn from behind, giving warm edges.
- **Rest frames 0–3.** A calm loop of 1–2 px moves: shoulders breathing, a head turning, a bottle lifted, someone leaning on the rail. No more than a third of the figures move in any one frame. Frame 3 must lead smoothly back to frame 0.
- **Cheer frames 4–7.** The crowd comes alive:
  - About half the spectators raise one or both arms, not a fixed number per group.
  - Arms are 3–4 px thick and attached at the shoulder, with a visible fist or open hand and a sleeve colour that matches the coat. They carry the venue's rim light.
  - Some wave hats or slips. Bodies rise 1–3 px on the jump.
  - Frame 7 leads back to frame 4.
  - Raised hands may reach up to row 4 and never above it.
- **Nothing floats.** Every arm and every hat connects to a spectator.
- **No text, no logos, no franchise characters**, and nothing that reads as a real person.

### 3.4 Avoid

- Cutting figures off at a fixed row (the v2 fault): check the top edge of every 128 px strip.
- Tiling: no repeat at 128, 192 or 256 px.
- Stick arms: an arm without a hand, sleeve or light.
- Bright highlights: the crowd sits just below the fighters' heads, so keep it darker than the fighters (asset spec §3: the bottom 40% of the palette's values, rim light excepted).

---

## 4. The Foundry's painted sign (C5, recommended)

`foundry_wall.png` is 680 × 360 RGB (parallax 0.25). It carries the faded painted lettering **VOLL WORKS** across rows 52–72. At 1× the weathered **K** loses its leg and the **S** fades out, so the word reads "WORI S".

- Keep the weathering, the colours and the position.
- Make every letter of **VOLL WORKS** readable at 1× on the 640 × 360 screen.
- Change no other pixels of the layer.

Skip C5 if it can't be done without repainting the wall.

---

## 5. Previews (P): recompose, don't draw

Each venue's `<venue>.png` (640 × 360 RGB) is used on the stage select, the title and the arcade ending. It must equal the layers composed exactly as the game draws them: frame 0 of every layer, camera at the middle of the stage, no drift. After placing the new layers in `Assets/Stages/`, run:

```sh
python3 Tools/Art/compose_previews.py
```

The art self-test fails if a preview and its layers disagree.

---

## 6. Check before delivering

With the source package (Python 3.13 and `pip install -r Tools/Art/requirements.txt`):

1. Put the new files in `Assets/Stages/<Venue>/` and run `python3 Tools/Art/compose_previews.py`.
2. `python3 Tools/Art/repair_crowds.py --check` reports 0 clipped group frames for all four venues. **Do not run it without `--check`**: that would overwrite the new art with copies.
3. `python3 Tools/Art/selftest.py` ends with `art selftest: ok`. It checks the crowds strip by strip and the previews against their layers.
4. Optional, if you can build the game: `IronRing -demo -stage <id>` for `pier_nine`, `lantern_row`, `foundry`, `the_crown`, and watch through a K.O. to see the cheer. `-assets-folder Assets` uses the files without repacking.
5. Look at every frame at 1× and at 4× on a mid-grey background. No heads cut off, no repeated figures, nothing floating, and the barrier unchanged (compare rows from §3.2 with the v2 file: the pixels must be identical).

---

## 7. Delivery package

One ZIP, **`IronRing_ArtPack_v2.1.zip`**, with paths that mirror the repository:

```text
IronRing_ArtPack_v2.1/
  README.md                   what changed, per deliverable; anything not done and why
  LICENSE                     zlib, as the project (the art is contributed under it)
  CONTENTS_SHA256.txt
  Assets/Stages/Foundry/foundry_crowd.png
  Assets/Stages/Foundry/foundry_wall.png     (only if C5 is done)
  Assets/Stages/Foundry/foundry.png          (recomposed preview)
  Assets/Stages/PierNine/pier_nine_crowd.png
  Assets/Stages/PierNine/pier_nine.png
  Assets/Stages/LanternRow/lantern_row_crowd.png
  Assets/Stages/LanternRow/lantern_row.png
  Assets/Stages/TheCrown/the_crown_crowd.png
  Assets/Stages/TheCrown/the_crown.png
  Previews/                   per venue: a contact sheet of the 8 frames at 2×, and a GIF of rest then cheer
  Source/                     optional: originals and prompts (large originals stay out of the repository)
```

- Don't include unchanged files, patches to code, or a new `stages.json`.
- The project will import the files, run the checks above, and look at them in the game. Then `Tools/Art/repair_crowds.py` is retired.

---

## Appendix: prompt notes for image generation

The asset spec's rules apply, and the generated image must then be registered onto the v2 layer, keeping its barrier rows (§3.2):

> Side view, a single continuous row of 40–48 standing spectators seen from behind a steel rail, waist up, 1990s arcade pixel art, dark silhouettes with a thin orange rim light from below (furnace glow), varied hats, caps, coats and builds, no faces, no text, transparent background, pixel-crisp, limited palette, 768 px wide.

The same, cheering: about half raise fists or hats, with arms drawn with sleeves and hands, and bodies lifted slightly.

Swap the light for each venue: the floodlight from the right (Pier 9), warm lanterns from above (Lantern Row), dawn from behind (The Crown, seated on a bench).
