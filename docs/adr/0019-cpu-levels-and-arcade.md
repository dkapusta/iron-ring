# ADR-0019: A stateful CPU with five levels; an arcade ladder as data

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

The CPU was `sim::cpuInput`, a pure function of the round with timer-based habits. It couldn't react late or early, combo, anti-air, or vary between matches, and it had one strength. M2 asks for an arcade ladder with CPU difficulty levels.

## Decision

**The CPU (`Simulation/Cpu`)**
- `sim::CpuPlayer` is a stateful input source in the simulation library. It reads only the round and draws randomness from its own xorshift32 generator, seeded when it is created, so a match replays exactly from its seed.
- It is not part of `Round` or the checksum: like a player, it only produces inputs. `cpuInput` stays as the simple scripted opponent used by older tests.
- Five levels: ROOKIE, BRAWLER, CONTENDER, VETERAN, CHAMPION.
- Each level is a profile (`CpuProfile`). It sets:
  - reaction time to an attack it sees start (24 → 7 ticks) and the time between neutral decisions;
  - how often it blocks, punishes a whiff or block, anti-airs, throws, techs and breaks away;
  - how often it cancels a connecting light attack into the special, and whether it confirms the hit first (VETERAN and up);
  - its aggression.
- Fighters keep a style: a grappler (special is a command throw) wants to stand in throw range and walks in under pokes; a striker plays at the tip of its light kick.
- A test pins the ranking: each level beats the level two below it in at least 75 % of matches over several seeds, both sides.
- Measured with the shipped fighters (row level against column level):

  | | vs ROOKIE | vs BRAWLER | vs CONTENDER | vs VETERAN | vs CHAMPION |
  | --- | --- | --- | --- | --- | --- |
  | **ROOKIE** | 50 | 2 | 0 | 0 | 0 |
  | **BRAWLER** | 97 | 50 | 0 | 0 | 0 |
  | **CONTENDER** | 100 | 100 | 50 | 10 | 0 |
  | **VETERAN** | 100 | 100 | 89 | 50 | 14 |
  | **CHAMPION** | 100 | 100 | 100 | 85 | 50 |

**Difficulty**
- `cpu-level` (Easy / Normal / Hard; Options, `prefs.cfg`, `-cpu-level`) sets the Versus CPU and training-dummy opponent: BRAWLER, CONTENDER or VETERAN.
- It also shifts every arcade bout by one level, down or up.

**The ladder is data.**
- `Data/arcade.json` lists the bouts: a stage, an opponent (`rival`, `mirror` or a fighter id) and a level at Normal; the last bout is marked `final`.
- AssetPacker checks the stages and the opponents.
- Today it has four bouts: Pier 9 (rival, BRAWLER), Lantern Row (mirror, BRAWLER), The Foundry (rival, CONTENDER), and The Crown (rival, VETERAN), the final.
- New fighters join the ladder with a line each in M3.

**Flow (`ArcadeScene`).**
1. A character select with one pick, showing the ladder for the fighter under the cursor.
2. A versus card before each bout.
3. The fight, which reports its winner back through `FightSetup::onFinished`.
4. On a loss, a 9-second CONTINUE countdown: a continue replays the same bout, and running out ends in GAME OVER.
5. After the final bout, the ending ("… TAKES THE CROWN", continues used), which returns to the title after 30 s, like a cabinet.

The run's rules are `content::ArcadeRun`, pure and unit-tested.

**Soak test.** `-autoplay` hands player 1 to a CHAMPION CPU. CI runs the whole ladder in batch mode at 640 × 360 (about 45 s) and keeps the ending frame.

## Consequences

- Rematches differ, because the CPU is reseeded per match, while staying deterministic.
- A recorded online or replay input stream doesn't need the CPU, only its inputs.
- The CPU knows a fighter only through its data (move reaches, startups, special kind), so new fighters get a usable CPU with no code. Bespoke behaviour would need a profile per fighter, which nothing needs yet.
- The ladder is short with two playable fighters (rival and mirror bouts). It grows with the roster.
- Score, a "new challenger" join-in and story endings are not here. Story and endings are M3.

## Alternatives considered

- **Keep a pure CPU function of the round:** no memory of what it has seen, so no reaction time and no plans (motions, combos) across ticks.
- **Difficulty as damage or health multipliers:** it feels unfair, and the fighters' frame data would no longer mean what the move list says.
- **The ladder in code:** every venue or roster change would need a rebuild; the stage and fighter catalogues are already data.
