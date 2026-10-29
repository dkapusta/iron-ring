# ADR-0006: Deterministic integer simulation, ported from the concept kit

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The concept kit ships a compact combat prototype (`combat.hpp/.cpp`) with tests: movement, jump, four attacks, standing/crouch guard, one hit per move, simultaneous trades, hitstop, six-tick input buffer, Current Break (now Breakaway) from blockstun, timer/KO, a deterministic CPU, and an FNV-1a state checksum. Rollback netcode (M4) needs bit-exact determinism from the start; retrofitting it is painful.

## Decision

- Port the prototype into `Source/Simulation` (`iron::sim`) as its own static library with **no SDL, I/O, clocks, randomness or floating point**.
- Keep behaviour bit-exact. A golden test replays the kit's 4000-tick pseudo-random input history and expects the kit's published checksum `17886306297783925268`. The checksum field order is part of the contract.
- Clean-ups that don't change state: named constants, enums for archetype/outcome/effect, readable formatting.
- New: `Match::step()` returns **`StepEvents`** (hits, guards, breaks with positions). Presentation reacts to events instead of diffing state; events are transient and excluded from snapshots, which is what rollback needs to avoid replaying sounds twice.
- The frontend runs the simulation at a fixed 60 Hz with an accumulator (capped at 250 ms per frame); `-batchmode` runs exactly one tick per frame.

## Consequences

- Gameplay changes are visible in review: the golden checksum must be updated deliberately, with a reason.
- Same-binary determinism is proven; cross-compiler/cross-platform determinism must be tested in CI before online play (integer-only state makes it likely).
- `Match` is copyable and copies are snapshots — adequate now; M4 replaces this with explicit versioned serialization.
- Move data is still compiled in (`kBaseMoves`). M1 moves it to validated JSON loaded at startup; the simulation will receive it as an immutable `Ruleset`.

## Alternatives considered

- **Rewrite from scratch**: no reason; the prototype is small, tested and matches the design docs.
- **Fixed-point library / floats with care**: unnecessary; 1/256-pixel integers are enough for a 2D fighter.
