# ADR-0022: A boss bout after the title, and the arcade story as fighter data

- **Status:** Accepted
- **Date:** 2026-10-31

## Context

M3 closes the arcade with the Broker, the Ring's promoter, as boss, and gives every fighter a story: why they enter, an ending, and win quotes. Until now the ladder ended at The Crown, opponents could only be roster fighters, and every ending read "THE IRON RING HAS A NEW CHAMPION".

## Decision

- **The boss is a fighter file like any other.** He is listed in `GameSettings.json → fighters`, and his roster entry carries `"boss": true`. `GameData` keeps such entries out of the select's roster and out of `playableIds()` (so "rival" never picks him), but `fighter(id)` finds him.
- **Boss bouts come after the title.** A ladder bout with `"boss": true` always fights the first opponent it names, whoever the player is, and is not counted as "met". The title bout (`final`) stays the last counted bout ("BOUT n OF 5", "FINAL BOUT"). The boss card reads "BY INVITATION" and shows his greeting; the select's ladder preview shows "VS ???". The last bout must be `final` or `boss`.
- **The story is fighter data.** Each fighter file has an optional `story`: `intro` (drawn before the first bout, or as the boss's greeting), `ending` (after the last bout, in place of the generic line) and `winQuotes` (one after each match won, picked from the loser and the round count, so a replay shows the same quote). Lines are sentence case in the data and upper-cased when drawn. AssetPacker rejects any line longer than 60 characters, or with a character the bitmap fonts can't draw.
- **The run opens on the story.** `ArcadeRun` gains a first step, `Story`, which leads to `Versus`. Autoplay skips it after a second.
- **The boss is tuned to be the hardest bout, not a wall.** In the CPU-vs-CPU win matrix at VETERAN, the Broker wins 51–83 % of matches against each fighter. He fights at CHAMPION level on NORMAL.

## Consequences

- More bosses or a second story path are data. A secret boss would need a condition on the bout, which the format doesn't have yet.
- The arcade soak in CI runs 20,000 frames to reach the ending through six bouts.
- Writers can change every line without touching code; the build catches lines the fonts can't render.

## Alternatives considered

- **A separate story file per run:** splits a fighter's identity across two files for no gain.
- **Make the Broker the title bout:** The Crown is already the series' final venue and the gatekeeper's bout; an invitation afterwards keeps both.
- **Random win quotes:** a replay would show a different line from the same match.
