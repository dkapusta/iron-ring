# Iron Ring: final art polish brief (Art Pack v3.1)

**For:** whoever produced Art Pack v3.
**From:** the Iron Ring project, 2026-11-01.
**With:** the current Iron Ring source package, which holds every production asset: Art Packs v1, v2, v2.1, v3A and v3B. Every path below is relative to its root.

All production art is in the game, and the owner has accepted it. This is the **final polish round**, from a review of the whole set side by side: seven fighters, fourteen portraits, seven venues and the front end. It mostly makes the later packs consistent with the earlier ones. Nothing new is drawn, and no game data changes.

The rules of the [Art Pack v3 brief](art-pack-v3-brief.md) §2 still apply, and so do the canvas, origin, scale, outline and palette limits of every fighter. For each fighter, keep:
- every pixel of the silhouette's alpha;
- the throw partner offsets;
- the feet.

---

## 1. What to deliver

| # | Item | Files | Priority |
| --- | --- | --- | --- |
| P1 | **One portrait backdrop for everyone** (§2) | all 7 `<id>_portrait.png` and the 6 `<id>_select.png` | Required |
| P2 | **Greta's skin**, out of the apron ramp (§3) | all 55 frames of Greta, `greta_portrait.png`, `greta_select.png` | Required |
| P3 | **Jun's skin and hair**, out of the shoe ramp (§4) | all 55 frames of Jun, `jun_portrait.png`, `jun_select.png` | Required |
| P4 | **The chain ring on the three M3 floors** (§5) | `dry_dock_floor.png`, `ship_hold_floor.png`, `brokers_club_floor.png`, and the three previews | Required |
| P5 | Dry Dock 4: a softer floodlight beam and a clean sky band (§6) | `dry_dock_mid.png`, `dry_dock_back.png`, `dry_dock.png` | Recommended |
| P6 | The Hold: a work lamp that lights the hold (§7) | `ship_hold_mid.png` (both frames), `ship_hold.png` | Recommended |
| P7 | Specks and stubs (§8) | `dee_select.png`, `bram_select.png`, Sable `walk_3.png` | Recommended (P7a is free once P1 is done) |

**Not wanted:**
- any other frame or layer;
- the title logo or the menu backdrop;
- effects, fonts or the HUD;
- atlases (the project repacks them);
- JSON or code.

The four M2 venues and the Broker are not touched, apart from their portraits under P1.

---

## 2. P1: one portrait backdrop

**Problem.** The portraits use two different backdrops. You can see it on the character select, where the six portraits sit side by side, and in the HUD corners:

| Portraits | Backdrop now |
| --- | --- |
| Dee, Bram (Art Pack v2) | `night_1` with a soft, low-contrast smudge of grey-blue in the upper left (`#201f28`, `#24232c`, `#2a2730`). Smooth, but not dithered. |
| Sable, Idris, Greta, Jun, the Broker (v3A) | `night_1` with a regular grid of single dots, 2 px apart, in **skin-brown** (`#4e2f1e`, `#6a4030`). It reads as a halftone screen or noise at 1×, and its colour belongs to the face, not the room. |

**Fix.** Put every portrait (7 HUD and 6 select) on **the same backdrop**. The project supplies it, so it is identical everywhere:

- `docs/art/templates/portrait_backdrop_48.png` for the select portraits;
- `docs/art/templates/portrait_backdrop_32.png` for the HUD.

Both templates are built the same way:
- three colours: `night_1` `#141a30`, `night_2` `#1f2742` and `#2a2730`;
- a glow from the upper left, ordered-dithered with a 4 × 4 Bayer matrix and fading out by about 60 % of the width;
- low contrast, so it reads as smooth at 1×.

- **Use the template exactly.** Every pixel outside the figure's outline is the template's pixel at that position. No other colours, dots or texture go in the backdrop.
- **The figure stays as it is.** Change only backdrop pixels, never the face, hair, clothes or the outline.
- **The side-colour band** at the bottom of the select portraits stays: teal, rust, violet, muted blue, brass, jade.
- **Dee and Bram are included,** so all seven match; their smudge goes.

**Check.** On the select screen, all six cells have the same corner. In the HUD, both portraits of any pairing do too.

---

## 3. P2: Greta's skin

