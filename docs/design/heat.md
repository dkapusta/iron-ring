# Heat: what it does today, and what it could do

Review, 2026-10-30, against the M2 code (`Source/Simulation/Round.cpp`) and the shipped fighter data. Heat is the 0–100 meter under each health bar.

## The rules as built

| | |
| --- | --- |
| **Gain** | landing a hit or a throw +12; taking a hit or being thrown +6; blocking +8; having your attack blocked +3. Capped at 100. |
| **Spend** | only on the **Breakaway**: guard + HP + HK, 50 Heat, within 120 px of the opponent. It cancels blockstun and shoves the opponent 55 px away; no damage. |
| **Lifetime** | one round. `Round::reset()` starts both fighters at 0 every round. |
| **Desperation / supers** | none, by decision (2026-10-29). |

## What it actually does

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

## Options (for the project owner)

All of these keep the "no supers" decision; Heat still pays only for the Breakaway.

- **A. Make the Breakaway a real guard cancel.**
  - Usable only in blockstun.
  - It staggers the attacker (about 14 ticks of no-damage stun) instead of pushing them away, so the defender wins the exchange instead of resetting it.
- **B. Give some moves pressure on block.**
  - Light attacks around −1 to +1 on block (more blockstun), so there are strings worth escaping.
  - Without this, even option A has little to cancel.
- **C. Keep Heat across rounds** (reset per match). It becomes a resource you save for the deciding round instead of a bar that refills for free.
- **D. Leave it as is** and treat Heat as flavour until the hit-feel pass (counter hits, combos), which will change block and hit advantage anyway.

**Recommendation:** A + B + C, folded into the M2 hit-feel pass. That pass already changes the simulation (counter hits, combo counts) and will rebaseline the golden checksum once. The CPU then spends Heat when it is actually blocking a string, not on a timer.
