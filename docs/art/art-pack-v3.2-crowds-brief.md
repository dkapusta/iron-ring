# Iron Ring: crowd art brief (Art Pack v3.2, crowds)

**For:** whoever produced Art Packs v3 and v3.1.
**From:** the Iron Ring project, 2026-11-07.
**With:** the current Iron Ring source package. Every path below is relative to its root. This brief has two reference images, `crowd_now.png` and `crowd_reference_spectator.png`. They come next to it in the brief's ZIP, and in the repository under `docs/art/crowd-reference/`.

One problem, in all seven venues: **the crowds look like they are standing with their backs to the fight.** This pass turns every spectator round to face the ring, with faces kept in shadow. Nothing else changes: no fighter, floor, wall, effect, font, HUD or game data.

The rules of the [Art Pack v3 brief](art-pack-v3-brief.md) §2 and the [asset specification](asset-spec.md) §3 still apply.

---

## 1. What to deliver

| # | Item | File | Priority |
| --- | --- | --- | --- |
| C1 | Pier 9 | `Assets/Stages/PierNine/pier_nine_crowd.png` | Required |
| C2 | Lantern Row | `Assets/Stages/LanternRow/lantern_row_crowd.png` | Required |
| C3 | The Foundry | `Assets/Stages/Foundry/foundry_crowd.png` | Required |
| C4 | The Crown | `Assets/Stages/TheCrown/the_crown_crowd.png` | Required |
| C5 | Dry Dock 4 | `Assets/Stages/DryDock/dry_dock_crowd.png` | Required |
| C6 | The Hold | `Assets/Stages/ShipHold/ship_hold_crowd.png` | Required |
| C7 | The Broker's Club | `Assets/Stages/BrokersClub/brokers_club_crowd.png` | Required |
| C8 | The seven stage previews, recomposed (§7) | `Assets/Stages/<Dir>/<id>.png` | Required (generated) |

**Not wanted:** any other layer; fighters, portraits, the menu backdrop, effects, fonts or the HUD; JSON or code.

---

## 2. The problem

`crowd_now.png` shows a 200 px strip of every crowd layer at 3×: a rest frame on the left and a cheering frame on the right. What makes them read as backs:

1. **A rim of light all round.** Each figure has a dotted 1 px outline in `sodium_2` (`#d9822b`) on both shoulders, both sides and over the head. Light on every edge and none on the front means the light is behind the figure, so the figure must be facing away. That is the strongest cue, and it is on every spectator in every venue: 1,100 to 4,100 such pixels per sheet.
2. **No front planes.** The coats are one flat dark mass. There are no lapels, collars, scarves or ties, no shirt at the neck, no buttons and no hands, which are the things you only see from the front.
3. **Heads are blank ovals or hat shapes,** symmetric, with nothing to show which way they face.
4. **Raised arms are thin sticks** from the outer edge of the body, with the hand at the top, as if seen from behind. Arms raised by someone facing us cross in front of the body or the head, and the fists face us.
5. **Nobody leans in.** Everyone stands bolt upright and evenly spaced, like a queue, not a crowd watching a fight.

---

## 3. What the crowd should look like

People **pressed against the barrier, facing the ring, lit by the ring's own light.** The faces sit under hat brims, hoods and the dark, so they stay anonymous, but everything else says *facing us*. See `crowd_reference_spectator.png` (a schematic at 12×, not final art).

### 3.1 Light

- **The key light is the fight's light:** the same venue light that falls on the fighters, from in front of the crowd (the ring side) and above. It lands on surfaces that face the ring and the sky:
  - the top edges of hats, brims and caps;
  - the tops of the shoulders;
  - the front of collars and lapels;
  - hands on the barrier;
  - a chin, or a nose tip.
- **Faces are in shadow.** Under the brim, the hood or the hair, the face is `night_0` (`#0c1020`) to `ink` (`#07090f`).
  - At most **1–2 px of lit skin** per face: a chin, a nose tip or a cheekbone, from the venue's skin-safe ramp.
  - **No eyes, no teeth, no white pixels** in a face.
- **Back-light only where a venue really has a light behind them,** as a minor accent: on one side of the figure only, at most 1 px, broken (not a continuous dotted outline). Never all round.
- **Ramps:** spectators use the venue's existing colours: the night ramp, the sodium ramp and muted clothing ramps from `Tools/Art/ironart/palette.py`. `sodium_2` (`#d9822b`) is the hottest tone and goes on a few top edges per figure, not on outlines. `sodium_3` and `sodium_4` only on lit hands or hats, if at all.

