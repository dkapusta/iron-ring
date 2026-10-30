# Iron Ring: art brief for the M2 vertical slice (Art Pack v2)

**For:** whoever produces the artwork (an artist, an image-generation pipeline, or both).
**From:** the Iron Ring project, 2026-10-30.
**With:** the Iron Ring source package. Every path below is relative to its root. The game, the import tools and the validation tools are all in it.

The vertical slice is the complete local game, Dee Marlowe vs Bram Hollis, at release quality:

- title, menus, character and stage select;
- arcade ladder with continues and an ending;
- versus, training and options;
- four venues.

All code for it is done. This pack replaces the remaining **placeholder art** and **corrects** the parts of Art Pack v1 that review found lacking.

This brief is self-contained: the numbers you need are written here. The repository documents are cited for detail and are the authority if anything disagrees:
- [`docs/art/asset-spec.md`](asset-spec.md): the technical contract;
- [`docs/art/frame-review.md`](frame-review.md): the review of Art Pack v1;
- [`docs/art/generation-prompts.md`](generation-prompts.md): prompts and workflow;
- [`docs/design/setting.md`](../design/setting.md): world, cast and tone.

---

## 1. What to deliver

| # | Deliverable | Size and format | Priority |
| --- | --- | --- | --- |
| F1 | **Dee: corrected frames** (§3.2) | 256 × 256 RGBA frames | Required |
| F2 | **Bram: corrected frames** (§3.2) | 256 × 256 RGBA frames | Required |
| F3 | **Champion pose animation**, 6 frames per fighter (§3.3): the arcade-ending victory | 256 × 256 RGBA frames | Required |
| F4 | Select portraits, one per fighter (§3.4) | 48 × 48 RGBA | Recommended |
| F5 | Ending pictures, one per fighter (§3.5) | 640 × 360 RGB | Recommended |
| S1 | **The Foundry**: final art for the new layered venue (§4.2) | 6 layers + preview | Required |
| S2 | **Pier 9**: layered (parallax) version (§4.3) | 5–6 layers + preview | Required |
| S3 | **Lantern Row**: layered version (§4.4) | 5–6 layers + preview | Required |
| S4 | **The Crown**: layered version (§4.5) | 5–7 layers + preview | Required |
| U1 | **Title logo** (§5) | 432 × 96 RGBA | Required |
| U2 | **Menu backdrop** (§5) | 640 × 360 RGB | Required |

**Not wanted** (keep the current ones):
- hit sparks and other effects;
- fonts;
- HUD;
- application icon;
- the four locked fighters (Sable, Idris, Greta, Jun: they come in M3);
- new venues beyond the four above;
- any music or sound.

If you think one of these needs work, add a note to the delivery instead.

---

## 2. Rules for everything

| Rule | Value |
| --- | --- |
| Screen | The game draws on a **640 × 360** logical canvas, scaled by whole numbers. 1 art pixel = 1 screen pixel. |
| Pixel art | Hard pixel edges. No anti-aliasing against transparency. **Alpha is 0 or 255** only. |
| Files | PNG, 8-bit, sRGB, no colour profile that changes values. Sprites and layers RGBA; opaque backgrounds RGB. |
| Palette | Stages and UI: the master palette (Appendix B, `Assets/Data/palette.gpl`). Each fighter: one limited palette of **at most 48 colours**, fitted to the fighter; keep Art Pack v1's colours where possible. |
| Outline | Every sprite silhouette has a 1 px outline in the **darkest tone of the material it bounds**: not pure black, and never a light or cyan rim. |
| Light | Warm key light from the upper left, a cool teal fill from the right kept *inside* the outline. Night venues; The Crown is at dawn. |
| Shading | About 4 tones per material plus its line colour. Gradients use **ordered (Bayer) dithering**, never smooth blends. |
| Tone | Gritty, after-hours, early-90s arcade. **No blood or gore.** No text except in-world signs. |
| Originality | Nothing from Fatal Fury, Street Fighter, the "Brass Current" concept kit or any other game: no characters, stages, logos or recognisable poses. No real brands or real people. |

---

## 3. Fighters

### 3.1 Frames: canvas, registration, scale

