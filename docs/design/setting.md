# Setting: the Iron Ring

Working bible for names, places and tone. Everything here may evolve; code and data use the ids in the tables, so renaming a display name is cheap, renaming an id is a small refactor.

## Premise

**Grayhaven, 1992.** A rust-belt port city whose shipyards closed a decade ago. Cranes still stand over empty piers; the night markets never quite shut; the money moved underground.

The **Iron Ring** is an illegal, invitation-only fighting tournament run by a promoter known only as **the Broker**. Every bout is held somewhere new — a cargo pier after the night shift, a market after curfew, a hotel roof at dawn — and the fighting floor is always marked the same way: a circle of welded chain dragged into place before the crowd arrives. Fighters enter for the purse, to clear a debt, or to settle something the courts won't. Spectators bet; nobody calls the police.

**Tone.** Gritty, sweaty, after-hours — sodium lamps, steam, cigarette haze, crowds pressed against barriers — but not gory. No blood or gore effects; impact is sold with hitstop, sparks, sound and crowd reaction (as the concept kit recommends). Early-90s arcade spirit: big personalities, short bios, one-line win quotes.

## Terms

| Term | Meaning | In code / data |
| --- | --- | --- |
| **Heat** | 0–100 meter built by hitting, getting hit and blocking — the crowd's attention; it lasts the whole match | `Fighter::meter`, HUD label `meterName` |
| **Breakaway** | Guard + heavy punch + heavy kick for 50 Heat while blocking: a guard cancel that staggers the attacker, no damage ([heat.md](heat.md)). The only thing Heat pays for | `Effect::Breakaway`, `sim::kBreakCost` |
| **Desperation moves** | None, by decision (2026-10-29). Low health gives no super; comebacks come from Heat and reads | — |
| **The Broker** | Unseen promoter; final-boss candidate | — |

## Roster

Archetypes and gameplay roles are inherited from the concept kit; identities are new. The two starter fighters are fully specified in data; the other four are design placeholders.

| id | Name | Role | Hook |
| --- | --- | --- | --- |
| `dee` | **Dee Marlowe** | Boxer / counter — *protagonist* | Golden Gloves contender banned from sanctioned boxing; fighting to clear her brother's debt to the Broker. |
| `bram` | **Bram Hollis** | Grappler / power — *starter rival* | Laid-off shipyard rigger and the Ring's gatekeeper; nobody reaches the Crown without going through him. |
| `sable` | **Sable Reyes** | Mobile kickboxer | Night courier who knows every rooftop in Grayhaven; fights strictly for the purse. |
| `idris` | **Idris Okafor** | Long-range striker | Merchant sailor stranded when his ship was impounded; long, deliberate kicks. |
| `greta` | **Greta Voll** | Power brawler | Foundry forewoman whose plant the Broker's money shut down. |
| `jun` | **Jun Arata** | Rushdown / feint | Pickpocket who works the crowd between bouts; feints, resets, cheap shots that are technically legal. |

### Starter fighters in detail

Moves and frame data live in `Assets/Fighters/<Name>/<id>.fighter.json`.

**Dee Marlowe**: lean, athletic, late twenties. Cropped dark curls, a silver stud earring. Teal sleeveless top, charcoal track pants with a chalk side stripe, white hand wraps and tape to mid-forearm, black boxing boots. A compact boxer's stance, chin tucked. Fast, precise, punishes whiffs.

- Stiff Jab (LP), Cross Counter (HP), Shin Kick (LK), Leg Sweep (HK, low, knockdown)
- Clinch Toss (throw, LP + LK)
- **Rush Uppercut** (236 + P): a dashing uppercut that reaches high and knocks down; it closes the gap on Bram's slow advance, but leaves her wide open if blocked

**Bram Hollis**: huge and heavy, forties. Shaved head, full dark beard, heavy brow. Charcoal T-shirt, rust work overalls with the bib folded down and straps hanging, leather wrist straps, steel-toe boots. A wide, low stance with open hands. Slow, but every touch hurts.

- Palm Shove (LP), Anchor Hook (HP), Boot Kick (LK), Hull Breaker (HK, low, knockdown)
- Pier Toss (throw, LP + LK)
- **Dock Hoist** (63214 + P): a command grab that lifts the opponent overhead and slams them; it can't be teched, so Dee has to stay out of reach or hit him first

The colour coding stays with the characters: teal for Dee, rust/coral for Bram.

## Venues

| id | Name | Scene |
| --- | --- | --- |
| `pier_nine` | **Pier 9** | Closed cargo pier at night; chain ring between crane legs, dockers betting behind the rail, floodlights from a moored freighter. |
| `lantern_row` | **Lantern Row** | Night market after curfew; shutters down, the ring roped off under the old station clock, lanterns half lit. |
| `foundry` | **The Foundry** | The shut-down Voll Works, Greta Voll's old plant: the ring laid between cold furnaces under a hanging ladle, the last night shift betting behind the rail. |
| `the_crown` | **The Crown** | Roof terrace of the derelict Crown Hotel at first light, under the hotel's half-dead bulb sign — the final bout. |

Planned additions (M3): a flooded dry dock, a ship's hold, the Broker's private club. The Foundry is in the game with placeholder art (the first layered venue).

## Art direction

The binding rules (sizes, palette, frames, layout) are in the [asset specification](../art/asset-spec.md); the fighters and stages come from Art Pack v1 ([ADR-0012](../adr/0012-adopting-external-art.md)); effects, fonts and the icon are procedural ([ADR-0011](../adr/0011-procedural-interim-art.md)).

- The concept kit is a **visual reference only** (silhouette clarity, palette discipline, sprite scale, HUD layout); none of its art ships.
- Night is the default; The Crown is the one dawn stage. Warm sodium/amber key light against teal-blue shadow keeps the kit's teal/coral readability.
- The chain ring on the floor is the series' visual signature; keep the floor clear and lower-contrast than the fighters.
- Crowds sit behind barriers: dockers, market traders, bookmakers with chalk boards, a few well-dressed spectators who obviously aren't from the docks.
- Costumes: work clothes, street clothes, wraps and tape — no uniforms, gis, headbands-and-caps combos or anything that echoes Fatal Fury's cast ([research note](../research/fatal-fury-2.md)).