### 3.2 Figures

- **Front cues, at least two per figure.** Keep them simple: these are 6–10 px wide people.
  - a collar or lapel V, a lighter shirt, a scarf or a tie at the neck;
  - a coat opening down the front, or a button line;
  - **hands on the top rail** (the strongest cue, on about a third of the figures);
  - a cigarette ember at the mouth (1 px of `sodium_3`);
  - a betting slip or a folded newspaper held up;
  - a bookmaker's chalk board facing out.
- **Heads:**
  - **Most face the ring**, square on or in three-quarter view turned toward the layer's centre (x 384). People left of the centre turn right, people right of it turn left.
  - A three-quarter head shows its shadowed side and a lit cheek or ear on the near side.
  - **Some lean in**, head forward and lower than the shoulders.
- **Arms, cheering:**
  - fists or open hands raised **in front** of the body or the head, forearms crossing the silhouette;
  - hats and caps waved;
  - two arms up on a few figures only.
  - No stick arms from the outer edge.
- **A crowd, not a queue.** Uneven spacing, overlapping shoulders, two rows where there's depth, mixed heights: tall dockers, short market women, a child on someone's shoulders at Pier 9 or Lantern Row.
  - Keep **head and shoulders above the barrier along the whole width,** in every frame (the art self-test checks the top rows).
- **Who they are,** from the setting bible: dockers, market traders, bookmakers with chalk boards, foundry night shift, and a few well-dressed spectators who obviously aren't from the docks. At the Broker's Club it's the reverse: mostly money (suits, furs, hats), with a few hired heavies. Work clothes and street clothes only: no uniforms, no other franchise's looks.

### 3.3 Per venue

| Venue | Key light on the crowd | Back-light allowed | Crowd |
| --- | --- | --- | --- |
| Pier 9 | The freighter's floodlights and the sodium lamps, warm from the ring side | Cool water glow from below and behind, very sparse | Dockers in caps and pea coats, a few in hard hats |
| Lantern Row | Half-lit lanterns overhead | None | Traders, aprons, shawls, a cook in whites. The visible band is the full 84 px here, the tallest, so use two rows |
| The Foundry | The work lights over the rail | Faint furnace orange from one end | The night shift: overalls, flat caps, leather aprons |
| The Crown | Dawn: cool pink sky light from above, a warm fill from the ring | Yes, the dawn sky is behind them: a thin cool top edge on heads and shoulders, one side | Late-night gamblers in coats, a few in evening wear |
| Dry Dock 4 | The sodium floodlight tower (to the left in the layer) | None | Dockers and riggers in rain gear |
| The Hold | The swinging work lamp: a warm pool on the heads of the figures under it | None | Sailors, crew, a few in oilskins |
| The Broker's Club | Chandelier gold from above | None | Suits, furs, homburgs, cigarettes. The heavies stand at the rope's ends |

---

## 4. What must not change

The crowd is drawn in the engine as a layer. These are hard rules, and the art self-test and the AssetPacker check several of them:

| Rule | Value |
| --- | --- |
| **Sheet size** | 768 × 672 px, RGBA (alpha 0 or 255 only, no partial alpha) |
| **Frames** | 8, stacked vertically, 84 px each. Frames 0–3 are the crowd **at rest** and loop; frames 4–7 are the crowd **cheering** and loop. The engine bobs between them faster as excitement rises. |
| **One person, one place** | Each spectator keeps the same position, body and colours in all 8 frames. Frames differ only by small motion: a head bob, a shoulder shift, a lean; in the cheer frames, arms and hats. |
| **The barrier** | Keep it **pixel-identical** to the current sheet, from the barrier's top rail down, in every frame. The only exception: hands may rest on the top rail and cover it. The top rail is the first row that is opaque across (almost) the whole width: row 49 at Pier 9, 49 at Lantern Row, 48 at the Foundry and The Crown, 50 at Dry Dock 4, 49 at The Hold and 54 at the Broker's Club. |
| **Spectators stay behind the barrier** | No body below the top rail, except hands on it. |
| **Heads above row 37** | All along the width, in every rest frame: at least one opaque pixel above row 37 in every 128 px strip (self-test `check_crowds`). Cheer frames may go higher; nothing above row 2. |
| **The visible band** | The floor layer covers the lower part of the sheet on screen. Visible crowd rows per frame: Pier 9 0–63, Lantern Row 0–83, Foundry 0–75, Crown 0–77, Dry Dock 0–75, Hold 0–75, Club 0–75. Spend the detail above those rows. |
| **Colours** | Venue palette only. At most **28 colours** per sheet (17–23 now). No new hues. |
| **Contrast** | The crowd stays **darker and lower in contrast than the fighters**: its brightest tone at most `sodium_3`, and only on a few pixels per figure. The fighters must still pop in front of it. |
| **Seams** | The layer doesn't drift, so its edges don't need to tile. Don't cut a figure at x 0 or x 767 halfway through the head. |