- One PNG per frame, named exactly as the frame (`idle_0.png`, `champion_3.png`).
- **256 × 256 RGBA**, character **facing right**. The game mirrors it for player 2.
- **Origin at (128, 232):** the point on the ground midway between the feet in the idle stance. It is the same body point in every frame, so nothing slides when frames change.
- **One scale per fighter for every frame.** Never fit a frame to a box.
  - Standing height (feet to the top of the head) is **Dee 130 px** and **Bram 134 px**, as in Art Pack v1.
- Feet on the ground line (y 232). Nothing more than 3 px below it.
- The body fills its hurtbox. Boxes are x from the origin, forwards; y up from the ground; then width and height:

  | Fighter | Standing | Crouching |
  | --- | --- | --- |
  | Dee | `[-16, 0, 32, 124]` | `[-18, 0, 38, 88]` |
  | Bram | `[-20, 0, 40, 128]` | `[-22, 0, 44, 92]` |

- Art Pack v1's frames are the starting point. They are in `ArtSource/Fighters/<Name>/frames/` at exactly this canvas and origin, so you can edit them directly.
  - For generation, export pose references at 4× (1024 × 1024, origin 512, 928):

    ```sh
    python3 Tools/Art/import_art.py refs dee  refs/dee
    python3 Tools/Art/import_art.py refs bram refs/bram
    ```

    This also writes procedural pose references for frames that don't exist yet (`champion_0`…`champion_5`) on the same canvas, at the same origin and scale.

### 3.2 Corrections to Art Pack v1 (F1, F2)

From the frame review ([`frame-review.md`](frame-review.md)). It measured every frame and played them in the game. Deliver every frame you change. Unchanged frames can be left out, as long as the ones you deliver match their scale and palette.

**P1: readability (required).** Players misread these moves today.

| Fighter | Frames | Correction |
| --- | --- | --- |
| Bram | `lp_1` (Palm Shove) | Both palms pushed out at **full arm length**, fingers up, heels of the hands forward. The hands reach about **x +55 to +60** from the origin, at chest height; shoulders forward. Today the hands stay at the chest. |
| Bram | `hp_1` (Anchor Hook) | A real hook: the lead arm bent 90°, sweeping across. The fist reaches **x +60 to +65**, the torso turns into it, the rear heel lifts. |
| Bram | `throw_0`, `throw_1` | `throw_0`: a lunge with the arms wide. `throw_1`: big hands clamped on the opponent's shoulders, elbows bent. Today the two differ by 5 %. |
| Bram | `special_0`, `special_1` (Dock Hoist grab) | `special_0`: a low lunge, arms spread wide (a bear-hug reach). `special_1`: arms closed around the opponent's waist. |
| Dee | `throw_0`, `throw_1`, `throw_hold_0` | `throw_0`: both arms forward at chest height, open hands. `throw_1`: hands closed on collar and upper arm, elbows bent, weight forward. `throw_hold_0`: the clinch, opponent pulled chest to chest, rear foot braced. Today all three are the same stance. |
| Dee | `special_2` (Rush Uppercut) | Angle the arm **20–30° forward** so forearm and fist sweep through **x +15 to +35**, rising onto the toes. Today the fist goes straight up, outside most of the hitbox `[10, 60, 40, 90]`. |

**P2: animation quality (required for release quality).**

| Fighter | Frames | Correction |
| --- | --- | --- |
| Both | `idle_0`–`idle_3` | Breathing: head and shoulders **0, 1, 2, 1 px** lower across the four frames; guard hands follow by 1 px; feet don't move; head within ±1 px horizontally. Bram's head jumps about 6 px today. |
| Both | `walk_0`–`walk_5` | The torso stays within **±2 px** of the centre line (Dee swings 4 px, Bram 5–11 px today). `walk_0`/`walk_3` are contact poses (feet apart, body 1–2 px lower). `walk_1`/`walk_4` are passing poses (feet together, 1 px higher). The planted foot stays planted and the rear foot pushes off. Bram plants heavily. |
| Bram | `hp_2`, `hp_3` | `hp_2`: the fist carries past the centre line (follow-through). `hp_3`: the arm comes back to guard and the body unwinds. Today both are near-idle. |
| Bram | `throw_hold_0` | The lift: knees bent, the opponent pulled up to chest height. Today it repeats `throw_0`. |
| Bram | `hit_crouch` | A flinch: head snapped back, arms in, shoulders hunched. Today the arms reach forward and read as a grab. |

**P3: polish (wanted; deliver if time allows).**

