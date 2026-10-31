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
| **The Broker** | The promoter, and the arcade's boss: he waits at his club for whoever takes the Crown | `broker` (`roster.json`: `boss`) |

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

### Fighters in detail

Moves and frame data live in `Assets/Fighters/<Name>/<id>.fighter.json`.

**Dee Marlowe**: lean, athletic, late twenties. Cropped dark curls, a silver stud earring. Teal sleeveless top, charcoal track pants with a chalk side stripe, white hand wraps and tape to mid-forearm, black boxing boots. A compact boxer's stance, chin tucked. Fast, precise, punishes whiffs.

- Stiff Jab (LP), Cross Counter (HP), Shin Kick (LK), Leg Sweep (HK, low, knockdown)
- Clinch Toss (throw, LP + LK)
- **Rush Uppercut** (236 + P): a dashing uppercut that reaches high and knocks down; it closes the gap on Bram's slow advance, but leaves her wide open if blocked

**Bram Hollis**: huge and heavy, forties. Shaved head, full dark beard, heavy brow. Charcoal T-shirt, rust work overalls with the bib folded down and straps hanging, leather wrist straps, steel-toe boots. A wide, low stance with open hands. Slow, but every touch hurts.

- Palm Shove (LP), Anchor Hook (HP), Boot Kick (LK), Hull Breaker (HK, low, knockdown)
- Pier Toss (throw, LP + LK)
- **Dock Hoist** (63214 + P): a command grab that lifts the opponent overhead and slams them; it can't be teched, so Dee has to stay out of reach or hit him first

**Sable Reyes**: lean and long-legged, early twenties. Black hair pulled into a high ponytail, a violet sleeveless top, black kickboxing shorts, white shin wraps, worn grey trainers, fingerless gloves. A tall, bladed stance with the hands high, always moving. She runs packages across Grayhaven's rooftops by night and fights strictly for the purse.

- Snap Jab (LP), Rear Cross (HP), Calf Kick (LK, low), Wire Kick (HK, a head-high roundhouse)
- Rooftop Drop (throw, LP + LK)
- **Switch Step** (236 + K): a stance switch, a step and a flying knee that knocks down; it gets her past a jab, but it's wide open if blocked

**Idris Okafor**: tall and rangy, forties. Close-cropped hair going grey and a short beard, a faded blue work shirt with the sleeves rolled to the elbow, canvas trousers rolled at the shin, a length of rope for a belt (tied in a bowline), deck shoes. Upright and side-on, open hands. Stranded when his ship was impounded; he fights to buy passage out.

- Long Jab (LP), Hawser Straight (HP), Mast Kick (LK, a push kick), Deck Sweep (HK, low, knockdown)
- Gangway Heave (throw, LP + LK)
- **Bowline Heel** (214 + K): a turning heel kick at head height that knocks down; slow, so it's thrown at where the opponent is going

**Greta Voll**: broad and solid, fifties. Iron-grey hair in a tight bun, a khaki work shirt with rolled sleeves under a brass-brown leather foundry apron, dark work trousers, heavy leather gloves, steel-toe boots. A square, low stance, fists at the chin. She ran the Voll Works floor until the Broker's money shut it; she fights at The Foundry like it's still hers.

- Short Jab (LP), Slag Knuckle (HP), Toe-Cap Kick (LK), Pig-Iron Sweep (HK, low, knockdown)
- Crucible Toss (throw, LP + LK)
- **Ladle Drop** (623 + P): both fists brought down like a tipping ladle, lunging in; it knocks down, and misses are very expensive

**Jun Arata**: small and wiry, early twenties. Messy black hair, a faded jade track jacket with the sleeves shoved up, loose black trousers, red high-tops, taped wrists. Hunched and light on his feet, never still. He works the crowd's pockets between bouts and fights like he steals: quick, close, gone before you react.

- Pick Jab (LP), Pocket Hook (HP), Lift Step (LK, a stepping low kick), Turnstile Kick (HK)
- Purse Snatch (throw, LP + LK)
- **Sleight Feint** (236 + P): a low dash into a quick elbow that leaves him ahead on block, so the pressure starts over

