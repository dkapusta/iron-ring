# Art Pack v3.2 (crowds): review

**Reviewed:** 2026-11-07, against the [crowd brief](../../docs/art/art-pack-v3.2-crowds-brief.md), with the project tools and in the game.

**Verdict:** adopted as delivered. The crowds now face the ring in all seven venues. One defect remains, and it comes from the brief, not the pack: a band of the **old** spectators survives between the new figures and the barrier. It is the subject of a follow-up, [Art Pack v3.2.1](../../docs/art/art-pack-v3.2.1-crowd-band-brief.md).

## What the pack does

- **Facing the ring.** Every spectator faces the ring:
  - a shadowed face under a lit hat, cap or hood, with at most two muted chin or cheek pixels;
  - a lit collar, lapels, a scarf, an apron or a shirt front;
  - about a third with hands on the rail.
- **Cheering.** Forearms bend across the body, with fists, hats and slips held forward.
- **Backlit rims.** The dotted `sodium_2` rims are gone from the new figures: 0 such edge pixels, against 775–1,946 per sheet before.
- **Per venue.** Clothing and light follow the brief's table. Pier 9 was the reference pass: dockers in pea coats and hard hats, and a child on someone's shoulders.
- **Sheet format.** All 7 sheets:
  - 768 × 672 px, 8 frames of 84 px;
  - alpha 0 or 255 only;
  - 14–20 colours (the limit is 28), **all from the master palette**;
  - heads above row 37 in every 128 px strip of every frame;
  - no figure cut at the sheet's edges.
- **The protected region** is untouched from the brief's rows down, except the hand overlays within 2 rows of it. The art self-test passes.
- **Animation.** The rest and cheer loops close without a jump; idle motion is 1 px. I checked this with the pack's GIFs and in the game.
- **In the game,** at 1× and 3×, all seven venues:
  - the crowd reads as facing the fight;
  - the fighters and sparks stay clearly brighter;
  - the barrier sits where it was.

## The defect, and why

The brief gave each venue's protected "top rail" as **the first row that is opaque across (almost) the whole width**: rows 49, 49, 48, 48, 50, 49 and 54. That row isn't the barrier. It is where the *old* spectators' coats merged into one solid band, 6 to 20 rows above the real barrier:

| Venue | Brief's row | Real barrier | Band left with old bodies |
| --- | ---: | --- | --- |
| Pier 9 | 49 | steel rail, row 60 | rows 49–59 |
| Lantern Row | 49 | rope at rows 57–61, posts | rows 49–60, between the posts |
| The Foundry | 48 | steel rail, row 60 | rows 48–59 |
| The Crown | 48 | parapet, row 60 | rows 48–59 |
| Dry Dock 4 | 50 | posts and a slack chain; solid wall from row 68 | rows 50–67, behind and through the chain |
| The Hold | 49 | chain-link fence; solid from row 70 | rows 49–69, behind the fence |
| The Broker's Club | 54 | posts and a red rope; solid from row 68 | rows 54–67, behind the rope |

The pack followed the brief exactly, so those rows still hold the old lower bodies, **with their dotted rims**. In the game this shows as:
- a dark band of torsos between the new figures and the barrier;
- orange dots in vertical runs, faint at Pier 9 and clearest at The Crown, Dry Dock 4 and The Hold;
- hands on the "rail" at the brief's row, which is mid-torso: they read as hands held at the waist rather than on the barrier.

`docs/art/crowd-reference/crowd_v3.2_legacy_band.png` marks the band in all seven sheets.

**A mechanical fix was tried and rejected.** I tried recolouring the 1 px dots in the band to their neighbours' colours. It caught Lantern Row's post highlights, which are barrier, and missed the `sodium_1` dots and the stacked runs. The band needs to be painted, so it goes back to the artist with the correct rows.

## Checks

- `Checks/project_checks.txt`: sizes, alpha, colours, the protected rows, heads and rims. Off-palette colours: 0 in every sheet.
- `Checks/art_selftest.txt`: `art selftest: ok`. `compose_previews.py` reproduces all seven delivered previews unchanged.
- **Build:** AssetPacker and `ctest` pass. The simulation is unchanged.
- **Provenance:**
  - The pack's `PROVENANCE.md` records an image-generation study for Pier 9. It was used only as a design reference, not shipped.
  - The delivered sheets were built at native resolution from the supplied palette.
  - zlib licence.