| Fighter | Frames | Correction |
| --- | --- | --- |
| Both | all frames | **Outline pass:** recolour every silhouette-edge pixel to the darkest tone of its material, and remove the generator's cyan and light rim. Bram has 11 % light and 9 % cyan edge pixels today. It shimmers on the teal-lit harbour. |
| Both | `win_1` | A clear change from `win_0`. Dee: fist higher and head thrown back, or the other arm raised. Bram: a shout, chin up, mouth open, fists higher. |
| Dee | `hit_0`, `hit_1` | `hit_0`: the snap, head back and shoulders up. `hit_1`: the stagger, weight on the rear foot, arms dropping. |
| Dee | `lp_0` | A small chamber: lead fist pulled back 2–3 px, lead shoulder dropped slightly. |
| Bram | `hoist_1` | Thicker arms. Today they are thin and straight against his bulk. |

Keep the throw-hold partner points. Every frame listed below has a `partner` offset in `frames/manifest.json`: where the opponent's origin goes, `[dx, dy]` from this fighter's origin, dx forwards and dy up.

| Fighter | Frames with partner offsets |
| --- | --- |
| Dee | `throw_hold_0`, `throw_hold_1` |
| Bram | `throw_hold_0`, `throw_hold_1`, `hoist_0`, `hoist_1`, `hoist_2` |

If a correction moves the hands, update that frame's offset in the manifest so the opponent still lands in them.

### 3.3 New: the champion pose (F3)

The arcade ending shows the winner on the roof of The Crown at dawn, alone, holding up **a length of the Iron Ring's own chain**. This is the welded chain that marks every fighting floor in the game.
- **Six frames per fighter:** `champion_0`–`champion_2` play once, then `champion_3`–`champion_5` loop.
- Same canvas, origin and scale as every other frame (§3.1).
- Draw it facing right; it is never mirrored.

**The chain** is a prop drawn into the sprite:
- welded steel links, alternating flat (an open oval about 7 × 4 px) and edge-on (a bar about 5 × 2 px);
- about 16–20 links, roughly 60–70 px of chain;
- the shared `metal` ramp (`#0b0d12`, `#3d4a66`, `#5a6782`, `#8a93a8`, `#d5dbe6`), counted within the 48 colours;
- the same look as the ring on the stage floors (see the floor of any venue);
- 1 px outline like the rest of the sprite.

There is room above: the canvas allows up to 232 px above the ground, so raised arms with the chain can reach 190 px.

| Frame | Ticks (60/s) | Dee Marlowe | Bram Hollis |
| --- | --- | --- | --- |
| `champion_0` | 20 | Turned slightly towards the viewer, the chain hanging from both wrapped hands at waist height, head down, catching her breath. | Chain draped over one shoulder, both fists at his belt, chest out, looking at the crowd. |
| `champion_1` | 16 | Lifting: the chain rising past her face, elbows coming up. | Pulling the chain off his shoulder with both hands, arms rising wide. |
| `champion_2` | 14 | Both fists overhead, the chain stretched taut between them, shouting, up on the balls of her feet. | Arms wide and high, the chain held taut across above his head, roaring, mouth wide. |
| `champion_3` | 18 | Holding the chain overhead, mouth closed, chest up (loop start). | Holding it high and taut, chest out (loop start). |
| `champion_4` | 18 | The same, shoulders 1 px lower (breath), chain sagging 1–2 px. | The same, shoulders 1 px lower, chain sagging 1–2 px. |
| `champion_5` | 18 | The same, head thrown back, shouting. | The same, chin up, roaring. |

Rules:
- Consecutive frames differ visibly (at least 12 % of the silhouette between distinct key poses).
- The loop (3 → 4 → 5 → 3) must not pop: the feet stay planted and the torso within ±2 px.
- The pose references from `import_art.py refs` place the hands and body; draw the chain between the hands.

### 3.4 Select portraits (F4, recommended)

`dee_select.png`, `bram_select.png`: 48 × 48 RGBA. They are shown 1:1 in the character-select grid; today the 32 × 32 HUD portrait is scaled up 1.5× and looks soft.
- Head and shoulders facing right, the same likeness as the HUD portrait.
- 1 px outline; dark `night_1` (`#141a30`) backdrop with a dithered warm glow from above.
- The fighter's side colour at the bottom edge: teal for Dee, rust for Bram.