**The Broker**: tall and lean, sixties. Silver hair combed straight back, a charcoal three-piece suit, a white shirt and a burgundy tie, a gold watch chain across the waistcoat, black kid gloves, polished shoes. Nobody knows his name. He owns half of Grayhaven's debts, and he fights like the bare-knuckle prints on his walls: upright, fists forward, never in a hurry. Not on the select screen: he's the last bout of the arcade, by invitation, at his club.

- Cufflink Jab (LP), Signet Cross (HP), Oxford Kick (LK), Collection Sweep (HK, low, knockdown)
- Foreclosure (throw, LP + LK)
- **Final Offer** (236 + P): a long step into an open-palm strike that knocks down

### Story

The arcade opens on the player's fighter and why they entered; after the title bout at The Crown, the Broker invites the new champion to his club; beat him and the fighter's ending plays. Every fighter also has three win quotes, one shown after each match won. The lines live in each fighter file's `story` (`intro`, `ending`, `winQuotes`); in short:

| Fighter | Enters because | Ending |
| --- | --- | --- |
| Dee | Her brother's debt to the Broker | The Broker tears up the marker; she keeps fighting to keep him honest |
| Bram | Twelve years the gatekeeper; this year he takes the Crown | He buys the old Pier 9 crane; the Ring meets under his hook |
| Sable | The biggest fee in town | Gone by midnight; postcards turn up at Lantern Row |
| Idris | Passage home for himself and his crew | Buys back the *Marisol* and sails with the tide |
| Greta | The Broker shut her foundry | Walks out with the deed; the furnaces are lit on Monday |
| Jun | The crowds' wallets | Picks the Broker's pocket: every marker he holds in the city |

The colour coding stays with the characters: teal for Dee, rust/coral for Bram, violet for Sable, muted blue for Idris, brass for Greta, jade for Jun, gold and charcoal for the Broker.

## Venues

| id | Name | Scene |
| --- | --- | --- |
| `pier_nine` | **Pier 9** | Closed cargo pier at night; chain ring between crane legs, dockers betting behind the rail, floodlights from a moored freighter. |
| `lantern_row` | **Lantern Row** | Night market after curfew; shutters down, the ring roped off under the old station clock, lanterns half lit. |
| `foundry` | **The Foundry** | The shut-down Voll Works, Greta Voll's old plant: the ring laid between cold furnaces under a hanging ladle, the last night shift betting behind the rail. |
| `the_crown` | **The Crown** | Roof terrace of the derelict Crown Hotel at first light, under the hotel's half-dead bulb sign — the final bout. |
| `dry_dock` | **Dry Dock 4** | A flooded dry dock under a laid-up freighter's bow; the ring on wet concrete, dockers behind a sagging chain. |
| `ship_hold` | **The Hold** | The hold of an impounded freighter (Idris's ship): steel ribs, stacked crates, a work lamp swinging under the open hatch. |
| `brokers_club` | **The Broker's Club** | The Broker's private club: velvet, brass and gilt frames, the city's money watching from behind a red rope. |

The three M3 venues (Dry Dock 4, The Hold, the Broker's Club) have procedural placeholder art for now; the first four have final art (Art Packs v2 and v2.1).

## Art direction

The binding rules (sizes, palette, frames, layout) are in the [asset specification](../art/asset-spec.md); the fighters and stages come from Art Pack v1 ([ADR-0012](../adr/0012-adopting-external-art.md)); effects, fonts and the icon are procedural ([ADR-0011](../adr/0011-procedural-interim-art.md)).

- The concept kit is a **visual reference only** (silhouette clarity, palette discipline, sprite scale, HUD layout); none of its art ships.
- Night is the default; The Crown is the one dawn stage. Warm sodium/amber key light against teal-blue shadow keeps the kit's teal/coral readability.
- The chain ring on the floor is the series' visual signature; keep the floor clear and lower-contrast than the fighters.
- Crowds sit behind barriers: dockers, market traders, bookmakers with chalk boards, a few well-dressed spectators who obviously aren't from the docks.
- Costumes: work clothes, street clothes, wraps and tape — no uniforms, gis, headbands-and-caps combos or anything that echoes Fatal Fury's cast ([research note](../research/fatal-fury-2.md)).
