# Art Pack v3A (characters): technical review

**Reviewed:** 2026-10-31, against the [Art Pack v3 brief](../../docs/art/art-pack-v3-brief.md), in the game and with the project tools.
**Verdict:** adopted as delivered. Four corrections are needed before release; they are written up as the [v3A.1 correction brief](../../docs/art/art-pack-v3a.1-brief.md). Everything else meets the brief.

## Contents

| Fighter | Frames | HUD portrait | Select portrait | Colours | Standing height |
| --- | ---: | --- | --- | ---: | ---: |
| Sable Reyes | 55 | 32 × 32 | 48 × 48 | 36 | 132 px |
| Idris Okafor | 55 | 32 × 32 | 48 × 48 | 37 | 139 px |
| Greta Voll | 55 | 32 × 32 | 48 × 48 | 36 | 121 px |
| Jun Arata | 55 | 32 × 32 | 48 × 48 | 37 | 118 px |
| The Broker | 49 | 32 × 32 | none (not on the select) | 38 | 140 px |

Each fighter is 269 frames and 9 portraits in all. `CONTENTS_SHA256.txt` verifies.

## Checks that pass

**Format.** The frames meet the canvas and colour rules:
- every frame the fighter files reference is present, with no extras;
- 256 × 256 RGBA at origin (128, 232);
- binary alpha;
- nothing below the ground line;
- at most 38 colours per fighter, shared with its portraits;
- standing heights exact to the brief.

**Tools.**
- `review_frames.py` reports 0 findings for all five; the placeholders had 1–2 each.
- `selftest.py` passes once the five folders are external.
- The atlases repacked by `import_art.py` are byte-identical to the pack's.

**Build.** AssetPacker validation, `ctest` and the golden checksum pass. Art doesn't touch the simulation; the balance matrix is unchanged.

**In the game.** Training, a demo, the select screen and the arcade ending (as Greta) look right. The new select portraits sit well beside Dee's and Bram's.

**Throws.** The new partner offsets (`throw_hold_1` raised to [35–54, 65–91]) put the opponent in the thrower's hands. The new fighters' `held_*` frames also land correctly under Dee's throw.

**Quality.** The sprites match Dee and Bram in line weight, shading and detail. Each cast description reads:
- Sable's ponytail and shin wraps;
- Idris's rope belt;
- Greta's apron and bun;
- Jun's jade jacket and red high-tops;
- the Broker's three-piece suit and watch chain.

Win poses and champion poses follow the brief; the chain is the shared metal ramp.

## Corrections required (v3A.1)

| # | Fighter | Frames | Problem | Measured |
| --- | --- | --- | --- | --- |
| 1 | The Broker | every frame and the HUD portrait | **Skin is drawn with the gold ramp.** The face and hands are mostly `#d0a038` and `#a07020`, the watch chain's colours, so he looks gold-masked or jaundiced. The red tie tone on his cheeks reads as blood. | In the top 22 rows of `idle_0` (the head), `#d0a038` covers 84 pixels and `#a07020` 23; the skin ramp's `#6e4c3e` and `#9a7060` cover 33 and 30, only in the shadows. |
| 2 | Sable | `walk_0`, `walk_3` | **A flat skin-coloured slab joins the legs** between the thighs and the shins. It has hard rectangular edges and no shading, and flickers twice per walk cycle. | Visible at 1×; clear at 4×. |
| 3 | Idris | `lk_1` (Mast Kick) | **The foot is at head height, not belly height.** The move is a push kick whose hitbox and active hurtbox sit at y 58–78, so it hits crouchers. The drawn leg sits above it, so the art doesn't match what the move does. | Leg at y 77–102 in front of x 54; the hitbox is y 58–78. Only 14 opaque pixels fall inside the box. |
| 4 | Idris | `special_2` (Bowline Heel) | **The heel is above its box.** | Leg at y 98–127; the hitbox is y 76–100 and x 46–88, with 18 pixels inside. |

## Smaller notes (not blocking)

- **Sable `hk_1` (Wire Kick):** the foot is at y 98–116 against a box at y 82–104, overlapping it by only 6 px. Lowering the kick 8–10 px would centre it. Wanted, if the frame is touched anyway.
- **Greta's Ladle Drop:** `special_0` and `special_1` raise one fist, not both locked together as the brief described. `special_2` lands two-handed, so the move still reads. `hp_1` (Slag Knuckle) is closer to a straight than an overhand. Acceptable.
- **Fists past their boxes:** several jabs and crosses reach 5–12 px beyond the front of the hitbox (Sable `hp_1`, Jun `hp_1`, the Broker `lp_1`). This is normal for fighting-game art and not a defect, but it should not grow.
- **Jun's hair:** it is a dark red-brown; the brief said black. It works with the jacket. No change wanted.

## Adoption

- **Frames:** copied to `ArtSource/Fighters/<Name>/frames/` and repacked with `import_art.py`.
- **Portraits:** in `Assets/Fighters/<Name>/`; `selectPortrait` is added to the four playable fighter files.
- **Ownership:** the five folders join `EXTERNAL` in `Tools/Art/ironart/ownership.py`, so `generate.py` no longer draws any fighter.
- **Provenance:** this folder holds the pack's README, provenance, licence, the frame-review and validation reports, and its small source records and palettes (about 0.8 MB). The 34 MB of generated studies and the 9 MB of previews stay outside the repository ([ADR-0008](../../docs/adr/0008-repository-and-binary-assets.md)).
