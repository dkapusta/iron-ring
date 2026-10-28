# ADR-0007: Brass Current kit as placeholder content; setting reconciliation

- **Status:** Proposed (needs the owner's call on the setting)
- **Date:** 2026-10-28

## Context

Three inputs shape the content:

1. The project brief: *early-90s arcade fighter themed around illegal underground fight clubs and a tournament.*
2. The **Fatal Fury 2** cover (1992) as the stated original inspiration — see [research note](../research/fatal-fury-2.md).
3. The **Brass Current: Floodlight Circuit** concept kit and book: six original fighters, three harbor-city arenas, pose atlases, effects, HUD layout, placeholder sounds, and a tested combat core. Its tone is explicitly *public, competitive, exuberant, no gore*.

The kit is the only production-shaped material we have; the brief's tone differs from it.

## Decision

- Import the kit's runtime-relevant assets into `Assets/` unchanged (stages, Mara/Rook atlases, FX atlas, HUD layout, palette, WAV cues, roster/moves/stages JSON), and its mockups/key art into `docs/concept/` as compressed references. Exclusions are listed in the kit review.
- Treat all of it as **placeholder**: names, art and audio may change once the setting is settled.
- Keep gameplay terms from the kit in code for now (`Current`, `Current Break`), because the tested simulation uses them.

**Proposed reconciliation (to confirm):** Port Meridian's legal *Floodlight Circuit* is the public face; the **Iron Ring** is the illegal after-hours bracket held in the same harbor after closing, plus hidden venues (warehouses, ship holds, basements). This reuses every kit location as a "day" variant and adds darker "night ring" variants, keeps the kit's cast with sharper motives, and stays non-gory in line with the kit — grit comes from lighting, crowds and stakes, not blood.

## Consequences

- M0/M1 engineering is unblocked regardless of the final setting.
- Art production (M1 redraw) should wait for the decision; the redraw is the expensive step.
- If the reconciliation is rejected, only `Assets/`, `ProjectSettings/GameSettings.json` and display names change; code is unaffected.

## Alternatives considered

- **Discard the kit and start fresh**: loses a coherent visual identity and tested data for no gain at this stage.
- **Rename everything now**: premature before the owner chooses the setting.
