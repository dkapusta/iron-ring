# Reference: Fatal Fury 2 (SNK, 1992)

Why this game: the project owner supplied its cover art as the original inspiration. This note records what we take from it as **genre and era reference** — and what we deliberately don't. No artwork, characters, names, logos, music or sounds from it enter the repository.

## Facts

- Developer/publisher SNK; Neo Geo MVS arcade release November 1992 (North America) and 10 December 1992 (Japan); Neo Geo AES home release March 1993; ports to SNES, Mega Drive/Genesis, PC Engine CD, X68000, Game Boy and Neo Geo CD followed in 1993–94.
- **Four attack buttons**: light/strong punch, light/strong kick.
- **Two-plane system**: fighters can hop between a foreground and a background line; some attacks knock the opponent into the other plane, and dedicated attacks hit across planes. Some stages add hazards that interact with the planes.
- **Desperation moves**: a powerful special that only becomes available when the life bar is low (about a quarter) and flashing.
- Single-player structure: eight regular opponents (including a mirror match), then four non-playable bosses, with a bonus round every fourth match.
- Contemporary reviews praised the improved graphics over the first game, the background art and the amount of interaction with the scenery.

Source: [Wikipedia — Fatal Fury 2](https://en.wikipedia.org/wiki/Fatal_Fury_2) (checked 2026-10-28).

## The cover, read as a design brief

The supplied cover (kept outside the repo) is a painted arcade-poster ensemble:

- **Hierarchy**: three heroes large in the foreground, the protagonist dead centre and frontal; the rest of the cast stacked behind them by importance, with the main rival looming at the top.
- **Setting as backdrop**: a harbour/industrial skyline at dusk under a red, lightning-split sky — the city is a character.
- **Energy**: diagonal lightning and backlighting carry motion; faces are lit, silhouettes are dark-edged.
- **Logo**: heavy chrome/brass lettering with electric accents, top centre.
- **Cast language**: everyday street clothes and work gear rather than costumes — the cast reads as city people who fight.

The Brass Current key art already follows the same principles with an original cast: centred protagonist, flankers, harbour at sunset, brass "current" streaks, chunky metallic logo.

## What Iron Ring takes

| Trait | Take it? | How |
| --- | --- | --- |
| Large, expressive sprites; readable silhouettes | Yes | 640×360 canvas, ~140–160 px tall fighters |
| Four-button layout | Yes | already in the simulation |
| Lively, inhabited backgrounds | Yes | layered stages with animated crowds (M2) |
| City/harbour tournament framing | Yes | Port Meridian (kit); underground "Iron Ring" proposal (ADR-0007) |
| Painted ensemble key art | Yes, as a style | original cast only |
| Two-plane movement | Not now | possible later experiment (ROADMAP "Later / maybe") |
| Low-health desperation moves | Undecided | competes with the meter-based Current system; pick one |
| Boss ladder, bonus rounds | Later | arcade mode in M2/M3 |

## What we must not do

- Reuse or trace any character, costume signature (cap + vest, headband, etc.), name, logo, stage or music.
- Name moves, characters or places in ways that point at the reference.
- Put the reference images in the repository or in builds.
