# Iron Ring: production art brief for the M3 roster and venues (Art Pack v3)

**For:** whoever produces the artwork (an artist, an image-generation pipeline, or both).
**From:** the Iron Ring project, 2026-10-31.
**With:** the latest Iron Ring source package. Every path below is relative to its root. The game, the import tools and the validation tools are all in it.

M3 added five fighters and three venues. All the code for them is done and the arcade plays end to end, but their art is **procedural placeholder art** made by `Tools/Art/generate.py`. This pack replaces exactly that placeholder art with production art at the quality of Art Packs v2 and v2.1. Everything else in the game is already final and stays as it is.

This brief is self-contained: the numbers you need are written here. The repository documents are cited for detail and are the authority if anything disagrees:

- [`docs/art/asset-spec.md`](asset-spec.md): the technical contract (§2 fighters, §2.6 animation rules, §3.1 layered stages);
- [`docs/art/vertical-slice-art-brief.md`](vertical-slice-art-brief.md): the v2 brief. Its rules, champion-pose brief and prompts apply here unchanged;
- [`docs/art/art-pack-v2.1-brief.md`](art-pack-v2.1-brief.md): the crowd rules (§3.3), which apply to the new crowds;
- [`docs/design/setting.md`](../design/setting.md): world, cast and tone.

---

## 1. What to deliver

| # | Deliverable | Size and format | Priority |
| --- | --- | --- | --- |
| F1 | **Sable Reyes**: the full frame set (§3.2, §4.1) | 55 frames, 256 × 256 RGBA | Required |
| F2 | **Idris Okafor**: the full frame set (§4.2) | 55 frames | Required |
| F3 | **Greta Voll**: the full frame set (§4.3) | 55 frames | Required |
| F4 | **Jun Arata**: the full frame set (§4.4) | 55 frames | Required |
| F5 | **The Broker** (boss): the full frame set, without champion frames (§4.5) | 49 frames | Required |
| P1 | HUD portraits, one per fighter above (§3.5) | 32 × 32 RGBA | Required |
| P2 | Select portraits for Sable, Idris, Greta and Jun (§3.5) | 48 × 48 RGBA | Recommended |
| S1 | **Dry Dock 4**: final layered art (§5.2) | 5 layers + preview | Required |
| S2 | **The Hold**: final layered art (§5.3) | 5 layers + preview | Required |
| S3 | **The Broker's Club**: final layered art (§5.4) | 5 layers + preview | Required |

That is 269 fighter frames, 9 portraits and 15 stage layers.

**Not wanted** (keep the current ones exactly):

- **Dee and Bram:** every frame, portrait and ending picture.
- **The four M2 venues:** Pier 9, Lantern Row, The Foundry, The Crown. All their layers and previews stay as they are.
- **The front end:** the title logo and the menu backdrop.
- **Procedural assets that are final:** hit sparks and other effects, fonts, the HUD, the application icon and the palette files.
- **Ending pictures** (640 × 360) for any fighter. The arcade ending is the animated champion pose over The Crown ([ADR-0020](../adr/0020-adopting-art-pack-v2.md)).
- **Data files:** fighter frame data, hitboxes, `stages.json` and `roster.json`. The art fits the existing numbers. If a pose truly cannot fit its box, say so in the README rather than changing the box.
- **Music and sound.**

If you think something outside this list needs work, add a note to the delivery instead.

**Two drops are fine.** If the work is split, deliver:

1. **Drop A:** the four playable fighters (F1–F4) with their portraits.
2. **Drop B:** the Broker (F5) and the three venues (S1–S3).

Each drop must pass §7 on its own.

---

## 2. Rules for everything

These are the v2 rules, unchanged.

| Rule | Value |
| --- | --- |
| Screen | The game draws on a **640 × 360** logical canvas, scaled by whole numbers. 1 art pixel = 1 screen pixel. |
| Pixel art | Hard pixel edges. No anti-aliasing against transparency. **Alpha is 0 or 255** only. |
| Files | PNG, 8-bit, sRGB, no colour profile that changes values. Sprites and stage layers RGBA. Opaque back layers and previews RGB (or RGBA, fully opaque). |
| Palette | Stages: the master palette (`Assets/Data/palette.gpl`, Appendix B of the v2 brief). Each fighter: one limited palette of **at most 48 colours**, fitted to the fighter. The colour keys are in §4 and `Tools/Art/ironart/palette.py`. |
| Outline | Every sprite silhouette has a 1 px outline in the **darkest tone of the material it bounds**. Not pure black, and never a light or cyan rim. |
| Light | Warm key light from the upper left, a cool teal fill from the right kept *inside* the outline. All three new venues are night scenes. |
| Shading | About 4 tones per material plus its line colour. Gradients use **ordered (Bayer) dithering**, never smooth blends. |
| Tone | Gritty, after-hours, early-90s arcade. **No blood or gore.** No text except in-world signs. |
| Originality | Nothing from Fatal Fury, Street Fighter, the "Brass Current" concept kit or any other game. That means no characters, stages, logos or recognisable poses. No real brands or real people. |
| Match the finished art | The new fighters stand next to Dee and Bram on the select screen and in fights, and the new venues follow Pier 9, Lantern Row, The Foundry and The Crown. Match their line weight, shading density, dithering and level of detail. `ArtSource/Fighters/Dee/frames/` and `Assets/Stages/Foundry/` are the reference. |