**Problem.** Greta's face, ears, neck and arms are drawn with her **apron ramp** (`#c4893a`, `#9a6228`). It is the same fault as the Broker's skin in v3A, which v3A.1 fixed. Her skin ramp (`#94604a` and below) appears only in the shadows.
- Her skin and her brass-brown apron have the same hue, so the apron's edge disappears into her arms.
- At 1× she reads as gilded or orange.
- It is in all 55 frames: in the top 20 rows of each figure, which is mostly the head, apron tones cover 46–800 pixels, with a median of 109. The portraits have it too.

**Fix.** Recolour the skin only, in every frame and both portraits, to her skin ramp from `Tools/Art/ironart/palette.py`:

```text
#2a1810  #6a4030  #94604a  #b88468  #d6a88a
```

A weathered woman in her fifties, lit warm from the upper left. Slight shifts are allowed.

- **Keep the apron** exactly in its brass-brown ramp. After the fix, skin and apron must read as two materials.
- **Unchanged:** the hair, shirt, trousers, gloves, boots and the chain in the champion frames.
- **Colour count:** within 48 (37 now).
- **Method.** As K1 was done for the Broker: a masked recolour of the skin areas only. Protect the apron pixels; where the two ramps meet, the region decides, not the colour.

---

## 4. P3: Jun's skin and hair

**Problem.** Jun's skin is lit with his **shoe red** (`#c44a38`, `#9c2a22`, `#4a1010`), on his face, neck and forearms. His hair takes the same red-brown.
- At 1× his face looks sunburnt or streaked, and on the portraits the red reads as cuts.
- It is in 54 of 55 frames.
- The brief asked for messy **black** hair; it reads auburn.

**Fix.** In every frame and both portraits:

- **Skin** to his skin ramp:

  ```text
  #1e120c  #4a2e1e  #74503a  #9a7052  #bf9474
  ```

- **Hair** to his hair ramp, black with cool highlights:

  ```text
  #050507  #0e0e14  #1a1a24  #282838  #3c3c52
  ```

- **The red stays only on the high-tops.**
- **Unchanged:** the jade jacket, the white wrist tape and the trousers.
- **Colour count:** within 48.

---

## 5. P4: the chain ring on the M3 floors

**Problem.** The chain ring is the series' signature (asset spec §3), and the four M2 floors draw it as **distinct welded links**: lit pale ovals and bars you can count at 1×. The three v3B floors draw it as a thin, dark, broken line that disappears against the floor. Measured in the ring's band (rows 60–68 of each floor layer), the 95th-percentile brightness against the floor's median is:

| Floor | Ring band, 95th percentile | Floor median | Reads as |
| --- | ---: | ---: | --- |
| Pier 9 | 63 | 17 | links |
| Dry Dock 4 | 57 | 17 | a faint line, its links lost in the reflections |
| The Hold | 40 | 27 | a thin dark line |
| The Broker's Club | 52 | 30 | a thin dark line in the parquet |

**Fix.** Redraw the ring on the three floors to match Pier 9's and The Foundry's:
- **Shape:** the same flattened ellipse, centred at x 400, y 64 of the layer (world x 400, y 326), about 564 × 48 px.
- **Links:** welded, alternating flat (an open oval, about 7 × 4 px) and edge-on (a bar, about 5 × 2 px).
- **Colours:** the shared `metal` ramp (`#0b0d12`, `#3d4a66`, `#5a6782`, `#8a93a8`, `#d5dbe6`), each link lit on its upper-left edge by the venue's key light.
- **Lighting by venue:**
  - Dry Dock: the sodium floodlight.
  - The Hold: the work lamp.
  - The Club: chandelier gold.
- **Contact with the floor:** the links sit *on* the floor with a 1 px contact shadow, under any reflections.
- **Contrast:** the ring stays darker than the fighters' brightest tones and never reaches the HUD band (it is far below it).

Change nothing else in the floor layers. Recompose the previews afterwards (§9).

---

## 6. P5 (recommended): Dry Dock 4

1. **The floodlight beam** (`dry_dock_mid.png`, about x 61–222 and y 75–217) is a large, coarse cone of bright dither, the loudest thing on screen. At the right end of the camera travel, only part of it shows, as a bright patch at the screen's left edge.
   - Make it narrower and dimmer, about half the bright area: the lamp head bright, the beam a faint dithered wedge that fades well before the crowd.
   - Keep the tower and the lamp where they are.
