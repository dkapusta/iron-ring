# Generation prompts

Prompts for producing Iron Ring art with an outside image generator. Every result goes through `Tools/Art/import_art.py` and has to meet the [asset specification](asset-spec.md). Image generators are inconsistent across frames, so the workflow anchors each frame to a pose reference instead of trusting text alone.

## Workflow that keeps frames consistent

1. Export the current frames as pose references:

   ```sh
   python3 Tools/Art/import_art.py refs dee build/refs/dee      # 1024x1024, green key, origin (512, 928)
   ```

2. Make one **character sheet** first (front, side, back, plus a close-up of the face) from the character prompt below. Pick the best result; it is the identity reference for every frame.
3. For each frame, generate with the frame's reference as the **pose/structure input** (img2img at about 0.45–0.6 strength, or a pose or line-art control) and the character sheet as the **identity/style input**. Keep the canvas 1024 × 1024 and don't let the generator move or rescale the figure.
   - **One generation per frame.** Multi-pose boards tend to collapse distinct key poses into one generic stance (Art Pack v1: reach, grip and hold came out the same). Generate each frame on its own, with that frame's prompt fragment below.
   - **Never fit a frame to a bounding box.** All frames of a fighter share one scale; the importer derives it once (`--height` on `idle_0`, or `--scale`).
   - **Prompt for the extreme of the pose**, not "in the style of the reference": "arm fully extended", "fist past the centre line". Generators soften key poses unless told otherwise.
4. Clean up by hand where needed: hands, feet on the baseline, the outline. Then import, using the command `refs` printed:

   ```sh
   python3 Tools/Art/import_art.py fighter dee path/to/dee_frames --origin 512 928 --scale 0.25 --manifest build/refs/dee/manifest.json
   ```

