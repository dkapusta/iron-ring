# ADR-0018: Counter hits, combos and cancels; the Breakaway as a guard cancel; Heat for the match

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

Hits had one weight whatever the situation, and nothing chained. The [Heat review](../design/heat.md) found the meter near full and nearly useless: every attack was 5–23 ticks minus on block, and the Breakaway pushed the attacker out of punish range. The owner chose options A + B + C of that review. The M2 roadmap also asks for a hitstop, shake and flash pass, HDR-aware sparks, and hit-confirm audio layering, which needs counter hits and combo counts in the simulation.

## Decision

**Simulation** (deterministic; every new field is in the checksum):

- **Counter hits.** A strike that lands while the opponent's own attack is in startup or active frames is a counter hit, unless both hit on the same tick (a trade). It does 125 % damage, +6 ticks of hitstun and +4 ticks of hitstop.
- **Combos.** `Fighter::combo` counts the hits taken while still in hitstun, and resets when the fighter recovers.
  - Damage scales 100 / 85 / 70 / 55 %, then 50 % for every later hit, so long strings don't delete a health bar.
- **Cancels.** A move marked `"cancel": true` that connected (hit or block) can be cancelled into the fighter's special during its active frames plus 6 ticks. The light normals of Dee and Bram carry it.
- **(B) Pressure.**
  - Light attacks have real blockstun: jabs 11, light kicks 17. They are now −1 to +2 on block.
  - Heavies and sweeps get 10–12 and stay clearly punishable.
- **(A) Breakaway = guard cancel.** It is only possible in blockstun, still guard + HP + HK for 50 Heat. It ends the blockstun and staggers the attacker: 18 ticks of hitstun, no damage, a 24 px slide. The defender gets a punish.
  - Outside blockstun the chord is just a heavy attack.
  - The CPU uses it only while blocking.
- **(C) Heat lasts the match.** `Match` carries each fighter's Heat into the next round. A new match starts at 0.
  - The gains are lower (hit +7, hit taken +4, block +5, blocked +2) so 50 Heat is something to save for.
- `StepEvents` items gain `attacker`, `combo` and `counter`, for presentation.
- The golden checksum is rebaselined once for all of this. The history is in the test.

**Presentation** (reacts to events only):

- **HUD callouts:** "N HITS" for combos of two or more, COUNTER and BREAKAWAY on the side that did it.
- **Audio layering:** the hit sound, plus a counter sting, plus a combo blip whose pitch rises 12 % per hit (up to 1.7×), using `MIX_SetTrackFrequencyRatio`.
- **Shake:** a horizontal world shake that keeps counting through hitstop, so it sells the freeze. The HUD stays still. Amplitudes:

  | Event | Shake |
  | --- | --- |
  | Heavy hit | 2 px |
  | Counter heavy hit | 3 px |
  | Slam | 4 px |
  | K.O. | 5 px |
  | Breakaway | 2 px |

- **Flash:** a short additive white wash on counter hits and K.O.s.
- **Screen effects option:** `screen-effects=full|reduced`, in Options and `prefs.cfg`. Reduced caps the shake at 1 px and turns flashes off.
- **HDR sparks:** on HDR output, effects get a second, additive pass whose strength follows the display's headroom, so sparks glow above SDR white. SDR output is unchanged.
  - Float textures were not needed: the sources are 8-bit pixel art, and the colour scale already reaches above 1.

## Consequences

- Light-attack pressure exists, and the Breakaway is the answer to it. Saving Heat across rounds is a choice.
- Training (F3) shows the frame data; the new on-block numbers are in [controls.md](../controls.md).
- Balance now depends on more numbers: blockstun, cancel flags, scaling, Heat gains. They all live in the fighter files or in named constants in `Round.h` and `Round.cpp`.
- The CPU still doesn't use cancels or specials. Arcade difficulty levels (the M2 arcade ladder) will have to.

## Alternatives considered

- **Chains (light into light) instead of special cancels:** more input-heavy for an early-90s feel. Special cancels are the genre's classic first step.
- **Counter hits on any hit during the opponent's recovery:** that punishes whiffs twice; recovery is already the punish window.
- **Vertical shake:** every world-space draw would need a y offset. Horizontal shake reads well at 640 × 360.