---

## 3. Fighters: shared rules

### 3.1 Canvas, registration, scale

- **Files:** one PNG per frame, named exactly as the frame (`idle_0.png`, `special_2.png`).
- **Canvas:** **256 × 256 RGBA**, character **facing right**. The game mirrors it for player 2.
- **Origin at (128, 232):** the point on the ground midway between the feet in the idle stance. It is the same body point in every frame, so nothing slides when frames change.
- **Scale:** one per fighter, for every frame. Never fit a frame to a box.
- **Ground:** feet on the ground line (y 232), with nothing more than 3 px below it.
- **Standing height** (feet to the top of the head, in `idle_0`) and **hurtboxes**: keep the heights of the current placeholders, because the frame data was tuned to them. Boxes are `[x, y, w, h]`: x from the origin forwards, y up from the ground. The body should fill its hurtbox.

  | Fighter | Standing height | Standing hurtbox | Crouching hurtbox |
  | --- | --- | --- | --- |
  | Sable | 132 px | `[-15, 0, 30, 124]` | `[-17, 0, 36, 90]` |
  | Idris | 139 px | `[-17, 0, 34, 132]` | `[-19, 0, 40, 92]` |
  | Greta | 121 px | `[-20, 0, 40, 116]` | `[-22, 0, 44, 86]` |
  | Jun | 118 px | `[-14, 0, 28, 112]` | `[-16, 0, 34, 82]` |
  | The Broker | 140 px | `[-17, 0, 34, 132]` | `[-19, 0, 40, 92]` |

  For comparison, Dee is 130 px and Bram 134 px. Greta is short and broad; Jun is the smallest in the cast; Idris and the Broker are the tallest.
- **Pose references.** The placeholder frames are the pose references. Every limb position is where the frame data expects it. Export them at 4× (1024 × 1024, origin 512, 928):

  ```sh
  python3 Tools/Art/import_art.py refs sable  refs/sable
  python3 Tools/Art/import_art.py refs idris  refs/idris
  python3 Tools/Art/import_art.py refs greta  refs/greta
  python3 Tools/Art/import_art.py refs jun    refs/jun
  python3 Tools/Art/import_art.py refs broker refs/broker
  ```

  This also writes each fighter's `manifest.json` with the throw partner offsets (§3.4). The placeholders are stick-built mannequins. Take the **pose, reach and timing** from them, not the anatomy, the costume or the shading.

### 3.2 The frame list

Every playable fighter needs these **55 frames**. The Broker needs the same set **without `champion_0`…`champion_5` (49 frames)**: he is the boss and is never the champion. Ignore his champion references if `refs` exports them.

| Frames | Pose |
| --- | --- |
| `idle_0`…`idle_3` | Fighting stance with a breathing bob: head and shoulders 0, 1, 2, 1 px lower. Feet still; head within ±1 px horizontally. |
| `walk_0`…`walk_5` | Walk cycle with the guard up. The torso stays within ±2 px of the centre line. `walk_0`/`walk_3` are contact poses (1–2 px lower); `walk_1`/`walk_4` are passing poses (1 px higher). The planted foot stays planted. Also played backwards for walking back. |
| `crouch`, `guard`, `guard_crouch` | Crouching stance, standing block (forearms up, chin tucked) and crouching block. Held. |
| `jump_rise`, `jump_apex`, `jump_fall` | Tuck up, peak, legs reaching down. Drawn relative to the ground origin as if standing; the game lifts them. |
| `lp_0`…`lp_2` | Light punch: chamber, **extended (active)**, retract. |
| `hp_0`…`hp_3` | Heavy punch: wind-up, **impact (active)**, follow-through, recover. |
| `lk_0`…`lk_2` | Light kick: chamber, **extended (active)**, retract. |
| `hk_0`…`hk_3` | Heavy kick: wind-up, **impact (active)**, follow-through, recover. It is *not* always a sweep: see each fighter in §4. |
| `throw_0`, `throw_1` | Throw: reach, grab. `throw_0` doubles as the whiff recovery. |
| `throw_hold_0`, `throw_hold_1` | Holding the opponent: lift, toss. These carry partner offsets (§3.4). |
| `special_0`…`special_3` | The special move. **`special_2` is the active frame** for all five; see §4. |
| `hit_0`, `hit_1`, `hit_crouch` | Hit reaction: `hit_0` snaps back (head back, shoulders up), `hit_1` recoils (weight on the rear foot); `hit_crouch` is the crouching flinch. |
| `held_0`, `held_1` | Being thrown: grabbed, then lifted and airborne. Drawn over the *opponent's* throw. |
| `down_0`…`down_3` | Knockdown: falling backwards, lying on the back (head towards −x; also the K.O. pose), sitting up, getting up. |
| `win_0`, `win_1` | Victory loop of two frames that clearly differ. Each fighter has their own (§4). |
| `champion_0`…`champion_5` | Arcade ending, holding up a length of the ring's chain (§3.3). Playable fighters only. |