The 32 × 32 HUD portraits can stay as they are. Touch them up only if you redraw a face.

### 3.5 Ending pictures (F5, recommended)

`dee_ending.png`, `bram_ending.png`: 640 × 360 RGB. Shown full-screen behind the text "<NAME> / TAKES THE CROWN" and "THE IRON RING HAS A NEW CHAMPION".
- **The scene:** The Crown's roof terrace at dawn (coral and amber sky, the half-dead CROWN HOTEL bulb sign, the water tower), the champion in their `champion_2` pose with the chain overhead, a few spectators as silhouettes at the parapet.
- **Text bands:** keep **y 0–96** and **y 292–360** calm and dark-ish; the game draws a translucent band and text there. The figure stands in the middle band.
- **Style:** the same pixel-art rules as the stages. The figure can be drawn larger than in-game (up to about 200 px tall) as long as it reads as the same character.

Without these pictures, the ending shows The Crown's stage art with the champion sprite.

### 3.6 Fighter workflow

1. `python3 Tools/Art/import_art.py refs <id> refs/<id>`: pose references for every frame, including the new ones.
2. One generation (or drawing) **per frame**, with that frame's reference as the pose input and a character sheet as the identity input. Prompts and a style block are in Appendix A. Prompt for the extreme of the pose ("arm fully extended", "fist past the centre line").
3. Clean up by hand: hands, feet on the ground line, the outline.
4. Bring frames to the 256 × 256 canvas. Either:
   - import 1024 × 1024 references back with the command `refs` prints (`--origin 512 928 --scale 0.25`); or
   - deliver frames at 256 × 256 directly; I import them with `--origin 128 232 --scale 1 --no-quantize`.
5. Check them (§8).

---

## 4. Stages

### 4.1 How layered stages work

Every venue becomes **800 px wide**. The camera shows 640 px and follows the fighters, so it travels 160 px from end to end. Each layer scrolls by its **parallax** factor:
- 0 is fixed to the screen;
- 1 moves with the floor;
- above 1 is a foreground that passes faster and is drawn over the fighters.

Layers are separate RGBA PNGs, transparent wherever the layers behind should show through. They are stacked back to front.

**Width rule.** A layer must be at least `640 + 160 × parallax − x` px wide, where `x` is its left offset (usually 0). A drifting layer is the exception: it repeats horizontally, so it can be any width but its left and right edges must tile.

| Parallax | Minimum width |
| --- | --- |
| 0 | 640 |
| 0.25 | 680 |
| 0.3 | 688 |
| 0.5 | 720 |
| 0.55 | 728 |
| 0.6 | 736 |
| 0.8 | 768 |
| 1.0 | 800 |
| 1.25 | 840 + any negative x |

**Animated layers.** Frames are stacked vertically in one PNG (each frame the full layer width).
- Plain animated layers loop at a set rate: flickering lanterns, blinking bulbs.
- A **crowd** layer has **8 frames**: frames 1–4 are the crowd at rest (small bob, a few heads turning), frames 5–8 the crowd cheering (arms up, bigger bob). The same people stand in the same places in every frame. The game switches between the halves as the fight heats up.

**Layout rules hold on screen at every camera position:**
- **Ground line y 316.** The fight floor band is about y 296–340.
- **The chain ring** is a flattened ellipse of welded chain on the floor, centred at **world x 400, y 326**, about 564 × 48 px. It is the game's signature and appears on every venue.
- **HUD bands:** **y 0–52** and **y 330–360** stay calm and dark-ish. No bright lights, busy detail or text there.
- **Spectators** stand behind a barrier whose top is below **y ≈ 250**, so they never overlap the fighters' heads.
- The floor is lower-contrast than the fighters.
- Foreground (`front`) layers frame the stage **ends only**, so they never cover the middle 560 px. Dark silhouettes; nothing in the HUD bands' middle.

**Preview.** Also deliver `<venue>.png` (640 × 360 RGB): the layers composed with the camera in the middle (x = 80), crowd at rest. It appears on the stage select and the title screen. A preview script is fine; I can make it on import.

**Layer list.** For each venue, list in the delivery's `README.md` each layer's file, parallax, x, y, and (if animated) frames and ticks per frame, or fill in a `layers.json`. I turn it into the game's `Assets/Data/stages.json` entry. The Foundry's entry there is a working example.

