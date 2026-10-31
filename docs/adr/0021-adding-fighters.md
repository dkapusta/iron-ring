# ADR-0021: Add fighters as data with procedural placeholder art, and resolve the arcade ladder per run

- **Status:** Accepted
- **Date:** 2026-10-31

## Context

M3 grows the roster from two fighters to six: Sable first, then Idris, Greta and Jun. The fighter format ([ADR-0010](0010-fighter-content-format.md)) already makes a fighter data. Adding the first new one exposed three gaps:

1. **Art ownership.** The `fighters` part of `generate.py` was all external: with Dee and Bram's final art in `Assets/`, the generator refused to write any fighter there. A new fighter had no route to placeholder art.
2. **The arcade ladder** named each bout's opponent as `rival` (the first other fighter in roster order), `mirror` or a fixed id. With three or more fighters, `rival` always picked the same one, and a fixed id clashed whenever it was the player's own fighter.
3. **The CPU** typed every special with a punch, so a 236 + K special never came out.

## Decision

- **Placeholder fighters are procedural.** Ownership is per fighter folder (`ownership.py`). `generate.py fighters` draws every fighter whose folder isn't external and leaves Dee and Bram alone. A new fighter brings:
  - a `Character` (build and costume) in `ironart/characters.py`;
  - a `Style` and any custom poses in `ironart/poses.py`;
  - colour ramps in `ironart/palette.py`;
  - the fighter file, a roster entry, a `GameSettings.json` entry and a `<id>_wins` announcer line.
- **Final art comes later.** When an art pack delivers a fighter, its folder joins `EXTERNAL`, exactly as Dee and Bram did ([ADR-0012](0012-adopting-external-art.md)). Until then, the self-test keeps the placeholder equal to the generator's output.
- **Ladder bouts list candidates.** `opponent` may be a list of ids (and `rival` or `mirror`). `arcadeOpponents()` resolves the whole run when it starts, so the character select's ladder preview and the run always agree. Each bout's opponent is the first of these that applies:
  1. the first listed fighter not met yet;
  2. anyone not met yet;
  3. a listed fighter, met before;
  4. whoever was met longest ago;
  5. the player's own fighter, for a roster of one.
- **The CPU fits the button to the special.** It keeps asking for a light or a heavy, and takes the kick of the same strength when the special wants a kick.
- **`-fighters p1,p2`** picks who fights in `-demo`, `-training` and `-stage`.

## Consequences

- Adding Idris, Greta and Jun is content plus their pose code: no engine change unless a fighter needs a new mechanic.
- The shipped ladder (`Pier 9: [sable, dee]`, `Lantern Row: [dee, sable]`, `Foundry: rival`, `The Crown: [bram, dee]`) gives each fighter both others before any repeat. Bram, the gatekeeper, waits at The Crown for everyone but himself.
- Placeholder fighters look visibly simpler than Dee and Bram. This is accepted until their art pack: the slice standard applies to Dee vs Bram.
- Content tests pin the ladder rules, and that the CPU lands Sable's 236 + K.

## Alternatives considered

- **A random opponent order:** the ladder preview and the run could disagree, and replays would depend on a seed outside the match.
- **Hand-written ladders per fighter:** six files to keep in step for every roster change.
- **Commission final art before adding fighters:** blocks gameplay work on art, the opposite of the M1/M2 order ([ADR-0011](0011-procedural-interim-art.md)).
