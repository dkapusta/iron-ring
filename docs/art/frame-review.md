# Frame review: Art Pack v1 fighters

Review of Dee's 49 and Bram's 50 frames as adopted in [ADR-0012](../adr/0012-adopting-external-art.md). These notes are for the next art pass. The current frames are shippable: every frame exists, sizes and pivots are consistent, every active frame reaches its hitbox, and throws line up.

**How it was reviewed.**
- Per-animation sheets with hurtboxes, active hitboxes and throw partner points at 3× (`python3 Tools/Art/review_frames.py --sheets build/review`).
- The tool's measurements (head travel, body drift, silhouette change between key poses, hitbox reach, crouch height, outline colour).
- In-game play in training mode on all three stages (`IronRing -training -stage <id>`).

Re-run the tool after each pass; its thresholds are the rules in [asset-spec §2.6](asset-spec.md#26-animation-rules).

## Summary

| Area | Dee | Bram |
| --- | --- | --- |
| Model consistency (size, proportions, costume) | good | good |
| Striking poses and reach | good, except the uppercut | punches read short |
| Locomotion (idle, walk) | flat idle, walk rocks | flat idle, walk rocks strongly |
| Throws and grabs | key poses too similar | key poses too similar |
| Reactions, knockdown, win | good | good, weak `hit_crouch` |
| Outline | mostly dark; some cyan rim | light and cyan rim on 20 % of edges |

## Priorities

**P1 — gameplay readability** (players misread the move):
1. Bram `lp_1`, `hp_1`: extend the striking arm (see below). The fists touch the hitbox only at its near edge, so the moves look like they hit from out of reach.
2. Both `throw_0` → `throw_1`, and Bram `special_0` → `special_1`: make the grab visible. Today the reach and the grip are the same pose, so a throw looks like a free animation.
3. Dee `special_2`: angle the uppercut forward into its hitbox.

**P2 — animation quality** (reads, but feels stiff or wobbly):
4. Both idles: add the breathing bob.
5. Both walk cycles: stop the body rocking; add contact and passing poses with a 1–2 px dip.
6. Bram `hp_2`, `hp_3`: real follow-through and recovery poses instead of near-idle.
7. Bram `hit_crouch`: a flinch, not a forward reach.

**P3 — polish**:
8. Outline pass on both fighters: remove the generator's cyan rim and light edge pixels.
9. Distinct second frames for `win_1` (both) and `hit_1` (Dee): more change between the two.
10. Dee `lp_0`: a small chamber, so the jab has a wind-up.

## Dee Marlowe

| Frame(s) | Status | Correction |
| --- | --- | --- |
| `idle_0`–`idle_3` | P2 | Head and shoulders stay at 130 px. Lower the shoulders and head by 0, 1, 2, 1 px across the four frames, and let the guard hands follow by 1 px. Don't move the feet. |
| `walk_0`–`walk_5` | P2 | Body centre swings about 4 px backwards on `walk_1` and forwards on `walk_3`. Keep the torso within ±2 px of the centre line. Make `walk_0`/`walk_3` contact poses (feet apart, 1 px lower) and `walk_1`/`walk_4` passing poses (feet together, 1 px higher). The rear foot should push off, not slide. |
| `crouch`, `guard_crouch` | ok | 96/94 px against a 92 px hurtbox; the extra is hair. Keep. |
| `lp_0` | P3 | Identical to idle. Pull the lead fist back 2–3 px and drop the lead shoulder slightly. |
| `lp_1`, `lp_2` | ok | Good extension and retraction. |
| `hp_0`–`hp_3` | ok | Clear wind-up and extension; `hp_2` holds the arm out as a follow-through. |
| `lk_0`–`lk_2` | ok | The chamber and the extension read well. The hitbox moved to the foot in ADR-0012. |
| `hk_0`–`hk_3` | ok | The sweep is clear, and the leg runs along the floor inside the box. |
| `throw_0`, `throw_1`, `throw_hold_0` | P1 | All three are the same reaching stance. `throw_0`: both arms forward at chest height, open hands. `throw_1`: hands closed on the opponent's collar and upper arm, elbows bent, weight forward. `throw_hold_0`: clinch: pull the opponent in, chest to chest, rear foot braced. |
| `throw_hold_1` | ok | The lean-back toss reads; the hands are near the partner point. |
| `special_0`, `special_1` | ok | Good dip and drive. |
| `special_2` | P1 | The fist goes straight up above the head, so the arm is outside most of the hitbox `[10, 60, 40, 90]`. Angle the arm 20–30° forward so the forearm and fist sweep through x 15–35; rise onto the toes. |
| `special_3` | ok | Landing recovery reads. |
| `hit_0`, `hit_1` | P3 | Both are the same recoil. Make `hit_0` the snap (head back, shoulders up) and `hit_1` the stagger (weight on the rear foot, arms dropping). |
| `hit_crouch`, `held_0`, `held_1` | ok | |
| `down_0`–`down_3` | ok | Fall, lie, sit-up and rise read well; grounded within 2 px. |
| `win_0`, `win_1` | P3 | Nearly identical. Give `win_1` a clear change: fist higher and head thrown back, or the other arm raised. |

## Bram Hollis

| Frame(s) | Status | Correction |
| --- | --- | --- |
| `idle_0`–`idle_3` | P2 | No vertical movement, and the head jumps about 6 px forward and back between `idle_0` and `idle_1`, so the loop pops. Breathing bob 0, 1, 2, 1 px on the head and shoulders; keep the head within ±1 px horizontally. A heavy fighter can breathe slower: 12 ticks per frame in the data is fine. |
| `walk_0`–`walk_5` | P2 | Body centre swings 5–11 px between frames (`walk_2` leans back, `walk_3` snaps forward). Torso within ±2 px of the centre line; contact and passing poses as for Dee, a 2 px dip, heavy plant on contact. |
| `crouch` | ok | 102 px against a 100 px hurtbox (skull). Keep. |
| `guard`, `guard_crouch` | ok | |
| `lp_0`–`lp_2` (Palm Shove) | P1 | `lp_1` keeps both hands at the chest. Push both palms out at full arm length (fingers up, heels of the hands forward) so the hands reach about x 55–60 at chest height; shoulders forward. |
| `hp_0`–`hp_3` (Anchor Hook) | P1/P2 | `hp_1`: a real hook. The lead arm bends 90° and sweeps across, the fist reaches x 60–65, the torso turns into it and the rear heel lifts. `hp_2`: the fist carries past the centre line, follow-through. `hp_3`: the arm comes back to guard, the body unwinds. Today `hp_2`/`hp_3` are near-idle. |
| `lk_0`–`lk_2`, `hk_0`–`hk_3` | ok | The boot kick and Hull Breaker sweep are strong. |
| `throw_0`, `throw_1` | P1 | Only 5 % silhouette change. `throw_0`: lunge, arms wide. `throw_1`: big hands clamp on the opponent's shoulders, elbows bent. |
| `throw_hold_0`, `throw_hold_1` | ok/P2 | `throw_hold_0` repeats `throw_0`; make it the lift: knees bent, the opponent pulled up to chest height. `throw_hold_1` (toss) reads. |
| `special_0`, `special_1` (Dock Hoist grab) | P1 | Too similar. `special_0`: low lunge, arms spread wide (a bear-hug reach). `special_1`: arms closed around the opponent's waist. |
| `hoist_0`–`hoist_2` | ok | Lift, press overhead and slam read; the partner points match the hands. `hoist_1` arms could be thicker (straight thin arms against his bulk). |
| `hit_0`, `hit_1`, `held_0`, `held_1` | ok | |
| `hit_crouch` | P2 | Arms reach forward, which reads as a guard or grab. Head snapped back, arms in, shoulders hunched. |
| `down_0`–`down_3` | ok | |
| `win_0`, `win_1` | P3 | Double-biceps in both; `win_1` only shifts the arms. Make `win_1` a shout: chin up, mouth open, fists higher. |

## Outline pass (both, P3)

The generator painted a cool rim light on the silhouette edges. After quantisation:
- **Bram:** 11 % of edge pixels are light and 9 % are cyan.
- **Dee:** the same rim shows on her arms and legs, though part of her teal is genuine costume.

On teal-lit stages such as Pier 9's harbour, this rim makes the fighters shimmer. Fix: recolour every silhouette-edge pixel to the darkest tone of the material it bounds. Keep the rim light one pixel inside the outline, if at all.

## Process notes for the next pack

The pack's frames came from multi-pose boards and were cropped and fitted afterwards. That produced the scale problem fixed in ADR-0012 and most of the "same pose twice" problems above: the generator drew a generic stance where the board slot asked for a key pose. The updated workflow in [generation-prompts.md](generation-prompts.md) asks for one generation per frame, anchored on the frame's pose reference, with the key-pose descriptions from this review. It also asks for a review-tool run before delivery.