### 4.2 The Foundry (S1): final art for the new venue

The shut-down **Voll Works**, Greta Voll's old plant: the ring laid between cold furnaces under a hanging ladle, the last night shift betting behind a steel rail.

The current placeholder shows the exact composition and sizes:
- files: `Assets/Stages/Foundry/`;
- generator: `python3 Tools/Art/generate.py foundry --assets out`;
- in the game: `IronRing -stage foundry -demo`.

Replace it layer for layer:

| File | Parallax | Size | Position | Content |
| --- | --- | --- | --- | --- |
| `foundry_wall.png` | 0.25 | 680 × 360, opaque | 0, 0 | Back wall of the foundry hall at night: soot-dark brick, three or four tall arched windows with cold teal light and broken panes, steel girders between them, a painted **VOLL WORKS** sign at about y 56 with two letters gone dark. |
| `foundry_furnace.png` | 0.55 | 728 × 360 | 0, 0 | Three or four brick furnaces with glowing coral-orange mouths at about y 215, a gantry beam at y 86–93, a steel ladle hanging from it with a still-glowing rim. Transparent above and between. |
| `foundry_steam.png` | 0.55, drift −6 px/s | 640 × 360, **tiles** | 0, 0 | A few sparse dithered puffs of steam between y 110 and 230; nothing else. |
| `foundry_crowd.png` | 0.8 | 768 × 84 per frame, **8 frames** (768 × 672) | 0, 186 | Spectator silhouettes, heads and shoulders, warm rim light from the furnaces behind. A steel rail at y 60–63 *of the frame*, posts every ~48 px. Frames 1–4 at rest, 5–8 cheering. |
| `foundry_floor.png` | 1.0 | 800 × 98 | 0, 262 | Riveted steel deck plates, low contrast, slag stains, furnace light pooling behind the ring; the **chain ring** centred at x 400, y 64 of the layer. |
| `foundry_front.png` | 1.25 | 850 × 360 | −10, 0 | Two dark riveted steel columns at the very ends; everything else transparent. |

### 4.3 Pier 9 (S2): layered version

Keep Art Pack v1's Pier 9 (`Assets/Stages/PierNine/pier_nine.png`) as the look and the colour key: a closed cargo pier after the night shift, rust-red gantry crane legs, a moored freighter with a floodlight, a teal harbour, the city skyline, dockers and bookmakers behind a steel rail, wet planks. **Split it into layers and widen it to 800 px.** Extend each layer at its edges, more crane structure and pier, not stretched pixels.

| Layer (suggested file) | Parallax | Minimum size | Content |
| --- | --- | --- | --- |
| `pier_nine_sky.png` | 0.2 | 672 × 360, opaque | Night sky with a few stars, the distant city skyline with lit windows, the far shore. |
| `pier_nine_harbour.png` | 0.2, optional drift | 672 wide band, or tiling | Teal harbour water with light reflections. A 2–4 frame shimmer is welcome. |
| `pier_nine_ship.png` | 0.5 | 720 × 360 | The moored freighter and its floodlight beam, distant cranes, lamp posts with sodium glow. |
| `pier_nine_crowd.png` | 0.8 | 768 wide × ~84 per frame, 8 frames | Dockers and bookmakers behind the steel rail, chalk betting boards, crates and bollards along it. Rest / cheering halves. |
| `pier_nine_floor.png` | 1.0 | 800 wide, from the rail's foot (~y 250) down | Wet planks with sodium and teal reflections, the chain ring centred at x 400, y 326. |
| `pier_nine_front.png` | 1.25, front | 840+ × 360 | The big rust-red gantry crane legs, moved out to frame the ends (today they sit at the screen edges). |

### 4.4 Lantern Row (S3): layered version

From Art Pack v1's Lantern Row: a night market after curfew, the station's brick facade with its old clock, shuttered stalls under striped awnings, strings of paper lanterns half of them dark, the ring roped off on worn tiles, traders and strangers behind red rope.