5. Run `python3 Tools/Art/review_frames.py --sheets build/review` and go through its findings and sheets against [asset-spec §2.6](asset-spec.md#26-animation-rules).
6. Build, run `IronRing -training -stage <id>`, press F3 and check hitboxes. Fix the art or the boxes in `dee.fighter.json`.

If the generator can't follow a pose reference, generate larger poses freely, align them yourself on a shared canvas (feet on one baseline, same scale), and pass `--origin`/`--height` instead.

## Shared style block

Append this to every sprite prompt:

> 16-bit arcade fighting game sprite, early-1990s style, hand-placed pixel art at low resolution, limited palette, 1-pixel dark coloured outline around the silhouette (no light or coloured rim on the outer edge), 4-tone cel shading with ordered dithering, warm key light from the upper left, subtle cool fill light from the right kept inside the outline, full body, character facing right, side-on three-quarter view, feet on a flat ground line, flat pure green (#00FF00) background, no shadow on the ground, no text, no border, no anti-aliasing

Negative prompt (where supported):

> blurry, soft gradients, painterly, 3D render, photo, anti-aliased edges, glowing rim light on the silhouette, halo, background scenery, multiple characters, cropped feet, extra limbs, text, watermark, logo, frame, gore, blood

## Characters

### Dee Marlowe (`dee`), boxer, protagonist

> Dee Marlowe, a lean, athletic Black woman in her late twenties, banned Golden Gloves contender. Cropped dark curls, small silver stud earring, focused and calm expression. Teal sleeveless athletic top; charcoal track pants with a single chalk-white side stripe; white hand wraps and tape up to mid-forearm (no gloves); black high-top boxing boots. Compact boxer's stance, chin tucked, lead hand forward at shoulder height, rear fist by the cheek.

Palette ramps: `dee_skin`, `dee_hair`, `dee_top` (teal), `dee_pants`, `dee_stripe`, `dee_wraps`, `dee_boots`.

### Bram Hollis (`bram`), grappler, the Ring's gatekeeper

> Bram Hollis, a huge, heavy-set white man in his forties, laid-off shipyard rigger. Shaved head, full dark-brown beard, heavy brow, broken nose. Charcoal short-sleeved T-shirt; rust-orange work overalls with the bib folded down and the straps hanging at the hips; brown leather wrist straps; black steel-toe work boots. Wide, low stance, open hands forward like he is about to grab, weight on the back foot.

Palette ramps: `bram_skin`, `bram_beard`, `bram_shirt`, `bram_overall` (rust), `bram_straps`, `bram_boots`.

Costume rules from the [setting](../design/setting.md): work clothes, street clothes, wraps and tape. No gis, no headbands with caps, nothing that echoes Fatal Fury's cast.

## Frame prompts

Prefix each with the character prompt, append the style block. `(ref)` means: use the exported reference of that frame as the pose input.

| Frame | Prompt fragment |
| --- | --- |
| `idle_0…3` | fighting stance, breathing: shoulders and head 0 / 1 / 2 / 1 px lower, feet planted, head steady (ref) |
| `walk_0…5` | stepping forward in guard: contact (feet apart, body low) / passing (feet together, body high) / contact / passing; torso upright and centred, never leaning back (ref) |
| `crouch` | crouched low in guard, knees deep, back foot on toes (ref) |
| `guard` / `guard_crouch` | blocking: forearms up in front of the face, chin tucked, braced (ref) |
| `jump_rise` / `jump_apex` / `jump_fall` | jumping: knees tucked up / peak of the jump / legs reaching down to land (ref) |
| `lp_0…2` | quick jab: lead fist pulled back 2–3 px, shoulder dipped / lead arm fully extended at head height, shoulder forward / retract to guard (ref) |
| `hp_0…3` | heavy punch: wind-up, rear shoulder back / punching arm fully extended, torso turned in, rear heel up / fist carried past the centre line, follow-through / arm returning to guard (ref) |
| `lk_0…2` | front kick: knee chambered / leg extended at waist height, foot flexed / retract (ref) |
| `hk_0…3` | low leg sweep: dropping down / front leg sweeping along the ground at full length / sweep finishing behind / rising from the crouch (ref) |
| `throw_0` / `throw_1` | open hands reaching forward at chest height, weight forward / hands closed on the opponent's collar and upper arm, elbows bent, pulling in — clearly a different pose (ref) |
| `throw_hold_0` / `_1` | clinch: opponent pulled chest to chest, knees bent to lift, rear foot braced / tossing over the shoulder, body leaning back; hands where the opponent's body will be (ref) |
| `hit_0` / `hit_1` | snap: head thrown back, shoulders up, arms pulled in / stagger: weight on the rear foot, arms dropping, never reaching forward (ref) |
| `hit_crouch` | hit while crouching: flinch, head back, arms in, shoulders hunched (ref) |
| `held_0` / `held_1` | being grabbed, off-balance, arms flailing / lifted off the ground, body tilted (ref) |
| `down_0…3` | knocked down: falling backwards / lying on the back, head to the left / sitting up / getting up on one knee (ref) |
| `win_0` / `win_1` | victory pose, two clearly different frames (Dee: lead fist raised overhead / fist higher, head thrown back, shouting; Bram: double-biceps flex / chin up, shouting, fists higher) (ref) |
| Dee `special_0…3` | rushing uppercut: dip low / driving upwards and forwards / uppercut angled 20–30° forward, fist above head height in front of the face, rising onto the toes / landing (ref) |
| Bram `special_0`, `_1` | low lunge with both arms spread wide, a bear-hug reach / arms closed around the opponent's waist, clamped (ref) |
| Bram `lp_*` (Palm Shove) | both palms pushed out at full arm length at chest height, heels of the hands forward, shoulders forward (ref) |
| Bram `hp_*` (Anchor Hook) | lead arm bent 90° sweeping across in a hook, torso turning into it, rear heel lifting / fist past the centre line (ref) |
| Bram `hoist_0…2` | lifting an (invisible) opponent from the waist / pressing them overhead at full arm's length, thick arms locked / slamming them down in front (ref) |

## Portraits (32 × 32)

> Pixel-art portrait for a 1990s arcade fighting-game HUD, head and shoulders, facing right, 1-pixel dark outline, 4-tone shading, warm key light from the upper left, dark navy background with a dithered warm glow from above, [character prompt, face details only]

Generate at 512 × 512 and import with `import_art.py image face.png Assets/Fighters/Dee/dee_portrait.png --size 32 32`. Portraits almost always need a hand pass at 32 × 32: eyes, mouth, outline.

## Stages (640 × 360)

Shared stage block:

> 1990s arcade fighting-game background, pixel art at 640x360, wide side-on view of a fighting floor, the camera at chest height, a flattened ellipse of heavy welded chain lying on the floor in the centre as the ring, empty floor area in the lower third, spectators only as silhouettes behind a barrier in the middle distance, dark calm sky across the top fifth, limited palette, ordered dithering, no characters in the foreground, no text except in-world signs, no lens flare

| Stage | Prompt |
| --- | --- |
| `pier_nine` | Pier 9: a closed cargo pier at night; rust-red gantry crane legs framing both sides; a moored freighter with one floodlight; sodium lamps on posts; dockers and bookmakers with chalk boards behind a steel rail; wet planks; teal harbour water and a distant city skyline. |
| `lantern_row` | Lantern Row: a night market after curfew; shuttered stalls under strings of paper lanterns, half of them dark; an old station clock on a brick facade at the back; the ring roped off on worn tiles; traders and well-dressed strangers behind the ropes. |
| `the_crown` | The Crown: the tar-and-gravel roof terrace of a derelict grand hotel at first light; a coral and amber dawn sky behind a city skyline; a CROWN HOTEL bulb sign on a steel truss with dead bulbs; a wooden water tower on stilts; a brick parapet with a few spectators. |

Import with `import_art.py image pier.png Assets/Stages/PierNine/pier_nine.png --size 640 360`, then check the layout rules: ground at y 316, calm HUD bands at y 0–52 and 330–360, chain ring present. Generators rarely put the floor in the right place, so expect to compose or paint over parts.

## Layered stages

Spec: [asset-spec.md §3.1](asset-spec.md#31-layered-stages). Placeholder: `python3 Tools/Art/generate.py foundry`. Its six layers show the sizes, positions and transparency the engine expects; draw over them or replace them one at a time.

Generate each layer on its own, as a wide image at the layer's exact width, with the shared stage block plus:

> separate background layer for a side-scrolling fighting game, [what this layer holds], everything else fully transparent, no floor and no sky unless this layer is the floor or the sky, same light direction and palette as the other layers

| Foundry layer | Size | What it holds |
| --- | --- | --- |
| `foundry_wall` | 680 × 360, opaque | Back wall of a shut-down foundry hall at night: soot-dark brick, three or four tall arched windows with cold teal light and broken panes, steel girders between them, a painted "VOLL WORKS" sign with two letters gone dark. |
| `foundry_furnace` | 728 × 360 | Three or four brick furnaces with glowing coral-orange mouths at about y 215, a gantry beam at y 86–93, a steel ladle hanging from it with a still-glowing rim; transparent above and between them. |
| `foundry_steam` | 640 × 360, tiling | A few sparse dithered puffs of steam between y 110 and 230, tiling left to right; nothing else. |
| `foundry_crowd` | 768 × 84 per frame, 8 frames | Spectator silhouettes, heads and shoulders, warm rim light from behind, in front of them a steel rail at y 60–63 of the frame. Frames 1–4: standing, small bob. Frames 5–8: cheering, arms up, bigger bob. The same people in the same places in every frame. |
| `foundry_floor` | 800 × 98 (from y 262) | Riveted steel deck plates, low contrast, slag stains, the welded chain ring centred at x 400, y 64 of the layer (y 326 on screen), about 564 × 48. |
| `foundry_front` | 850 × 360 (at x −10) | Two dark riveted steel columns at the very ends; everything else transparent. |

The crowd frames are the hard part for generators, because people move between frames. Generate frame 1, then make the others by hand-editing it (raise arms, shift heads by 1–3 px). Review the result in the game with `-stage foundry -demo`; the camera reaches both ends within a match.

**Layered versions of the other venues** follow the same split: Pier 9's harbour and skyline at 0.25, crane legs and the freighter at 0.55, dockers at the rail at 0.8, the planks at 1.0, a crane leg in front at 1.25. They come with the next stage art pass. Until then, the Art Pack v1 images stay as flat stages.

## Front end: title logo and menu backdrop

Placeholders come from `python3 Tools/Art/generate.py ui`; sizes and layout rules are in [asset-spec.md §5](asset-spec.md#5-ui).

**Title logo (432 × 96, transparent).** Hand-pixel it, or generate a reference and redraw: generators blur thin outlines and invent letters.

> Pixel-art logo for a 1990s arcade fighting game, the words IRON RING in heavy block capitals with riveted brass plates for letters, a warm top-lit gradient from pale gold to dark amber, a two-pixel near-black outline and a hard drop shadow, a ring of heavy welded chain passing behind the letters as a flattened ellipse, flat transparent background, no other text, no gloss, no lens flare

Check: reads at 1× from across a room; the letter shapes match the in-game fonts' squared style; outline pixels are the palette's ink (`#07090f`); nothing outside 432 × 96.

**Menu backdrop (640 × 360, opaque).**

> 1990s arcade fighting-game menu background, pixel art at 640x360, a dark empty fight floor seen from the front at night, two cold floodlight beams falling from above the frame, a flattened ellipse of welded chain on the floor in the lower third, the silhouettes of a crowd's heads and shoulders along the bottom edge, a thin brass rule near the top and the bottom, mostly deep navy, limited palette, ordered dithering, very low contrast in the centre, no people in focus, no text

Check: the centre band (y 60–336) stays dark and even, because the select screen puts portraits, fighters and white text there; brass rules at about y 60 and y 336 line up with the layout.

## Effects, fonts, icon

These are small and exact. Draw them by hand in a pixel editor at 1× (or keep the procedural ones) rather than generating them; the sizes and frame counts are in [asset-spec.md §4–5](asset-spec.md#4-effects).
