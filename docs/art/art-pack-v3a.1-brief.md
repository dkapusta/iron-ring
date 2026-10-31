# Iron Ring: art correction brief (Art Pack v3A.1, characters)

**For:** whoever produced Art Pack v3A.
**From:** the Iron Ring project, 2026-10-31.
**With:** the current Iron Ring source package. Art Pack v3A is in it, at `ArtSource/Fighters/<Name>/frames/`. Every path below is relative to its root.

Art Pack v3A is in the game and passes every automated check: 0 frame-review findings, the art self-test, the build and the tests. The full review is in [`ArtSource/Pack-v3A/REVIEW.md`](../../ArtSource/Pack-v3A/REVIEW.md).

This is a small correction round of **four items**, for the Broker, Sable and Idris. Nothing else changes. When it is in, the venues (the v3 brief's S1–S3) follow as the next drop.

The rules of the [Art Pack v3 brief](art-pack-v3-brief.md) still apply. Keep:
- the canvas (256 × 256 RGBA) and origin (128, 232);
- hard alpha;
- each fighter's existing palette (at most 48 colours) and outline;
- the scale and standing height;
- the feet of every untouched frame.

---

## 1. What to deliver

| # | Deliverable | Files | Priority |
| --- | --- | --- | --- |
| K1 | **The Broker's skin**, recoloured (§2) | all 49 frames in `ArtSource/Fighters/Broker/frames/`, and `Assets/Fighters/Broker/broker_portrait.png` | Required |
| K2 | **Sable's walk contact frames** (§3) | `walk_0.png`, `walk_3.png` | Required |
| K3 | **Idris's Mast Kick** (§4) | `lk_1.png`; `lk_0.png` and `lk_2.png` if the chamber needs to follow | Required |
| K4 | **Idris's Bowline Heel** (§5) | `special_2.png`; `special_1.png` and `special_3.png` if the arc needs to follow | Required |
| K5 | Sable's Wire Kick, 8–10 px lower (§6) | `hk_1.png` | Recommended |

**Not wanted:** any other frame, any portrait other than the Broker's, the atlases (the project repacks them), the fighter JSON, the hitboxes or venue art.

---

## 2. K1: the Broker's skin

**Problem.** His face and hands are drawn mostly in the **gold ramp** (`#d0a038`, `#a07020`), the colours of his watch chain. His real skin ramp (`#6e4c3e`, `#9a7060`) survives only in the shadows.
- In the game, at 1×, he reads as gold-masked or jaundiced, in the HUD portrait as much as in the fight.
- The tie's red (`#701822`, `#962632`) also appears as streaks on his cheeks and jaw, where it reads as blood. The game has no blood.

**Fix.** Recolour the skin only: face, ears, neck and the bare wrists. A sixties, pale, well-kept man, lit warm from the upper left.

- **Use the Broker's skin ramp from `Tools/Art/ironart/palette.py`** (line → highlight):

  ```text
  #2a1a14  #6e4c3e  #9a7060  #bf9480  #dcb8a2
  ```

  You may shift these slightly to suit the art.
- **Keep the gold** where it belongs: the watch chain, cufflinks, the signet ring.
- **Replace the red on the face** with the skin's shadow tones. Keep the red on the tie.
- **Silver hair, suit, shirt and gloves stay exactly as they are.**
- **Do it in every frame,** so the face doesn't change colour between frames, and in `broker_portrait.png`.
- **Colour count:** the total stays within 48 (it is 38 now).

**Check.** On a mid-grey background at 1× and 4×:
- his face reads as skin next to Idris's and Dee's;
- no gold remains on skin pixels;
- no red remains on the face.

---

## 3. K2: Sable's `walk_0` and `walk_3`

**Problem.** In both contact frames, a **flat, unshaded, skin-coloured slab** fills the space between her legs, from the thighs down to the shins. It has hard rectangular edges and a notch, so the two legs read as one block. It flickers twice in every walk cycle, at 5 ticks per frame.

**Fix.** Redraw both frames as clean contact poses:
- **Legs:** feet apart; the front leg reaching, the rear leg pushing off; the gap between them open to the background (transparent).
- **Shading:** each leg has its own outline and the shading of the other walk frames.
- **Unchanged:** the torso, head, arms and feet positions stay where they are, so the cycle and the breathing rules (torso within ±2 px) still hold.
- **Distinct frames:** `walk_3` is the opposite contact (the other leg forward), not a copy of `walk_0`.

---

## 4. K3: Idris's Mast Kick (`lk_1`)

**Problem.** The Mast Kick is a **push kick at belly height**. Its hitbox, and the leg's active hurtbox, sit low enough to hit a crouching opponent. The drawn leg is a head-high kick about 22 px above them, so the art misleads players about what the move hits.

| | x (forward from origin) | y (up from ground) |
| --- | --- | --- |
| Hitbox | 54 – 94 | 58 – 78 |
| Leg's active hurtbox | 16 – 82 | 56 – 78 |
| Drawn leg now (in front of x 54) | 54 – 85 | **77 – 102** |

**Fix.**
- **The kick:** the lead leg straight out, roughly horizontal, **sole first**, the foot inside the hitbox: the sole at about x 80–94, y 60–76.
- **The body:** the hips pushed forward behind the kick; the torso upright or leaning back slightly; the arms in his open-hand guard.
- **The supporting foot:** stays where it is.
- **The neighbouring frames:** re-aim `lk_0` (chamber) and `lk_2` (retract) at the same height if they no longer match.

---

## 5. K4: Idris's Bowline Heel (`special_2`)

**Problem.** A spinning heel kick at **head height**, but the heel is drawn about 24 px above its box.

| | x | y |
| --- | --- | --- |
| Hitbox | 46 – 88 | 76 – 100 |
| Leg's active hurtbox | 10 – 82 | 74 – 98 |
| Drawn leg now | 46 – 94 | **98 – 127** |

**Fix.**
- **The kick:** lower the kicking leg so the **heel is inside the box**, at about x 74–88, y 82–96. That is head height for a standing opponent: his head is at about y 100–130 at Dee's and Bram's size. The full extension and the spin of the torso stay as they are.
- **The neighbouring frames:** if `special_1` (the turn) or `special_3` (the re-set) no longer flow into it, adjust them to match.

---

## 6. K5 (recommended): Sable's Wire Kick (`hk_1`)

The foot is at y 98–116, and the hitbox is x 42–80, y 82–104, so they overlap by only about 6 px. Lower the kicking leg 8–10 px so the foot sits in the box, centred on about y 94. Leave it if the pose suffers.

---

## 7. Check before delivering

With the source package:

1. **Put the files in place.** The corrected frames go into `ArtSource/Fighters/<Name>/frames/`, and the Broker's portrait into `Assets/Fighters/Broker/`.
2. **Repack:**

   ```sh
   python3 Tools/Art/import_art.py fighter broker ArtSource/Fighters/Broker/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Broker/frames/manifest.json
   #   ... the same for sable and idris
   ```

3. **Frame review:** `python3 Tools/Art/review_frames.py sable idris broker --sheets review` still reports 0 findings. In the sheets, the Mast Kick and Bowline Heel feet are inside their red boxes.
4. **Art self-test:** `python3 Tools/Art/selftest.py` ends with `art selftest: ok`.
5. **In the game** (if you can build it):
   - `IronRing -training -fighters idris,dee`, then press F3 to show the boxes, and throw both kicks;
   - `IronRing -training -fighters broker,dee` to look at the face and the HUD portrait;
   - walk Sable back and forth (`-fighters sable,dee`).

## 8. Delivery

One ZIP, **`IronRing_ArtPack_v3A.1.zip`**, with paths that mirror the repository:

```text
IronRing_ArtPack_v3A.1/
  README.md                   per item: what changed; anything not done and why
  CONTENTS_SHA256.txt
  ArtSource/Fighters/Broker/frames/*.png          all 49 (K1)
  Assets/Fighters/Broker/broker_portrait.png      (K1)
  ArtSource/Fighters/Sable/frames/walk_0.png, walk_3.png   (K2); hk_1.png (K5, if done)
  ArtSource/Fighters/Idris/frames/lk_*.png, special_*.png  (K3, K4; only the frames changed)
  Previews/                   before/after at 4× for each item; the review sheets for lk and special
```

- **Don't include** the atlases, manifests (unless a partner offset changed, which none should), code or JSON.
- **Licence:** the same terms as v3A (zlib); `PROVENANCE.md` is needed only if the tools or inputs differ from v3A.