2. **The sky** (`dry_dock_back.png`, rows 38–50) has a dither transition band inside the top HUD band. At 1× it reads faintly like a line of small text under the health bars.
   - Move the transition below row 52, or make it irregular (no horizontal run of the same pattern).
   - Stay dark (the HUD band's rule).

---

## 7. P6 (recommended): The Hold's lamp

The work lamp in `ship_hold_mid.png` is about 5 px, and its light hardly lands: the whole layer has fewer than 90 bright pixels. The Hold is the flattest venue as a result.

- **The lamp:** a caged work lamp about 10–12 px tall on its cable, with a hot centre.
- **The light pool:** a dithered, warm pool under it that reaches the netting, the near crates and the tops of the crowd's heads. Stop it short of the floor layer; the floor is a separate file.
- **Both frames:** keep the swing; the lamp and its pool move together. Everything else stays identical in both frames.
- **Optional:** brighten the moonlight shaft under the hatch by one tone, if it helps the cool fill.

---

## 8. P7 (recommended): specks and stubs

| File | Fix |
| --- | --- |
| `dee_select.png` | A single near-white pixel on her right shoulder (about x 38, y 37). Shade it into the skin. |
| `bram_select.png` | A single near-white pixel on his scalp (x 20, y 6). Use a skin highlight tone, not white. |
| Sable `walk_3.png` | One or two stray outline pixels beside each ankle, left from the v3A.1 leg repair. Remove them. |

The two select-portrait specks can be fixed while doing P1.

---

## 9. Check before delivering

With the source package:

1. **Put the files in place** under `ArtSource/Fighters/<Name>/frames/` and `Assets/`.
2. **Repack Greta and Jun** (and Sable, if P7 is done), then recompose the previews:

   ```sh
   python3 Tools/Art/import_art.py fighter greta ArtSource/Fighters/Greta/frames --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Greta/frames/manifest.json
   python3 Tools/Art/import_art.py fighter jun   ArtSource/Fighters/Jun/frames   --origin 128 232 --scale 1 --no-quantize --manifest ArtSource/Fighters/Jun/frames/manifest.json
   python3 Tools/Art/compose_previews.py
   ```

3. **Tools:** `python3 Tools/Art/review_frames.py greta jun sable` still reports 0 findings, and `python3 Tools/Art/selftest.py` ends with `art selftest: ok`.
4. **Portraits:** every pixel outside each figure equals the template (§2). A diff against `docs/art/templates/` should show only the figure.
5. **Skin colours:**
   - Greta's head and arms use no apron colours, and Jun's face, neck and arms no shoe colours.
   - Count them per frame; the counts should be 0 outside the apron and the shoes.
6. **Look at 1× and 4×:** Greta next to Dee and the Broker, and Jun next to Sable. The three floors next to Pier 9's should read as the same chain.

## 10. Delivery

One ZIP, **`IronRing_ArtPack_v3.1_Polish.zip`**, with paths that mirror the repository:

```text
IronRing_ArtPack_v3.1_Polish/
  README.md                         per item: what changed; anything not done and why
  CONTENTS_SHA256.txt
  Assets/Fighters/<Name>/<id>_portrait.png, <id>_select.png      (P1–P3, P7)
  ArtSource/Fighters/Greta/frames/*.png                          (P2, all 55)
  ArtSource/Fighters/Jun/frames/*.png                            (P3, all 55)
  ArtSource/Fighters/Sable/frames/walk_3.png                     (P7, if done)
  Assets/Stages/DryDock/dry_dock_floor.png, _mid.png, _back.png, dry_dock.png
  Assets/Stages/ShipHold/ship_hold_floor.png, _mid.png, ship_hold.png
  Assets/Stages/BrokersClub/brokers_club_floor.png, brokers_club.png
  Previews/                         before/after at 4× per item; the select screen's six portraits
                                    side by side; the seven floors stacked
```

- **Don't include:** atlases, manifests, code, JSON or unchanged files.
- **Licence:** the same terms as v3 (zlib). Add a `PROVENANCE.md` only if tools or inputs differ from v3B.
