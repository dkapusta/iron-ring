# Generation prompts

Prompts for producing Iron Ring art with an outside image generator. Every result goes through `Tools/Art/import_art.py` and has to meet the [asset specification](asset-spec.md). Image generators are inconsistent across frames, so the workflow anchors each frame to a pose reference instead of trusting text alone.

## Workflow that keeps frames consistent

1. Export the current frames as pose references:

   ```sh
   python3 Tools/Art/import_art.py refs dee build/refs/dee      # 1024x1024, green key, origin (512, 928)
   ```

2. Make one **character sheet** first (front, side, back, plus a close-up of the face) from the character prompt below. Pick the best result; it is the identity reference for every frame.
3. For each frame, generate with the frame's reference as the **pose/structure input** (img2img at about 0.45–0.6 strength, or a pose or line-art control) and the character sheet as the **identity/style input**. Keep the canvas 1024 × 1024 and don't let the generator move or rescale the figure.
4. Clean up by hand where needed: hands, feet on the baseline, the outline. Then import, using the command `refs` printed:

   ```sh
   python3 Tools/Art/import_art.py fighter dee path/to/dee_frames --origin 512 928 --scale 0.25 --manifest build/refs/dee/manifest.json
   ```

5. Build, run `IronRing -training`, press F3 and check hitboxes. Fix the art or the boxes in `dee.fighter.json`.

If the generator can't follow a pose reference, generate larger poses freely, align them yourself on a shared canvas (feet on one baseline, same scale), and pass `--origin`/`--height` instead.

## Shared style block

Append this to every sprite prompt:

> 16-bit arcade fighting game sprite, early-1990s style, hand-placed pixel art at low resolution, limited palette, 1-pixel dark coloured outline around the silhouette, 4-tone cel shading with ordered dithering, warm key light from the upper left, cool teal rim light from the right, full body, character facing right, side-on three-quarter view, feet on a flat ground line, flat pure green (#00FF00) background, no shadow on the ground, no text, no border, no anti-aliasing

Negative prompt (where supported):

> blurry, soft gradients, painterly, 3D render, photo, anti-aliased edges, background scenery, multiple characters, cropped feet, extra limbs, text, watermark, logo, frame, gore, blood

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
| `idle_0…3` | fighting stance, breathing, slight bob (ref) |
| `walk_0…5` | stepping forward in guard, one phase of a walk cycle (ref) |
| `crouch` | crouched low in guard, knees deep, back foot on toes (ref) |
| `guard` / `guard_crouch` | blocking: forearms up in front of the face, chin tucked, braced (ref) |
| `jump_rise` / `jump_apex` / `jump_fall` | jumping: knees tucked up / peak of the jump / legs reaching down to land (ref) |
| `lp_0…2` | quick jab: chamber / lead arm fully extended at head height / retract (ref) |
| `hp_0…3` | heavy straight punch: wind-up / rear arm fully extended, body turned in / over-extended follow-through / recovering (ref) |
| `lk_0…2` | front kick: knee chambered / leg extended at waist height / retract (ref) |
| `hk_0…3` | low leg sweep: dropping down / front leg sweeping along the ground / sweep finishing / rising from the crouch (ref) |
| `throw_0` / `throw_1` | reaching both hands forward to grab / gripping the opponent's collar and arm (ref) |
| `throw_hold_0` / `_1` | holding an (invisible) opponent: hoisting / tossing over the shoulder; hands where the opponent's body will be (ref) |
| `hit_0` / `hit_1` | hit reaction: head snapped back, body recoiling / staggering back (ref) |
| `hit_crouch` | hit while crouching, flinching (ref) |
| `held_0` / `held_1` | being grabbed, off-balance, arms flailing / lifted off the ground, body tilted (ref) |
| `down_0…3` | knocked down: falling backwards / lying on the back, head to the left / sitting up / getting up on one knee (ref) |
| `win_0` / `win_1` | victory pose, two frames of a small loop (Dee: lead fist raised overhead, then a shout; Bram: double-biceps flex, then a shout) (ref) |
| Dee `special_0…3` | rushing uppercut: dip low / driving upwards / full uppercut with the fist high above the head, rising onto the toes / landing (ref) |
| Bram `special_0`, `_1` | lunging forward with both arms wide / clamping both hands shut (ref) |
| Bram `hoist_0…2` | lifting an (invisible) opponent from the waist / pressing them overhead at full arm's length / slamming them down in front (ref) |

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

## Effects, fonts, icon

These are small and exact. Draw them by hand in a pixel editor at 1× (or keep the procedural ones) rather than generating them; the sizes and frame counts are in [asset-spec.md §4–5](asset-spec.md#4-effects).
