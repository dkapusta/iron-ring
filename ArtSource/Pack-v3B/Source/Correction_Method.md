# Correction authoring notes

All final character assets are native 256 × 256 RGBA masters at origin (128, 232), apart from the Broker’s 32 × 32 HUD portrait. The original V3A files remain the baseline for every comparison.

## Corrective study directions

These are summaries of the image-edit instructions, not claimed verbatim prompt transcripts. The existing frame was supplied as the visual reference in each case.

| Study | Direction |
| --- | --- |
| Broker skin | A pale, well-kept man in his sixties; remove gold and red from skin; warm upper-left lighting; preserve silver hair, charcoal suit, white shirt, burgundy tie, black gloves and gold jewellery. |
| Sable walk 0 | Clean reaching/pushing contact legs, independently shaded, with an open gap; retain costume, torso, hands, head and ankle/foot anchors. |
| Sable walk 3 | Opposite contact with reversed near/far leg treatment; the same preservation constraints. |
| Idris Mast Kick | A sole-first belly-height push kick; fixed support foot and open-hand guard; native hitbox target (54, 58, 40, 20). |
| Idris Bowline Heel | A fully extended spinning heel kick lowered into the head-height box (46, 76, 42, 24); preserve torso spin and support foot. |
| Sable Wire Kick | Lower the extended high roundhouse into (42, 82, 38, 22), retaining support foot and upper-body pose. |

## Native finishing

Broker skin replacements use the existing five-colour skin ramp. Warm pixels are selected within per-pose skin regions; red components belonging to the tie and gold accessories are protected. The alpha channel and unaffected material pixels stay exact. This is a colour-only repair.

Sable’s contact leg studies are reduced, registered between the existing shorts and ankle anchors, and snapped to her existing palette. Canvas rows below 215 and above 185 retain their original pixels. The two contacts have independent leg shading and silhouettes.

The active kick legs are registered as local articulated regions, retaining the original artwork’s texture and colour clusters. Approximate terminal offsets are: Mast Kick +4 x / +22 canvas-y; Bowline Heel −7 x / +28 canvas-y; Wire Kick −4 x / +13 canvas-y. The Mast Kick shoe receives local vertical foreshortening and ankle cleanup to fit its short box; no whole-character rescaling occurs. Supporting feet are unchanged.

The actual final PNGs, the before/after sheets and the project-generated box sheets are authoritative. The neighbouring chamber/recovery frames were inspected in sequence and remain unchanged. Preview attack GIFs use slower display timing for review only.
