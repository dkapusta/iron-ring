# Art Pack v3B (venues) and v3A.1 (corrections): technical review

**Reviewed:** 2026-11-01, against the [Art Pack v3 brief](../../docs/art/art-pack-v3-brief.md) (S1–S3) and the [v3A.1 correction brief](../../docs/art/art-pack-v3a.1-brief.md) (K1–K5), in the game and with the project tools.
**Verdict:** adopted as delivered. All five corrections are fixed, and the three venues meet the brief. A few polish notes below are left for the owner's play-test; none blocks adoption.

## Contents

The pack holds 54 corrected fighter frames and one portrait, plus three venues of five layers each with their previews. The checksums in `CONTENTS_SHA256.txt` verify.

## Checks that pass

- **Tools:**
  - `review_frames.py` reports 0 findings for all five fighters.
  - `selftest.py` passes; it checks the crowd heads strip by strip and every preview against its layers.
  - `compose_previews.py` changes nothing: the delivered previews equal the layers.
- **Build:** AssetPacker validation (layer widths, crowd frame counts) and `ctest` pass. The simulation is untouched, so the golden checksum holds.
- **Ownership:** the three venue folders join `EXTERNAL`, and `generate.py` stops writing the `venues` part. No procedural fighter or venue art remains in `Assets/`.

## Corrections (v3A.1)

| # | Item | Result |
| --- | --- | --- |
| K1 | The Broker's skin | **Fixed.** The head's most-used colours are now the skin ramp (`#bf9480`, `#9a7060`, `#6e4c3e`, `#2a1a14`). Gold stays on the chain and cuffs, and the red is gone from his face. In the HUD portrait and in fights, he reads as a pale, silver-haired man. |
| K2 | Sable `walk_0`, `walk_3` | **Fixed.** The legs are separate and individually shaded, with an open gap between them; the upper body and feet are pixel-identical. Small note: one or two stray outline pixels remain beside the ankles in `walk_3`. They are invisible at 1×. |
| K3 | Idris Mast Kick (`lk_1`) | **Fixed.** A belly-height push kick, sole first. 402 opaque pixels fall inside the hitbox (14 before). |
| K4 | Idris Bowline Heel (`special_2`) | **Fixed.** The heel sits in its head-height box: 527 pixels inside (18 before). |
| K5 | Sable Wire Kick (`hk_1`) | **Done**, lowered 13 px: 381 pixels inside (91 before). |

## Venues (v3B)

In training, each venue was checked at the middle, the left end and the right end of the camera travel:

- **Dry Dock 4:**
  - a laid-up freighter's bow with rust streaks and a hawse chain;
  - a floodlight tower;
  - cranes and city lights across the harbour;
  - dockers behind stanchions and a sagging chain;
  - wet concrete with reflections;
  - a bollard at each end.
- **The Hold:**
  - riveted ribs and plates;
  - a cargo net and a lamp on a cable that swings between the two `mid` frames (804 pixels change);
  - crates at both ends, and a ladder up the hull;
  - *MARISOL* stencilled behind the net;
  - sailors and dockhands behind the cargo netting.
- **The Broker's Club** is the strongest of the three:
  - damask wallpaper over panelling;
  - gilt-framed prize-fight prints and brass sconces;
  - two chandeliers below the HUD band;
  - the city's money behind a red velvet rope;
  - polished herringbone parquet;
  - curtain edges at both ends.
- **Crowds** (all three venues):
  - about 42 different spectators;
  - heads in every 128 px strip in the rest frames;
  - cheering arms the same 4 px width as the accepted v2.1 crowds, with hands, hats and glasses raised.
- **Fronts and HUD bands** (all three): the front layers stay at the stage ends, and the HUD bands stay dark.

## Polish notes (not blocking; for the play-test)

1. **Dry Dock 4's floodlight beam** is a large, coarse dither cone, the loudest thing in the scene. At the right camera end only part of it shows, as a dithered patch at the screen's left edge. A softer, narrower beam would sit better.
2. **Dry Dock 4's sky** has a dither transition band at rows 38–50, inside the top HUD band. At 1× it reads faintly like a line of small text under the health bars. Smoothing it, or moving it below row 52, would remove that.
3. **The Hold is the darkest venue.** The work lamp is only about 5 px, and its pool of light is barely visible, so the warm key light the brief described hardly lands on the crowd or the floor. The hatch and moonlight shaft are not visible. The venue reads; it is just flatter than the others.

If the play-test agrees these matter, they make a small v3B.1 round: the `dry_dock_back`, `dry_dock_mid` and `ship_hold_mid` layers, plus recomposed previews.

## Adoption

- **Fighters:** the corrected frames replace the v3A ones in `ArtSource/Fighters/<Name>/frames/`. The Broker, Sable and Idris are repacked; the manifests are unchanged.
- **Venue layers and previews** go to `Assets/Stages/<Venue>/` unchanged. The `stages.json` notes are updated; there are no data changes.
- **Provenance** (about 0.95 MB) is kept in this folder:
  - the pack's README, provenance, licence and validation;
  - the authoring notes and prompts;
  - the separable people and barrier masters, and the 42 individual spectators per venue.
- **Kept outside the repository** ([ADR-0008](../../docs/adr/0008-repository-and-binary-assets.md)): 20 MB of generated studies and 6 MB of previews.