---

## 5. Animation

- **At rest (frames 0–3):** a restless loop: a few people shift weight, one turns their head, a cigarette glows, a hat tips. Not everyone moves in every frame, about a third per frame. 1 px moves only.
- **Cheering (frames 4–7):**
  - about half the figures raise arms, fists forward, cycling over the four frames;
  - hats and slips are waved;
  - the rest lean in or jump 1–2 px.
- **Both loops are seamless:** frame 3 → 0 and 7 → 4. Each frame shows for 14 ticks (0.23 s), faster when the crowd is excited.

---

## 6. Method

- **Repaint over the existing figures, or replace them;** either is fine, as long as §4 holds. Pixel-art by hand, or a generator followed by a hand cleanup to the palette and the grid. No smoothing, anti-aliasing or partial alpha.
- **Do one venue first** (Pier 9: it is the signature venue and the cover), and include its before and after in the delivery notes. It is the reference for the other six.
- **Keep the venues distinct** in clothing and light (§3.3), but keep the construction the same everywhere: lit top planes, shadowed faces, front cues and hands.

---

## 7. Check before delivering

With the source package:

1. **Put the seven sheets in place** under `Assets/Stages/<Dir>/`.
2. **Recompose the previews.** The stage select, the title and the ending use frame 0 of every layer:

   ```sh
   python3 Tools/Art/compose_previews.py
   ```

3. **Self-test.** `python3 Tools/Art/selftest.py` must end with `art selftest: ok`. It checks the crowd head rows, and that each preview matches its layers.
4. **In the game.** Build (`cmake --preset release && cmake --build --preset release`), then capture each venue at 1× and 3×:

   ```sh
   cd Build/release/Player/Release
   ./IronRing.x86_64 -batchmode -demo -stage pier_nine -fighters dee,bram -frames 960 \
       -screen-fullscreen 0 -screen-width 1920 -screen-height 1080 -screenshot pier_nine.png
   ```

   Stage ids: `pier_nine`, `lantern_row`, `foundry`, `the_crown`, `dry_dock`, `ship_hold`, `brokers_club`. The same command always gives the same frame. Look at:
   - **The read:** at 1×, from arm's length, the crowd faces the ring.
   - **Contrast:** the fighters still stand out in front of it.
   - **The barrier** looks the same as before.
5. **Measure:**
   - per sheet, the count of `sodium_2` (`#d9822b`) pixels on outer silhouette edges should be **near zero**;
   - no figure is lit on both sides;
   - no face has a pixel brighter than its venue's skin mid-tone.

---

## 8. Delivery

One ZIP, **`IronRing_ArtPack_v3.2_Crowds.zip`**, with paths that mirror the repository:

```text
IronRing_ArtPack_v3.2_Crowds/
  README.md               per venue: what changed; anything not done and why
  CONTENTS_SHA256.txt
  Assets/Stages/PierNine/pier_nine_crowd.png, pier_nine.png
  Assets/Stages/LanternRow/lantern_row_crowd.png, lantern_row.png
  Assets/Stages/Foundry/foundry_crowd.png, foundry.png
  Assets/Stages/TheCrown/the_crown_crowd.png, the_crown.png
  Assets/Stages/DryDock/dry_dock_crowd.png, dry_dock.png
  Assets/Stages/ShipHold/ship_hold_crowd.png, ship_hold.png
  Assets/Stages/BrokersClub/brokers_club_crowd.png, brokers_club.png
  Previews/
    <id>_crowd_before_after_4x.png    frame 0 and frame 4, before above after, per venue
    <id>_ingame_1x.png                the §7 capture, per venue
```

- **Don't include:** other layers, atlases, JSON, code or unchanged files.
- **Licence:** the same terms as v3 (zlib). Add a `PROVENANCE.md` only if the tools or inputs differ from v3.1.