**Readability is the first job.** Draw every active frame at the *extreme* of the move: the arm fully extended, the knee driven up, the heel at head height. The striking limb must be inside that move's hitbox (listed per fighter in §4) on its active frame. Players read moves from silhouettes, and the v1 review's worst findings were active frames that looked like idle.

### 3.3 The champion pose (playable fighters)

The same brief as Dee's and Bram's ([v2 brief §3.3](vertical-slice-art-brief.md#33-new-the-champion-pose-f3)). The winner stands on the roof of The Crown at dawn, alone, holding up **a length of the Iron Ring's own welded chain**.

- **The six frames:** `champion_0`–`champion_2` play once (20, 16 and 14 ticks), then `champion_3`–`champion_5` loop (18 ticks each).
- **The chain:**
  - 16–20 links, about 60–70 px long, alternating flat links (an open oval about 7 × 4 px) and edge-on links (a bar about 5 × 2 px);
  - coloured with the shared `metal` ramp (`#0b0d12`, `#3d4a66`, `#5a6782`, `#8a93a8`, `#d5dbe6`), counted within the 48 colours;
  - a 1 px outline, like the rest of the sprite.
- **The poses:**
  - `champion_0`: turned slightly to the viewer, catching breath.
  - `champion_1`: lifting the chain.
  - `champion_2`: the chain held high and taut, shouting.
  - `champion_3`, `champion_4`, `champion_5`: the hold, a 1 px breath with the chain sagging 1–2 px, then a shout.
  - Each fighter's version is in §4.
- **The loop** (3 → 4 → 5 → 3) must not pop: feet planted, torso within ±2 px.
- Never mirrored. Room above: raised arms and the chain can reach 190 px above the ground.

### 3.4 Throw partner points

`throw_hold_0` and `throw_hold_1` each have a **partner** offset in `manifest.json`: where the opponent's origin goes, `[dx, dy]` from this fighter's origin (dx forwards, dy up). Today all five fighters use `[30, 0]` and `[34, 34]`.

If your hands end up somewhere else, update the offsets so the opponent's `held_0` and `held_1` land in them. Check by throwing in training (§7).

### 3.5 Portraits (P1, P2)

- **HUD portrait** `<id>_portrait.png`:
  - 32 × 32 RGBA, head and shoulders facing right, the fighter's likeness;
  - a 1 px outline;
  - it sits in the HUD's top corners next to the health bar.
  - The five placeholders are flat and generic, and must be replaced.
- **Select portrait** `<id>_select.png`:
  - 48 × 48 RGBA, shown 1:1 in the character-select grid (without one, the 32 px portrait is scaled up 1.5× and looks soft);
  - the same framing and likeness as the HUD portrait, drawn at the higher resolution;
  - a dark `night_1` (`#141a30`) backdrop with a dithered warm glow from above;
  - the fighter's side colour as a band at the bottom edge: violet (Sable), muted blue (Idris), brass (Greta), jade (Jun).
  - The Broker is not on the select screen and needs no select portrait.

### 3.6 Workflow

1. Run `import_art.py refs <id> refs/<id>` to get a pose reference for every frame (§3.1).
2. Make one character sheet per fighter first (front, side, back, colour key) and get it right. It is the identity input for every frame.
3. Generate or draw **one image per frame**, with that frame's reference as the pose input and the character sheet as the identity input. Prompt for the extreme of the pose. Prompts are in the appendix.
4. Clean up by hand: hands, feet on the ground line, the outline, the colour count.
5. Bring the frames to the 256 × 256 canvas, either:
   - deliver 1024 × 1024 frames on the reference canvas (the project imports them with `--origin 512 928 --scale 0.25`); or
   - deliver 256 × 256 frames at origin (128, 232) (imported with `--origin 128 232 --scale 1 --no-quantize`).
6. Check them (§7).

---

## 4. The fighters

The cast descriptions are from [`setting.md`](../design/setting.md); the colour keys are the placeholders' ramps in `Tools/Art/ironart/palette.py` (`[line, shadow, mid, light, highlight]`). Keep the hues, and refine the tones as the art needs.

Each move table lists what the **active frame** must show and the **hitbox** it must reach: `[x, y, w, h]` from the origin, x forwards and y up. Move names are shown in the game's move list, so the pose should read as the name.

### 4.1 Sable Reyes (F1): mobile kickboxer, violet

**Look.** Lean and long-legged, early twenties. Black hair in a **high ponytail that reads in every silhouette**. A violet sleeveless top, black kickboxing shorts, white shin wraps, worn grey trainers and fingerless gloves.

**Stance.** Tall and bladed, hands high, always moving. She is a night courier who runs packages over Grayhaven's rooftops and fights strictly for the purse.

**Colour key:**

| Material | Mid / light |
| --- | --- |
| Top | `#4a2470` / `#6e3aa0` |
| Shorts | `#1a1724` / `#292538` |
| Wraps | `#c2b9a0` / `#e2dbc6` |
| Skin | `#7a4b30` / `#a06a45` |

| Move | Frames (active) | Active pose | Hitbox |
| --- | --- | --- | --- |
| LP Snap Jab | `lp_1` | Lead jab at full reach, head height | `[34, 84, 22, 16]` |
| HP Rear Cross | `hp_1` | Rear straight, shoulder turned through, rear heel up | `[38, 88, 28, 18]` |
| LK Calf Kick | `lk_1` | A low round kick to the calf, shin parallel to the floor | `[40, 12, 36, 18]` |
| HK Wire Kick | `hk_0`–`hk_3` (`hk_1`) | A **high roundhouse**: knee chamber, the foot snapping out at head height, re-chamber, set down | `[42, 82, 38, 22]` |
| Throw Rooftop Drop | `throw_*` | Grab by the collar, swing the opponent over the hip | n/a |
| Special Switch Step (236 + K) | `special_0`–`special_3` (`special_2`) | Stance switch (`_0`), the step-in (`_1`), a **flying knee** driven up and forward (`_2`), and the landing (`_3`) | `[18, 68, 36, 44]` |

- **Win:** counting the purse, one fist up and the other hand on her hip; then a point at the crowd.
- **Champion:** both fists overhead, the chain stretched between them.

### 4.2 Idris Okafor (F2): long-range striker, muted blue

**Look.** Tall and rangy, forties. Close-cropped hair going grey and a short beard. A faded blue work shirt with the sleeves rolled to the elbow, canvas trousers rolled at the shin, a length of rope for a belt (tied in a bowline), and deck shoes.

**Stance.** Upright and side-on, open hands. He is a merchant sailor stranded when his ship was impounded, fighting to buy passage out. Long, deliberate kicks; nothing hurried.

**Colour key:**

| Material | Mid / light |
| --- | --- |
| Shirt | `#2c4260` / `#43607e` |
| Trousers | `#48453a` / `#666251` |
| Rope | `#937546` / `#b89a62` |
| Skin | `#4a2c1b` / `#66412a` |

| Move | Frames (active) | Active pose | Hitbox |
| --- | --- | --- | --- |
| LP Long Jab | `lp_1` | An open-hand jab at long reach | `[36, 88, 26, 14]` |
| HP Hawser Straight | `hp_1` | A long rear straight, the whole frame behind it | `[42, 92, 30, 18]` |
| LK Mast Kick | `lk_1` | A **push kick**: the lead leg straight out at belly height, sole first | `[54, 58, 40, 20]` |
| HK Deck Sweep | `hk_0`–`hk_3` (`hk_1`) | A low sweep, the foot along the ground | `[54, 0, 38, 14]` |
| Throw Gangway Heave | `throw_*` | Both hands on the opponent, heaving them aside like cargo | n/a |
| Special Bowline Heel (214 + K) | `special_0`–`special_3` (`special_2`) | A step back (`_0`), a turn on the lead foot (`_1`), a **long spinning heel kick at head height** (`_2`), and the slow re-set (`_3`) | `[46, 76, 42, 24]` |

- **Win:** arms folded and a slow nod; then one hand raised, palm out: done.
- **Champion:** both fists overhead, the chain stretched between them.

### 4.3 Greta Voll (F3): power brawler, brass

**Look.** Broad and solid, fifties. Iron-grey hair in a tight bun. A khaki work shirt with rolled sleeves under a **brass-brown leather foundry apron**, dark work trousers, heavy leather gloves and steel-toe boots.

**Stance.** Square and low, fists at the chin. She ran the Voll Works floor until the Broker's money shut it.

**Colour key:**

| Material | Mid / light |
| --- | --- |
| Apron | `#70421c` / `#9a6228` |
| Shirt | `#6e6340` / `#928658` |
| Trousers | `#2e3440` / `#444c5c` |
| Hair | `#6e6e74` / `#94949a` |
| Skin | `#94604a` / `#b88468` |

| Move | Frames (active) | Active pose | Hitbox |
| --- | --- | --- | --- |
| LP Short Jab | `lp_1` | A short, heavy jab | `[30, 84, 22, 16]` |
| HP Slag Knuckle | `hp_1` | An overhand right coming down, the whole body dropping into it | `[34, 86, 28, 22]` |
| LK Toe-Cap Kick | `lk_1` | A short, low front kick with the steel toe | `[40, 30, 30, 20]` |
| HK Pig-Iron Sweep | `hk_0`–`hk_3` (`hk_1`) | A low, heavy sweep | `[44, 0, 36, 14]` |
| Throw Crucible Toss | `throw_*` | Hauls the opponent up by the belt and tosses them like a crucible | n/a |
| Special Ladle Drop (623 + P) | `special_0`–`special_3` (`special_2`) | Both fists locked together and raised overhead (`_0`, `_1`), then **brought down like a ladle tipping** as she lunges forward (`_2`), and the heavy recovery (`_3`). The move travels: the fists land well ahead of her. | `[24, 40, 44, 72]` |

- **Win:** fist into palm; then the fist raised, for the Foundry.
- **Champion:** arms wide and high, the chain held taut across above her head.

### 4.4 Jun Arata (F4): rushdown and feints, jade

**Look.** Small and wiry, early twenties. Messy black hair. A faded **jade track jacket** with the sleeves shoved up, loose black trousers, **red high-tops** and taped wrists.

**Stance.** Hunched and light on his feet, never still. He is a pickpocket who works the crowd between bouts and fights like he steals: quick, close, gone.

**Colour key:**

| Material | Mid / light |
| --- | --- |
| Jacket | `#1a5e3c` / `#2a8556` |
| Trousers | `#1f2126` / `#2f323a` |
| Shoes | `#741a18` / `#9c2a22` |
| Skin | `#74503a` / `#9a7052` |

| Move | Frames (active) | Active pose | Hitbox |
| --- | --- | --- | --- |
| LP Pick Jab | `lp_1` | The fastest jab in the cast: a flick, the shoulder barely turned | `[30, 80, 20, 14]` |
| HP Pocket Hook | `hp_1` | A tight hook to the ribs | `[32, 84, 24, 18]` |
| LK Lift Step | `lk_1` | A low stamp at the shin as he steps in | `[36, 6, 32, 16]` |
| HK Turnstile Kick | `hk_0`–`hk_3` (`hk_1`) | A **mid spinning kick** (not a sweep): chamber, the foot round at chest height, re-chamber, set down | `[38, 68, 34, 20]` |
| Throw Purse Snatch | `throw_*` | A quick grab and a shove past, as if lifting a wallet | n/a |
| Special Sleight Feint (236 + P) | `special_0`–`special_3` (`special_2`) | A **low dash** (`_0`, `_1`) that ends in a quick elbow (`_2`), then straight back to his stance (`_3`) | `[16, 64, 30, 30]` |

- **Win:** a coin walked over the knuckles, then flicked up and caught.
- **Champion:** both fists overhead, the chain stretched between them.

### 4.5 The Broker (F5): boss, gold and charcoal

**Look.** Tall and lean, sixties. Silver hair combed straight back. A **charcoal three-piece suit**, a white shirt and a burgundy tie, a **gold watch chain across the waistcoat**, black kid gloves and polished shoes.

**Stance.** He fights like the old bare-knuckle prints on his walls: upright, fists forward, never in a hurry. Nobody knows his name; he runs the Iron Ring and owns half of Grayhaven's debts. He should feel composed and expensive next to the rest of the cast, and dangerous because he never seems to try.

**Colour key:**

| Material | Mid / light |
| --- | --- |
| Suit | `#1b1d28` / `#2a2d3c` |
| Shirt | `#ccc6b6` / `#e6e0d0` |
| Tie | `#701822` / `#962632` |
| Gold | `#a07020` / `#d0a038` |
| Hair | `#9a9ca4` / `#c4c6cc` |

| Move | Frames (active) | Active pose | Hitbox |
| --- | --- | --- | --- |
| LP Cufflink Jab | `lp_1` | A precise upright jab | `[36, 88, 24, 16]` |
| HP Signet Cross | `hp_1` | A rear cross, the ring hand leading | `[40, 90, 30, 18]` |
| LK Oxford Kick | `lk_1` | A short front kick with a polished shoe at belly height | `[44, 40, 32, 20]` |
| HK Collection Sweep | `hk_0`–`hk_3` (`hk_1`) | A low sweep | `[48, 0, 40, 14]` |
| Throw Foreclosure | `throw_*` | Takes the opponent by the lapels and sets them down hard | n/a |
| Special Final Offer (236 + P) | `special_0`–`special_3` (`special_2`) | The hand drawn back (`_0`), a long step (`_1`), an **open-palm strike to the chest** (`_2`), then a tug at the cuff (`_3`) | `[20, 80, 40, 30]` |

- **Win:** straightening the tie; then a small wave: next.
- **No champion frames.**

---

## 5. Venues

### 5.1 Shared layout

All three venues use the layered-stage contract of the M2 four ([v2 brief §4.1](vertical-slice-art-brief.md#41-how-layered-stages-work), asset spec §3.1).

- **The world:** 800 px wide. The camera shows 640 px and travels 160 px end to end.
- **The layer stack is fixed.** The layers and their placement are already in `Assets/Data/stages.json` and stay exactly as they are:

| File | Parallax | Size | Position | Role |
| --- | --- | --- | --- | --- |
| `<id>_back.png` | 0.25 | 680 × 360, opaque | 0, 0 | Back wall or sky |
| `<id>_mid.png` | 0.55 | 728 × 360 (The Hold: 2 frames, 728 × 720) | 0, 0 | Structure, transparent where the back shows |
| `<id>_crowd.png` | 0.8 | 768 × 84 per frame, **8 frames** (768 × 672) | 0, 186 | Spectators behind a barrier |
| `<id>_floor.png` | 1.0 | 800 × 98 | 0, 262 | The fight floor with the chain ring |
| `<id>_front.png` | 1.25, drawn over the fighters | 850 × 360 | −10, 0 | Framing pieces at the stage ends |

- **These rules hold on screen at every camera position:**
  - **Ground line y 316.** The fight floor band is about y 296–340.
  - **The chain ring** (welded chain, a flattened ellipse of about 564 × 48 px) is centred at **world x 400, y 326**, which is x 400, y 64 inside the floor layer. It appears on every venue.
  - **HUD bands:** y 0–52 and y 330–360 stay calm and dark-ish. No bright lights, busy detail or text there.
  - The floor has lower contrast than the fighters. The crowd is darker than the fighters (the bottom 40% of the palette's values, rim light excepted).
  - The `front` layer frames the **stage ends only**. Nothing of it may enter the middle 560 px of the world, and the rest is transparent.
- **Crowds** follow the v2.1 crowd rules in full ([v2.1 brief §3.3](art-pack-v2.1-brief.md#33-the-spectators)):
  - **Frames:** 0–3 at rest, 4–7 cheering.
  - **The row:** one continuous row of about 40–48 spectators, none repeated or mirrored, silhouettes with the venue's rim light and no faces.
  - **Heads:** head tops at rows 26–36 at rest. Every 128 px strip has a head above row 37 in frames 0–3 (the self-test checks this).
  - **Cheering:** arms with sleeves and hands, and nothing floating.
  - **The barrier** is drawn over the people and stays within rows 50–72 of the frame, as in each placeholder (its bars and rope pass below the heads). From row 66–72 down to row 83 it is the solid edge the crowd stands on.
- **The placeholder** shows the composition and the barrier positions. Copy those, not its rendering, which is flat. To see it:
  - files: `Assets/Stages/<Venue>/`;
  - the generator: `python3 Tools/Art/generate.py venues --assets out`;
  - in the game: `IronRing -demo -stage <id>`.

### 5.2 Dry Dock 4 (S1), `Assets/Stages/DryDock/`, id `dry_dock`

A flooded dry dock under a laid-up freighter's bow: the ring on wet concrete, dockers behind a sagging chain. The same harbour as Pier 9, further from the city. The key light is a **sodium floodlight** from the upper left; teal harbour light fills from the water.

| File | Content |
| --- | --- |
| `dry_dock_back.png` | Night sky with a few stars; distant cranes with red warning lamps and the city's lights along the horizon; the dock's far wall, stepped concrete altars going down to black water. A painted dock number **4** on the wall is welcome. |
| `dry_dock_mid.png` | The **bow of a laid-up freighter** on its keel blocks: riveted plates with rust running down, a faded red boot-topping stripe, the anchor hawse. A floodlight tower on the left, its beam crossing the dock. |
| `dry_dock_crowd.png` | Dockers and longshoremen in work jackets, caps and oilskins, standing on the dock edge behind **steel stanchions with a chain sagging between them** (posts about every 64 px, chain across rows 54–66). Rim light from the floodlight. |
| `dry_dock_floor.png` | Wet concrete slabs with seams; puddles holding the floodlight and the teal sky; the chain ring at x 400, y 64. |
| `dry_dock_front.png` | A mooring bollard with a hawser hanging off it at each end; dark, silhouetted. |

### 5.3 The Hold (S2), `Assets/Stages/ShipHold/`, id `ship_hold`

The hold of an impounded freighter: Idris's ship, the *Marisol*. Steel ribs, stacked crates, a work lamp swinging under the open hatch. Enclosed and close after the open harbours. The key light is the **warm work lamp** overhead; cold moonlight falls from the hatch.

| File | Content |
| --- | --- |
| `ship_hold_back.png` | The riveted bulkhead: frames (ribs) about every 60 px, plate seams and rivet rows, rust and old paint. High up (but below the HUD band's bright limit), the open hatch with a slice of night sky and a shaft of moonlight. A stencilled **MARISOL** or a hold number is welcome. |
| `ship_hold_mid.png` | **Two frames**, 30 ticks each (frame 1 at rows 0–359, frame 2 at 360–719). Crates and a shipping container (doors shut) stacked at both ends; a cargo net hanging from the hatch coaming; a **work lamp on a cable** that swings between the two frames, its pool of light moving with it. Everything except the lamp and its light identical in both frames. |
| `ship_hold_crowd.png` | Sailors and dockhands standing on crates behind **cargo netting strung between deck stanchions** (posts about every 56 px, net across rows 50–72). Rim light from the lamp, from above. |
| `ship_hold_floor.png` | Steel deck plates with weld seams, two drain grates, oil stains; the chain ring at x 400, y 64. |
| `ship_hold_front.png` | A steel ladder up the hull at each end, and a frame (rib) edge; dark. |

### 5.4 The Broker's Club (S3), `Assets/Stages/BrokersClub/`, id `brokers_club`

The Broker's private club, and the last fight of the arcade: velvet, brass and gilt frames, the city's money watching from behind a red rope. It should feel like the opposite of every other venue: warm, rich, clean, and wrong. The key light is the **warm gold of the chandeliers**; the fill is deep red from the velvet.

| File | Content |
| --- | --- |
| `brokers_club_back.png` | Dark damask wallpaper over dark wood panelling with a brass dado rail (at about y 198). Gilt-framed paintings: **old bare-knuckle prize-fight prints** (invented fighters, nothing recognisable). Brass wall sconces. |
| `brokers_club_mid.png` | A mezzanine across the top with a brass rail; red velvet drapes; **two chandeliers**. Keep the parts inside the top HUD band (y 0–52) dim; the chandeliers' brightest crystals sit below y 52. |
| `brokers_club_crowd.png` | The city's money: dinner jackets, evening coats, a fur stole, a cigar, a raised glass, a betting slip in a gloved hand. Behind **brass stanchions with a red velvet rope** (posts about every 72 px, the rope sagging across rows 52–66). Fewer and better-dressed than the dockers, but still at least one head in every 128 px strip. |
| `brokers_club_floor.png` | Polished herringbone parquet in two shades, the chandeliers reflected in it; the chain ring at x 400, y 64. The chain is the one rough thing in the room. |
| `brokers_club_front.png` | The edges of a velvet curtain at each end, gathered with a gold tie. |

### 5.5 Previews: recompose, don't draw

Each venue's preview `<id>.png` (640 × 360) appears on the stage select, the title attract screen and the arcade's versus card. It must equal the layers composed exactly as the game draws them: frame 0 of every layer, camera in the middle of the stage, no drift. After placing the layers in `Assets/Stages/<Venue>/`, run:

```sh
python3 Tools/Art/compose_previews.py
```

The art self-test fails if a preview and its layers disagree.

---

## 6. Delivery package

One ZIP, **`IronRing_ArtPack_v3.zip`** (or `_v3A` and `_v3B` for two drops), laid out like the v2 pack:

```text
IronRing_ArtPack_v3/
  README.md                 what is in the pack, per deliverable; anything not done and why;
                            any pose that could not fit its hitbox
  PROVENANCE.md             tools, models and inputs (see below)
  LICENSE                   zlib, as the project (the art is contributed under it)
  CONTENTS_SHA256.txt
  Fighters/Sable/frames/    the 55 frames + manifest.json (partner offsets)
  Fighters/Sable/           sable_portrait.png (32 × 32), sable_select.png (48 × 48)
  Fighters/Idris/ ...       the same for Idris, Greta and Jun
  Fighters/Broker/frames/   the 49 frames + manifest.json
  Fighters/Broker/          broker_portrait.png
  Stages/DryDock/           dry_dock_back/mid/crowd/floor/front.png, dry_dock.png (recomposed preview)
  Stages/ShipHold/          ship_hold_*.png, ship_hold.png
  Stages/BrokersClub/       brokers_club_*.png, brokers_club.png
  Previews/                 per fighter: a contact sheet of every frame at 2× on mid-grey, and a GIF
                            of idle, walk, each attack and the special; per venue: a 1× screenshot at
                            both camera ends and a GIF of the crowd at rest then cheering
  Source/                   character sheets and prompts; large originals optional
                            (they stay out of the game repository)
```

- **`PROVENANCE.md`** must state:
  - the tools and image models used;
  - what was given to them as input: the project's own references, pose guides and earlier Iron Ring art only;
  - that no franchise screenshots, characters, logos or real brands or people were used;
  - that the pack is contributed under the project's **zlib** licence.

  `ArtSource/Pack-v2/` has a good model.
- **Don't include** code patches, new fighter JSON, a new `stages.json` or unchanged files.

---

## 7. Check before delivering

The source package builds the game and the tools; `README.md` at its root explains how. Python 3.13 with `pip install -r Tools/Art/requirements.txt` runs the art tools. Run these checks and fix what they find.

1. **Frames.** `python3 Tools/Art/review_frames.py sable idris greta jun broker --sheets review`.
   - It measures the animation rules: breathing, rocking, key-pose difference, reach into hitboxes, crouch height and outline colours.
   - It draws review sheets with the boxes.
   - Aim for 0 findings; the placeholders have 1–2 each.
2. **Import and build.**

   ```sh
   python3 Tools/Art/import_art.py fighter sable <frames> --origin 128 232 --scale 1 --no-quantize --manifest <frames>/manifest.json
   #   ... the same for idris, greta, jun and broker
   python3 Tools/Art/compose_previews.py
   cmake --build --preset release
   ```

   - AssetPacker validates everything at build time, and errors fail the build. It checks:
     - missing frames;
     - animation lengths against the frame data;
     - layer widths against the camera travel;
     - crowd frame counts;
     - portrait sizes.
   - `ctest --preset release` runs the content tests.
3. **Art self-test.** `python3 Tools/Art/selftest.py` checks the crowds strip by strip and every preview against its layers.
   - It also compares the files with `generate.py`'s placeholders. Lines saying `pixels differ from generate.py output` are expected for the files you replaced; the project clears them on import.
   - Every other finding must be fixed.
4. **In the game.** The player is `Build/release/Player/Release/IronRing` (`.exe` or `.x86_64`). Add `-assets-folder Assets` to use loose files without repacking.
   - **Boxes and throws:** `IronRing -training -fighters sable,dee -stage dry_dock`, then press F3 to show hit and hurt boxes with frame data. On every active frame the striking limb is inside the red box. Throw the dummy and check it lands in the hands. Repeat for each fighter. The Broker can be set with `-fighters broker,dee`.
   - **Venues:** `IronRing -demo -stage dry_dock` (also `ship_hold`, `brokers_club`). The camera reaches both ends within a match. Look for seams, layers running out, the lamp swinging in The Hold, and the crowd cheering after a knockdown.
   - **The select screen:** check the new select portraits next to Dee's and Bram's. Heavy punch shows a fighter's move list; the moves should read as their names.
   - **The ending:** this command plays the whole arcade ladder with a CPU at the controls and saves the champion pose:

     ```sh
     IronRing -batchmode -autoplay -frames 20000 -keys "10:Return,30:Return,<cursor>90:Return" -screenshot ending.png
     ```

     The select grid is 3 × 2: Dee, Bram, Sable on top; Idris, Greta, Jun below. Before frame 90, move the cursor from Dee with presses at least 6 frames apart, starting at frame 70:
     - Sable: `<cursor>` = `70:Right,76:Right,`
     - Idris: `70:Down,`
     - Greta: `70:Down,76:Right,`
     - Jun: `70:Down,76:Right,82:Right,`
5. **Pixels.**
   - Alpha only 0 or 255.
   - At most 48 colours per fighter.
   - Outlines in the darkest tone of each material, with no light or cyan rim.
   - Nothing bright in the HUD bands.
   - Feet on the ground line.

**What happens next.** The project imports the pack, reviews every frame and layer against this brief in the game, and sends back a correction list with frame names, as with Art Packs v1 and v2. Once it is in, no placeholder art is left in the game.

---

## Appendix: prompts

The style block, negative prompt and stage block are in [`generation-prompts.md`](generation-prompts.md) and the v2 brief's Appendix A; use them unchanged. Character prompts, to be followed by the style block and that frame's pose:

> **Sable Reyes**: a lean, long-legged woman in her early twenties, black hair in a high ponytail, violet sleeveless top, black kickboxing shorts, white shin wraps, worn grey trainers, fingerless gloves, tall bladed kickboxing stance with hands high, athletic, alert, confident.

> **Idris Okafor**: a tall, rangy dark-skinned man in his forties, close-cropped greying hair and a short beard, faded blue work shirt with sleeves rolled to the elbow, canvas trousers rolled at the shin, a rope tied as a belt, deck shoes, upright side-on stance with open hands, calm, weathered sailor.

> **Greta Voll**: a broad, solid woman in her fifties, iron-grey hair in a tight bun, khaki work shirt with rolled sleeves under a brown leather foundry apron, dark work trousers, heavy leather gloves, steel-toe boots, square low boxing stance with fists at the chin, stern, immovable.

> **Jun Arata**: a small, wiry young man in his early twenties, messy black hair, faded jade-green track jacket with sleeves shoved up, loose black trousers, red high-top sneakers, taped wrists, hunched light-footed stance, sly grin, restless.

> **The Broker**: a tall, lean man in his sixties, silver hair combed straight back, charcoal three-piece suit, white shirt, burgundy tie, gold watch chain across the waistcoat, black leather gloves, polished shoes, upright old-fashioned bare-knuckle stance with fists forward, composed, cold, wealthy.

Stage layers: the shared stage block, then *"separate background layer for a side-scrolling fighting game, [what this layer holds], everything else fully transparent"*. Image generators don't keep a crowd consistent across frames: **generate crowd frame 0, then make the others by hand-editing it**. The Hold's two `mid` frames are one image with the lamp redrawn.