| Layer | Parallax | Minimum size | Content |
| --- | --- | --- | --- |
| `lantern_row_facade.png` | 0.3 | 688 × 360, opaque | The brick station facade with the old **station clock** centred near x 344, arched doorways, lit windows; dark sky above. |
| `lantern_row_lanterns.png` | 0.45, **2–3 frames** | 712 wide | Strings of paper lanterns crossing the street, some dark; a slow flicker between frames (a few lanterns change brightness). |
| `lantern_row_stalls.png` | 0.6 | 736 × 360 | Shuttered stalls under red-and-white and teal-and-white striped awnings, posters on the shutters. |
| `lantern_row_crowd.png` | 0.8 | 768 wide × ~84 per frame, 8 frames | Traders and well-dressed strangers behind the red rope and its brass posts. Rest / cheering. |
| `lantern_row_floor.png` | 1.0 | 800 wide, from the rope's foot (~y 270) down | Worn tiles with lantern reflections; the chain ring centred at x 400, y 326. |
| `lantern_row_front.png` | 1.25, front (optional) | 840+ × 360 | A stall's corner post with a hanging lantern at each end. |

### 4.5 The Crown (S4): layered version

From Art Pack v1's The Crown: the tar-and-gravel roof terrace of a derelict grand hotel at first light. A coral and amber dawn sky behind the city, the **CROWN HOTEL** bulb sign on a steel truss with dead bulbs, a wooden water tower on stilts, a brick parapet with a few spectators. This is the final bout: it should feel like the top of the world.

| Layer | Parallax | Minimum size | Content |
| --- | --- | --- | --- |
| `the_crown_sky.png` | 0 | 640 × 360, opaque | The dawn sky: night blue at the top through coral to amber at the horizon, dithered. |
| `the_crown_clouds.png` | 0.1, drift +3 px/s | tiling | Thin dawn clouds, lit from below. |
| `the_crown_city.png` | 0.25 | 680 × 360 | The city skyline in silhouette against the dawn, a few windows still lit. |
| `the_crown_sign.png` | 0.5, **2 frames** | 720 × 360 | The steel truss with the **CROWN** and **HOTEL** bulb letters, some bulbs dead; frame 2 blinks a few bulbs. The wooden water tower on its stilts. |
| `the_crown_crowd.png` | 0.8 | 768 wide × ~84 per frame, 8 frames | The brick parapet, a few spectators (sparser than the other venues, wealthier: coats, hats), vents and pipes. Rest / cheering. |
| `the_crown_floor.png` | 1.0 | 800 wide, from the parapet's foot (~y 252) down | Tar and gravel roof, puddles reflecting the sky, the chain ring centred at x 400, y 326. |
| `the_crown_front.png` | 1.25, front (optional) | 840+ × 360 | A chimney stack or a roof vent at each end. |

---

## 5. Front end (U1, U2)

The placeholders show size and layout:
- files: `Assets/Resources/UI/title_logo.png` and `menu_backdrop.png`;
- generator: `python3 Tools/Art/generate.py ui --assets out`.

**Title logo: `title_logo.png`, 432 × 96 RGBA, transparent background.**
- "IRON RING" in heavy block capitals made of riveted brass plates, with a warm top-lit gradient from pale gold to dark amber.
- A 2 px near-black outline (`#07090f`) and a hard drop shadow.
- A ring of welded chain passing behind the letters as a flattened ellipse. No other text.
- It sits at y 34 on the title screen, centred, over a darkened venue. It must read at 1× from across a room, on dark and on lit backgrounds.
- The letter shapes should feel related to the game's squared bitmap font (see the HUD).
- Generators blur thin outlines and invent letters: generate a reference if useful, then redraw by hand.

**Menu backdrop: `menu_backdrop.png`, 640 × 360 RGB.** It sits behind the character select, stage select, options and controls screens.
- A dark, empty fight floor at night seen from the front.
- Two cold floodlight beams falling from above the frame.
- The chain ring on the floor in the lower third.
- A crowd's heads and shoulders in silhouette along the bottom edge.
- Thin brass rules near **y 60** and **y 336**.
- The **centre band y 60–336 stays dark and even**: portraits, full-size fighters and white text sit there. No text in the art.

---

## 6. Prompts

Appendix A has the character prompts, the style block and the negative prompt from [`generation-prompts.md`](generation-prompts.md); that file also has a prompt fragment for every existing frame. Stage layers use the shared stage block there plus "separate background layer for a side-scrolling fighting game, [what this layer holds], everything else fully transparent".

Image generators don't keep a crowd consistent across frames. **Generate crowd frame 1, then make the others by hand-editing it**: raise arms, shift heads by 1–3 px.

---

## 7. Delivery package

