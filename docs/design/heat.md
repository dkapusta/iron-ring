# Heat: what it did, and what it does now

Heat is the 0–100 meter under each health bar.

> **Decision (2026-10-30): options A + B + C, built with the hit-feel pass ([ADR-0018](../adr/0018-hit-feel-and-heat.md)).** The section "Now" below is what the game does; the review that led to it follows.

## Now

| | |
| --- | --- |
| **Gain** | landing a hit or a throw +7; taking one +4; blocking +5; having your attack blocked +2. Capped at 100. |
| **Lifetime** | the whole match: Heat carries into the next round (C). |
| **Spend** | the **Breakaway**, only in blockstun (A): guard + HP + HK, 50 Heat. It ends your blockstun and **staggers the attacker** for 18 ticks without damage, sliding them 24 px: still in reach, so the defender punishes. |
| **Pressure to break** | light attacks are −1 to +2 on block and cancel into the special (B), so a defender can be kept under strings worth escaping. Heavies and specials stay punishable on block. |
| **CPU** | breaks away now and then while it is in blockstun, never in neutral. |

Measured again over 40 CPU-vs-CPU matches (93 rounds): Breakaways roughly doubled, and fighters end rounds with 61–67 Heat on average (from 81–91), now banked for the next round instead of thrown away. The lower gains keep a Breakaway something to save for.

## Review (before the change)

Against the M2 code and the shipped fighter data, as it was:

### The rules as built

| | |
| --- | --- |
| **Gain** | landing a hit or a throw +12; taking a hit or being thrown +6; blocking +8; having your attack blocked +3. Capped at 100. |
| **Spend** | only on the **Breakaway**: guard + HP + HK, 50 Heat, within 120 px of the opponent. It cancels blockstun and shoves the opponent 55 px away; no damage. |
| **Lifetime** | one round. `Round::reset()` starts both fighters at 0 every round. |
| **Desperation / supers** | none, by decision (2026-10-29). |

### What it actually did

Heat has a role on paper, but it hardly matters in play.

1. **There is no block pressure to escape.** Every attack has a blockstun of 7 ticks, so on block the attacker is far behind. At best (blocked on the first active frame) the attacker is 5 to 23 ticks behind:

   | Move | On block |
   | --- | --- |
   | Dee Stiff Jab | −5 |
   | Dee Cross Counter | −15 |
   | Dee Shin Kick | −11 |
   | Dee Leg Sweep | −20 |
   | Dee Rush Uppercut | −16 |
   | Bram Palm Shove | −5 |
   | Bram Anchor Hook | −18 |
   | Bram Boot Kick | −11 |
   | Bram Hull Breaker | −23 |

   A blocking player is already free to act; there are no chains or cancels to keep them locked down.
2. **The Breakaway usually hurts its user.** It pushes the attacker out of range at the exact moment the defender could punish, and charges 50 Heat for that.
3. **Heat is mostly wasted.** In 40 CPU-vs-CPU matches (86 rounds), fighters ended a round holding 81–91 Heat on average. Breakaways came to about 0.6 per round, and only because the CPU presses it on a timer. Whatever is left is discarded at the next round.

In short: the meter fills quickly, sits near full, and its only outlet is rarely worth taking. The crowd now reacts to big hits (stage crowds, M2). That reaction is presentation only and doesn't involve Heat.

### Options considered

All of these keep the "no supers" decision; Heat still pays only for the Breakaway.

- **A. Make the Breakaway a real guard cancel.**
  - Usable only in blockstun.
  - It staggers the attacker (about 14 ticks of no-damage stun) instead of pushing them away, so the defender wins the exchange instead of resetting it.
- **B. Give some moves pressure on block.**
  - Light attacks around −1 to +1 on block (more blockstun), so there are strings worth escaping.
  - Without this, even option A has little to cancel.
- **C. Keep Heat across rounds** (reset per match). It becomes a resource you save for the deciding round instead of a bar that refills for free.
- **D. Leave it as is** and treat Heat as flavour until the hit-feel pass (counter hits, combos), which will change block and hit advantage anyway.

**Chosen:** A + B + C.
