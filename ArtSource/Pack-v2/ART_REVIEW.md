# Art review and changes

## Snapshot review

The attached snapshot has production v1 fighter art already registered on 256×256 canvases, three flattened v1 venues, and procedural Foundry/front-end placeholders. The supplied frame review reproduced 11 baseline findings: 3 for Dee, 8 for Bram. Required visual corrections also include mistakes beyond the script's heuristics: a guard-like grab, a short palm shove, a straight arm where a hook should read, and the uppercut missing the intended forward sweep.

## Fighters

| Requirement | Delivered change |
| --- | --- |
| Scale and origin | Complete-canvas conversion, fixed (128,232); no frame-by-frame silhouette fitting. Existing v1 frames retained at their adopted scale. |
| Idle | One base pose with explicit 0/1/2/1 px head/shoulder travel; lower legs and feet identical. |
| Walk | Six drawn poses per fighter; contact versus passing silhouettes and 128/131/130/128/131/130 px head heights for Dee, 132/135/134/132/135/134 for Bram. Integer corrections keep torso centres near the adopted idle centre. |
| Bram palm shove | Two distinct open palms, fingers up and hand heels approximately x+55/x+62; forward shoulders. |
| Bram hook | Bent elbow with fist near x+63; torso twist and raised rear heel. Distinct follow-through and unwinding recovery. |
| Grabs | Open reach → bent-elbow C grips → close held clinch/lift. Dee's collar/upper-arm grip is drawn separately from guard. Partner offsets updated for the new hold positions. |
| Dock Hoist | Low arms-wide reach and waist clamp; thick overhead lift arms. |
| Dee uppercut | The striking arm is edited toward a 25° forward sweep, through the prescribed forward hitbox region. |
| Crouch recoil | Bram's head recoils and arms close in; compacted at the waist to fit the actual 100 px crouch box plus allowed skull slack. |
| P3 | Dark material outline pass on all frames; distinct win_1 for each fighter; Dee hit snap/stagger and jab chamber; Bram hoist arm thickness. |
| Champion | 0/1/2 intro plus 3/4/5 hold loop, alternating open/edge-on steel chain links; breathing/shout edits preserve the loop's feet and lower body. |

The measured torso-centre ranges in the walk's y120–164 band are Dee x+6.16…7.83 (idle +7.40), Bram x+15.43…18.64 (idle +16.94). These stay within approximately ±2 px of the adopted centres. This is a visual registration diagnostic, not a change to gameplay movement or collision.

## Venues and front end

The four venues have independent background, midground, crowd, floor and foreground layers. Depth and camera travel are supplied explicitly rather than widening a flattened picture. Source architecture is registered vertically to the brief's window/gantry, furnace-mouth, stall-awning and sign bands. Crowds are fixed cast groupings with hand-edited rest/cheer motion; flickering lanterns, blinking sign bulbs and water shimmer preserve structure.

All ring ellipses are drawn at world (400,326), approximately 564×48. Floor contrast and the lower HUD band are reduced; foreground columns/posts/chimneys remain at the very ends and leave the middle 560 px unobstructed. The title wordmark is redrawn in squared brass plates with native hard outline, bevel, seams, rivets and chain. The menu centre remains dark enough for the game's white text and fighter selection art.

## Review evidence and limits

- `frame_review.json`: zero findings for both fighters after correction.
- `validation.json`: native pixel/metadata contracts and exact repository importer reconstruction.
- `art_selftest.log`: supplied art self-test passed, including source/atlas matching and 4× reference round trip on the staged import.
- `source_snapshot_check.json`: all 406 original snapshot files unchanged, no added files.
- `Previews/`: source-based contact sheets, animation reels, camera endpoints, crowd states, paired throws and offline viewer.

The script checks complement the pose review; neither substitutes for a final game pass. CMake and SDL execution were unavailable here. In-game F3 limb/box inspection, camera following, crowd trigger timing and both arcade endings remain the project's final integration checks. These limits are documented rather than represented as completed playtests.
