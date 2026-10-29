# Iron Ring asset specification

The contract between art and the game. Anything that meets it can replace the interim procedural art (`Tools/Art/generate.py`, [ADR-0011](../adr/0011-procedural-interim-art.md)) without code changes. Where to make the art is up to you: by hand, with an image generator ([prompts](generation-prompts.md)), or a mix. `Tools/Art/import_art.py` turns raw images into game-ready files, and the AssetPacker step of every build validates the result.

Setting, tone and costume direction: [docs/design/setting.md](../design/setting.md). The concept kit and Fatal Fury 2 are references only: do not copy their characters, stages or logos.

## 1. Global rules

| Rule | Value |
| --- | --- |
| Logical canvas | **640 × 360** px. The game scales it by a whole number to the display (letterboxed), so 1 art pixel = 1 logical pixel. |
| Pixel art | Hard pixel edges. No anti-aliasing against transparency, no semi-transparent pixels in sprites (alpha is 0 or 255). |
| Format | PNG, 8-bit RGBA (sprites, effects, UI) or RGB (stages). sRGB. No colour profiles that change the values. |
| Palette | `Assets/Data/palette.gpl` (GIMP/Aseprite) and `palette.json`. Sprites should stay inside it; the importer snaps to it. |
| Outlines | Every sprite silhouette has a 1 px outline in the darkest tone of the material it bounds (not black, not the background). |
| Light | Warm key light from the upper left, in front of the picture plane; cool teal rim light from the right. Sprites are lit for facing right; the mirrored player-2 sprite keeps the same shading (arcade convention). Stages are night by default; The Crown is dawn. |
| Shading | 4 tones per material + its line colour (the ramps in `palette.json`). Gradients use ordered (Bayer) dithering, never smooth blends. |
| Gore | None. Impact is sold by poses, effects, hitstop and sound. |
| Text in art | None, except the HUD fonts and in-world signage on stages. |

### Palette structure

`palette.json` contains named colours (`night_*`, `steel_*`, `sodium_*`, `teal_*`, `coral_*`, neutrals) and **material ramps**: `[line, shadow, mid, light, highlight]`. Fighters use their own ramps (`dee_skin`, `dee_top`, `bram_overall`, …) plus the shared `metal` and `eye`. Player-side colour coding: **teal** for Dee, **rust/coral** for Bram. New fighters add their own ramps (5 colours each) to `Tools/Art/ironart/palette.py` and regenerate the palette files (`python3 Tools/Art/generate.py palette`).

## 2. Fighters

Each fighter lives in `Assets/Fighters/<Name>/`:

| File | Content |
| --- | --- |
| `<id>.fighter.json` | Gameplay data and animation timing ([schema](#25-fighter-definition-idfighterjson)). |
| `<id>_atlas.png` + `<id>_atlas.json` | All frames, packed, with pivots ([schema](#24-sprite-atlas-json)). |
| `<id>_portrait.png` | 32 × 32 HUD portrait: head and shoulders facing **right**, 1 px outline, dark night backdrop (`night_1`) with a dithered warm glow from above. |

### 2.1 Size and registration

- Draw every frame **facing right**. The game mirrors it for the other side.
- **Standing height** (feet to top of head, idle): 120–136 px. Dee is 130 px, Bram is 134 px. Proportions are realistic and slightly heroic (about 7.5 heads), not chibi.
- **Origin / pivot**: the point on the ground midway between the feet in the idle stance. It is the same body point for every frame, so the character never slides when frames change: in a lunge the pivot stays under the hips' starting position and the body moves forward of it.
- Nothing reaches more than **3 px below** the ground line (y of the pivot) except airborne frames, which are drawn above it anyway. The generator and importer both warn about this.
- The body should fill its hurtbox: Dee stands in `[-16, 0, 32, 124]`, Bram in `[-20, 0, 40, 128]` (x from the pivot, forwards; y up from the ground; w, h). Crouching: Dee `[-18, 0, 38, 88]`, Bram `[-22, 0, 44, 92]`.
- Attack frames must put the striking limb inside the move's hitbox during its **active** frames (§2.3). Turn on the training overlay (`-training`, F3) to check.

### 2.2 Frame list

Every fighter needs these frames (names are case-sensitive). Ticks are at 60 per second. Frames with "hold" timing are held as long as the state lasts.

| Frame(s) | Pose | Notes |
| --- | --- | --- |
| `idle_0`…`idle_3` | Fighting stance, breathing bob (0, −1, −2, −1 px) | Loops, about 9–10 ticks each. |
| `walk_0`…`walk_5` | Walk cycle, guard up | Loops, 6–7 ticks each; also played backwards for walking back. |
| `crouch` | Crouching stance | Hold. Head at about 88 px. |
| `guard` | Standing block: forearms up, chin tucked | Hold. |
| `guard_crouch` | Crouching block | Hold. |
| `jump_rise`, `jump_apex`, `jump_fall` | Tuck up, peak, legs reaching down | Drawn relative to the ground pivot as if standing; the game lifts them. |
| `lp_0`…`lp_2` | Light punch: chamber, extended, retract | |
| `hp_0`…`hp_3` | Heavy punch: wind-up, impact, follow-through, recover | |
| `lk_0`…`lk_2` | Light kick: chamber, extended, retract | |
| `hk_0`…`hk_3` | Heavy kick (low sweep for both starters): drop, sweep out, sweep through, recover | Low: the foot sweeps along the ground. |
| `throw_0`, `throw_1` | Throw attempt: reach, grab | `throw_0` doubles as the whiff recovery. |
| `throw_hold_0`, `throw_hold_1` | Holding the opponent: lift, toss | Each carries a **partner** offset (§2.4). |
| `special_*` | Special move frames | Per fighter, see below. |
| `hit_0`, `hit_1` | Hit reaction: snap back, recoil | |
| `hit_crouch` | Crouching hit reaction | |
| `held_0`, `held_1` | Being thrown: grabbed, lifted/airborne | Drawn by the **opponent's** throw (partner frames). Every fighter needs them. |
| `down_0`…`down_3` | Knockdown: falling backwards, lying on the back (head towards −x), sitting up, getting up | `down_1` is held for 30 ticks and is also the KO pose. |
| `win_0`, `win_1` | Victory pose, two-frame loop | |

Special frames for the starters:

- **Dee, Rush Uppercut (236P):** `special_0` dip, `special_1` rising, `special_2` full uppercut (fist high above the head; the hitbox is `[10, 60, 40, 90]`), `special_3` landing recovery.
- **Bram, Dock Hoist (63214P, command grab):** `special_0` reach, `special_1` grab; hold frames `hoist_0` (grab low), `hoist_1` (opponent hoisted overhead), `hoist_2` (slam). `hoist_*` carry partner offsets.

### 2.3 Timing contract

Animation timings are in `<id>.fighter.json` and must add up to the move's frame data. The build fails if they don't (`Source/Content/Validation.cpp`):

| Animation | Must last | Example (Dee) |
| --- | --- | --- |
| A move's `animation` | `startup + active + recovery` ticks | `light_punch` = 4 + 3 + 10 = 17 |
| A throw's `holdAnimation` | `throwDuration` ticks | `throw_hold` = 22 + 18 = 40 |
| `knockdown` | `body.knockdownTicks` (54) | 8 + 30 + 10 + 6 |

Convention: the first animation frame lasts for the startup, the impact frame for the active window, and the remaining frames split the recovery. Artists draw poses; designers set the ticks. Changing ticks in data never requires new art.

### 2.4 Sprite atlas JSON

```json
{
  "image": "dee_atlas.png",
  "frames": [
    {"name": "idle_0", "source_rect": [601, 2, 51, 130], "pivot_pixels": [21, 130]},
    {"name": "throw_hold_1", "source_rect": [429, 2, 59, 131], "pivot_pixels": [32, 130], "partner": [34, 34]}
  ]
}
```

- `source_rect`: `[x, y, w, h]` of the trimmed frame in the atlas image.
- `pivot_pixels`: the origin relative to the top-left of `source_rect`.
- `partner` (throw holds only): where the **opponent's** origin goes, `[dx, dy]` from this fighter's origin, dx forwards and **dy up**. The game draws the opponent's `held_*` frame there (which `held_*` is set per animation frame in the fighter definition). The opponent faces the thrower.
- Any extra keys (`$comment`) are ignored. Frame order doesn't matter; atlas width at most 1024 px is a good habit, not a rule.

### 2.5 Fighter definition (`<id>.fighter.json`)

Units: lengths in logical pixels, speeds in 1/256 px per tick, times in ticks. Boxes are `[x, y, w, h]` from the origin, x forwards, y up.

```json
{
  "id": "dee", "name": "Dee Marlowe", "displayName": "DEE MARLOWE",
  "atlas": "dee_atlas.json", "portrait": "dee_portrait.png",
  "physics": {"walkForward": 420, "walkBack": 340, "jumpVelocity": 1900, "gravity": 70},
  "body": {"standHurt": [-16, 0, 32, 124], "crouchHurt": [-18, 0, 38, 88], "pushWidth": 36, "knockdownTicks": 54},
  "moves": {
    "lightPunch": {"name": "Stiff Jab", "animation": "light_punch", "startup": 4, "active": 3, "recovery": 10,
                   "hitbox": [36, 84, 22, 16], "damage": 6, "hitstun": 14, "knockback": 5},
    "throw": {"name": "Clinch Toss", "kind": "throw", "animation": "throw", "holdAnimation": "throw_hold",
              "startup": 3, "active": 2, "recovery": 22, "hitbox": [8, 40, 24, 60],
              "damage": 10, "throwDuration": 40, "throwDistance": 50, "techable": true},
    "special": {"name": "Rush Uppercut", "input": "236P", "animation": "special", "...": "..."}
  },
  "animations": {
    "idle": {"loop": true, "frames": [["idle_0", 9], ["idle_1", 9], ["idle_2", 9], ["idle_3", 9]]},
    "throw_hold": {"frames": [["throw_hold_0", 22, "held_0"], ["throw_hold_1", 18, "held_1"]]}
  }
}
```

- `moves` keys: `lightPunch`, `heavyPunch`, `lightKick`, `heavyKick`, `throw`, `special` (all optional, but the starters define all six).
- Move fields: `name`, `animation`, `startup`, `active`, `recovery`, `hitbox`, `damage`, `hitstun`, `blockstun` (7), `knockback`, `blockPush` (3), `hitstopOnHit` (5), `hitstopOnBlock` (3), `low`, `heavy`, `knockdown`, `advance` (forward speed during the move); throws add `kind: "throw"`, `holdAnimation`, `throwDuration` (40), `throwDistance` (50), `techable`; specials add `input` in numpad notation (`236P`, `63214P`; P/K = either punch/kick). Values in brackets are defaults.
- `animations` must contain: `idle`, `walk`, `crouch`, `guard`, `guard_crouch`, `jump_rise`, `jump_fall`, `hit`, `hit_crouch`, `held`, `knockdown`, `ko`, `win`, plus every animation a move names. A frame entry is `[frame, ticks]` or, in throw holds, `[frame, ticks, opponentFrame]`.
- No desperation / super moves: the Heat meter only pays for the Breakaway (guard + HP + HK, 50 Heat), which uses no extra frames.

Full rationale: [ADR-0010](../adr/0010-fighter-content-format.md).

## 3. Stages

`Assets/Stages/<Name>/<id>.png`, listed in `Assets/Data/stages.json` (`id`, `name`, `image`).

- **Exactly 640 × 360** px, RGB, one flattened layer for now (parallax layers are planned for M3).
- **Ground line y = 316**: fighters' feet stand here. The fight floor is the band roughly y 296–340.
- **Playable width x 30–610**: fighters never go outside it; keep the edges for framing (crane legs, pillars, rope posts).
- **HUD bands**: top **y 0–52** (health bars, portraits, timer) and bottom **y 330–360** (Heat meters, status text) must be calm and dark-ish: no bright lights, no busy detail, no text.
- **The chain ring**: a flattened ellipse of welded chain on the floor, centred at x 320, y 326, about 564 × 48 px, slightly lit by the key light. It is the series' visual signature and must appear on every stage.
- The floor is **lower-contrast** than the fighters (value range in the bottom 40% of the palette); fighters must read clearly on it at a glance.
- Spectators stand **behind** a barrier (rail, rope, crates) whose top is below y ≈ 250, so they never overlap the fighters' heads. Silhouettes and rim light, few details, no faces.
- Night palette (`night_*`, `steel_*`) with sodium practical lights; The Crown uses the dawn variant (warm sky, long shadows).

The three venues: **Pier 9** (closed cargo pier, crane legs, freighter floodlights, dockers behind the rail), **Lantern Row** (night market after curfew, shutters, half-lit lanterns, old station clock), **The Crown** (derelict hotel roof terrace at dawn, water tower, a CROWN HOTEL bulb sign with dead bulbs, below the HUD band). Details in [setting.md](../design/setting.md#venues).

## 4. Effects

`Assets/Resources/FX/effects_atlas.png/.json` (atlas format as §2.4). The pivot is the **impact point** (the frame's centre, except `slam`: bottom centre on the ground). Each frame shows for 3 ticks. Colours come from the palette's bright end; the game brightens them on HDR displays, so keep them saturated, not white.

| Effect | Frames | Size | Look |
| --- | --- | --- | --- |
| `light_hit` | `light_hit_0`…`_3` | 28 × 28 | Small chalk-and-sodium star burst, shrinking. |
| `heavy_hit` | `heavy_hit_0`…`_4` | 48 × 48 | Larger coral-and-sodium burst with radial streaks and sparks. |
| `guard` | `guard_0`…`_3` | 28 × 28 | Teal crescent flash (blocked hit). |
| `breakaway` | `breakaway_0`…`_5` | 80 × 80 | Expanding, dissolving teal shock ring for the Breakaway shove. |
| `tech` | `tech_0`…`_3` | 28 × 28 | A chalk-and-teal X flash (throw broken). |
| `slam` | `slam_0`…`_4` | 80 × 32 | Floor dust and grit cloud, pivot on the ground. |

## 5. UI

| Asset | File | Spec |
| --- | --- | --- |
| Small font | `Resources/UI/font_small.png/.json` | Monospaced 5 × 7 glyphs in 6 × 8 cells, 16 per row, white on transparent (tinted at runtime). |
| Large font | `Resources/UI/font_large.png/.json` | 17 × 23 glyphs in 18 × 24 cells, same layout; brass gradient fill, dark outline, drawn untinted. Banners and the timer. |
| Font charset | both | `␠ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?:-+/'"%()#=<>*` in that order. Uppercase only. |
| HUD layout | `Resources/UI/hud_layout.json` | Rects/points on the 640 × 360 canvas; edit to move elements, no art change. |
| Icon | `Resources/UI/icon.png` | 256 × 256 RGBA: a chain ring around the letters IR, designed on a 32 × 32 grid so it reads at 16 × 16. Also the Windows `.ico` (`Source/Launcher/Resources/IronRing.ico`, 16–256 px). |
| Portraits | per fighter | 32 × 32, see §2. |

## 6. Audio

`Assets/Resources/Audio/*.wav`, wired in `ProjectSettings/GameSettings.json → sounds`.

- WAV, PCM 16-bit, **48 kHz, mono**. Short (under 1 s), trimmed with no leading silence (they fire on the impact tick), peaks around −3 dBFS.
- Required: `light_hit`, `heavy_hit`, `guard`, `breakaway`, `round_bell`, `menu_confirm`. Throw slams use `heavy_hit`; throw techs use `guard` (separate `throw`, `tech`, `ko` sounds are planned).
- Texture: leather and tape on skin, chain rattle for the Breakaway, a cracked boxing bell for rounds. No voices yet.

## 7. Naming and layout

- Folders `PascalCase` (`Fighters/Dee`, `Stages/PierNine`), files `snake_case` (`dee_atlas.png`, `pier_nine.png`), ids lowercase (`dee`, `pier_nine`).
- Everything under `Assets/` is packed into `IronRing_Data/*.assets` by AssetPacker, following `ProjectSettings/AssetPipeline.json`. Virtual paths are relative to `Assets/`.

## 8. Producing art elsewhere

1. Generate or draw **one image per frame**, all at the same size and character scale, character facing right, on a **flat key colour** that isn't in the palette (pure green `#00FF00` works) or on transparency. Keep the feet on the same baseline and the body in the same spot across frames: put a faint guide grid or a reference frame next to the generator input if it supports image prompts. Prompts: [generation-prompts.md](generation-prompts.md).
2. Name the files after the frames (`idle_0.png`, `hk_2.png`, …) in one folder per fighter.
3. Find the **origin** in source pixels: the point on the ground between the feet in `idle_0`.
4. Import:

   ```sh
   pip install -r Tools/Art/requirements.txt
   python3 Tools/Art/import_art.py fighter dee path/to/dee_frames --origin 512 940 --height 130
   ```

   It keys out the background (including pockets enclosed by limbs), scales every frame by the same factor (from `--height`, measured on `idle_0`, or `--scale`), hardens the alpha, snaps colours to the fighter's ramps, forces fringe pixels on the silhouette edge to outline colours, trims, packs and writes the atlas. It lists missing frames and frames that reach below the floor.
   - Frames whose origin differs (a lunge drawn off-centre) and throw partner offsets go in a manifest: `--manifest dee_manifest.json` with `{"throw_hold_1": {"origin": [530, 940], "partner": [34, 34]}}`. Partner offsets are in game pixels.
   - `--no-quantize` keeps the original colours (for art already made in the palette).
5. Stages and portraits: `python3 Tools/Art/import_art.py image source.png Assets/Stages/PierNine/pier_nine.png --size 640 360` (scale to cover, centre-crop, snap to the palette).
6. Build. AssetPacker validates everything; then run `IronRing -training` and press F3 to check hitboxes against the art.

## 9. Acceptance checklist

- [ ] All frames from §2.2 present, names exact; the build reports no content errors.
- [ ] Idle height within 120–136 px; the character doesn't change size or slide between frames.
- [ ] No frame reaches more than 3 px below the ground (importer/generator warning is clean).
- [ ] Attack limbs inside the hitbox on active frames; body roughly fills the hurtboxes (F3 overlay).
- [ ] Throw holds: opponent's `held_*` frame lines up with the thrower's hands (play a throw in training mode).
- [ ] Hard edges, 1 px coloured outline, palette colours only, no semi-transparent pixels.
- [ ] Facing right; reads clearly in silhouette at 1× scale against all three stages.
- [ ] Stages are 640 × 360, ground at y 316, calm HUD bands, chain ring present, crowd behind the barrier.
- [ ] Effects and fonts follow §4–5; sounds are 48 kHz mono 16-bit WAV.
- [ ] Nothing copied from Fatal Fury 2, the concept kit or other existing games; no logos or real brands.