Name it `IronRing_ArtPack_v2/`, laid out like the game's folders so it imports mechanically:

```text
IronRing_ArtPack_v2/
  README.md               what is in the pack, open issues, the stage layer lists (§4.1)
  PROVENANCE.md           how it was made: tools, models, inputs (see below)
  Fighters/Dee/frames/    <frame>.png, 256 × 256, origin (128, 232); manifest.json (partner offsets)
  Fighters/Dee/           dee_select.png (48 × 48), dee_ending.png (640 × 360)
  Fighters/Bram/frames/   ... the same for Bram
  Fighters/Bram/          bram_select.png, bram_ending.png
  Stages/Foundry/         foundry_*.png layers, foundry.png preview
  Stages/PierNine/        pier_nine_*.png layers, pier_nine.png preview
  Stages/LanternRow/      lantern_row_*.png layers, lantern_row.png preview
  Stages/TheCrown/        the_crown_*.png layers, the_crown.png preview
  UI/                     title_logo.png, menu_backdrop.png
  Source/                 prompts, character sheets, and the large originals if you want to keep them
                          (they stay out of the game repository)
```

**`PROVENANCE.md`** must state:
- the tools and image models used;
- what was given to them as input: the project's own references, pose guides and earlier art only;
- that no franchise screenshots, characters, logos or real brands were used as inputs;
- that the pack is contributed under the project's **zlib** licence.

Art Pack v1's provenance is a good model: `ArtSource/Pack-v1/PROVENANCE.md`.

---

## 8. Check before delivering

The source package builds the game and the tools; `README.md` at its root explains how. Run these checks and fix what they find:

1. **Frames:** `python3 Tools/Art/review_frames.py --sheets review` measures the animation rules (breathing, rocking, key-pose difference, reach into hitboxes, crouch height, outline colours) and draws review sheets with boxes. Every P1 and P2 item from §3.2 should come out clean.
2. **Import:** import the frames, then build. AssetPacker validates everything at build time and names the file and the problem. Errors fail the build; warnings are printed. It checks, among others:
   - missing frames;
   - layer widths that don't cover the camera travel;
   - crowd frame counts;
   - preview sizes (a warning);
   - select portraits that aren't 48 × 48;
   - ending pictures that aren't 640 × 360.

   ```sh
   python3 Tools/Art/import_art.py fighter dee  <frames> --origin 128 232 --scale 1 --no-quantize --manifest <frames>/manifest.json
   python3 Tools/Art/import_art.py fighter bram <frames> --origin 128 232 --scale 1 --no-quantize --manifest <frames>/manifest.json
   cmake --build --preset release
   ```

3. **In the game.** The player is `Build/release/Player/Release/IronRing` (`.exe` or `.x86_64`):
   - `IronRing -training -stage pier_nine`, then press F3: hit and hurt boxes over the sprites, and frame data. The striking limb must be inside the box on active frames.
   - `IronRing -demo -stage <venue>`: the camera reaches both ends of each venue within a match. Look for seams, layers running out, and the crowd switching to cheering after a knockdown.
   - `IronRing -batchmode -autoplay -frames 14000 -keys "10:Return,30:Return,60:Return" -screenshot ending.png`: plays the whole arcade ladder as Dee and saves the ending with the champion pose. For Bram, move the cursor first: `-keys "10:Return,30:Return,50:Right,60:Return"`.
   - The title screen, the character select and the options screen (Escape backs out).
4. **Pixels:** alpha only 0 or 255; at most 48 colours per fighter; outlines in the darkest tone of each material; nothing in the HUD bands.

**What happens next.** I import the pack, review every frame and layer against this brief in the game, and send back a list of corrections with frame names. You correct, and we finish with a polish pass, as with Art Pack v1. Items marked P3 or Recommended can come in a second drop.

---

## Appendix A: prompts

**Style block**, appended to every sprite prompt:

> 16-bit arcade fighting game sprite, early-1990s style, hand-placed pixel art at low resolution, limited palette, 1-pixel dark coloured outline around the silhouette (no light or coloured rim on the outer edge), 4-tone cel shading with ordered dithering, warm key light from the upper left, subtle cool fill light from the right kept inside the outline, full body, character facing right, side-on three-quarter view, feet on a flat ground line, flat pure green (#00FF00) background, no shadow on the ground, no text, no border, no anti-aliasing

**Negative prompt** (where supported):

> blurry, soft gradients, painterly, 3D render, photo, anti-aliased edges, glowing rim light on the silhouette, halo, background scenery, multiple characters, cropped feet, extra limbs, text, watermark, logo, frame, gore, blood

**Dee Marlowe**, boxer, the protagonist:

> Dee Marlowe, a lean, athletic Black woman in her late twenties, banned Golden Gloves contender. Cropped dark curls, small silver stud earring, focused and calm expression. Teal sleeveless athletic top; charcoal track pants with a single chalk-white side stripe; white hand wraps and tape up to mid-forearm (no gloves); black high-top boxing boots. Compact boxer's stance, chin tucked, lead hand forward at shoulder height, rear fist by the cheek.

**Bram Hollis**, grappler, the Ring's gatekeeper:

> Bram Hollis, a huge, heavy-set white man in his forties, laid-off shipyard rigger. Shaved head, full dark-brown beard, heavy brow, broken nose. Charcoal short-sleeved T-shirt; rust-orange work overalls with the bib folded down and the straps hanging at the hips; brown leather wrist straps; black steel-toe work boots. Wide, low stance, open hands forward like he is about to grab, weight on the back foot.

**Champion frames**, added to the character prompt:

> victory on a rooftop at dawn, holding up a length of heavy welded steel chain with both hands, [the frame's pose from §3.3], triumphant

**Stage block** (stages and layers):

> 1990s arcade fighting-game background, pixel art at 640x360, wide side-on view of a fighting floor, the camera at chest height, a flattened ellipse of heavy welded chain lying on the floor in the centre as the ring, empty floor area in the lower third, spectators only as silhouettes behind a barrier in the middle distance, dark calm sky across the top fifth, limited palette, ordered dithering, no characters in the foreground, no text except in-world signs, no lens flare

## Appendix B: master palette

Stages and UI use these. Fighters may add their own fitted colours, up to 48 in total.

| Group | Colours |
| --- | --- |
| Night | ink `#07090f`, night_0 `#0c1020`, night_1 `#141a30`, night_2 `#1f2742` |
| Steel | steel_0 `#2b3550`, steel_1 `#3d4a66`, steel_2 `#5a6782`, fog `#8a93a8` |
| Sodium (warm light) | `#5a2e17`, `#9c4f1e`, `#d9822b`, `#f2b04e`, `#ffe3a1` |
| Teal (player 1, cold light) | `#0d2f36`, `#145560`, `#1f8a8c`, `#3fc4b8`, `#a6f0e0` |
| Coral (player 2, impact) | `#3d1216`, `#7a1f22`, `#bf3a2e`, `#ef6a45`, `#ffb48a` |
| Neutrals | chalk `#f1ead8`, bone `#cfc4a8`; wood `#2a1a12`, `#4a2e1d`, `#6e4528`, `#946238`; brick `#3a1a1a`, `#6b2e26` |
| Metal (chain) | `#0b0d12`, `#3d4a66`, `#5a6782`, `#8a93a8`, `#d5dbe6` |

The full set, with each fighter's material ramps: `Assets/Data/palette.json` and `palette.gpl` (GIMP / Aseprite palette).

## Appendix C: every fighter frame

The complete set each fighter has. **Bold** frames are new or must change in this pack.

```text
idle_0..3   walk_0..5   crouch   guard   guard_crouch   jump_rise   jump_apex   jump_fall
lp_0..2     hp_0..3     lk_0..2  hk_0..3
throw_0  throw_1  throw_hold_0  throw_hold_1
special_*   (Dee: special_0..3 · Bram: special_0, special_1, hoist_0..2)
hit_0  hit_1  hit_crouch   held_0  held_1   down_0..3   win_0  win_1
champion_0..5   (new)
```

Must change:

| Fighter | Required (P1, P2) | If time allows (P3) |
| --- | --- | --- |
| Dee | **idle_0..3, walk_0..5, throw_0, throw_1, throw_hold_0, special_2, champion_0..5** | win_1, hit_0, hit_1, lp_0; the outline pass on all |
| Bram | **idle_0..3, walk_0..5, lp_1, hp_1, hp_2, hp_3, throw_0, throw_1, throw_hold_0, special_0, special_1, hit_crouch, champion_0..5** | win_1, hoist_1; the outline pass on all |
