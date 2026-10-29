# Review: Brass Current game kit 0.1 and concept book

Inputs reviewed on 2026-10-28: `Brass_Current_Game_Kit.zip` (29 MB) and `Brass_Current_Concept_Book.pdf` (8 pages). This note records what they contain, what was imported, and the issues found.

## What the kit is

An original, AI-generated concept for a classic side-on arcade fighter — *Brass Current: Floodlight Circuit* — set in the port city of Port Meridian, plus a small C++20/SDL3 starter.

- **Cast (6):** Mara Voss (boxer/counter, teal/orange), Rook Calder (grappler/power, rust/amber), Lina Vale (mobile kickboxer), Tariq Moss (range striker), Edda Flint (power brawler), Kei Sato (rushdown/feint). Only Mara and Rook have pose atlases.
- **Arenas (3):** Breaker Pier (sunset timber pier), Lantern Exchange (night market), Observatory Crown (dawn terrace). Clean 1920×1080 flattened backdrops, foot baseline at 88 % height.
- **Mechanics:** four attacks, directional or button guard, high/low defense, hitstop, six-tick buffer, 0–100 *Current* meter, **Current Break** (guard + HP + HK, 50 meter): non-damaging pushback that can cancel blockstun.
- **Code:** integer combat core, tests (incl. a 4000-tick determinism check), replay tool, and an SDL3 frontend that the kit itself says was never compiled.
- **Art status (kit's own words):** pose prototypes, not animation; uneven atlas grids compensated by measured rectangles and pivots; flattened stages; placeholder synthesized WAVs.

The concept book adds the pitch ("the fight comes first", readable silhouettes, felt commitment, modern foundations), the production order (two great fighters → local play → online) and four 1920×1080 target mockups.

## What was imported

> **Update (M1):** all kit images in `Assets/` were replaced by Iron Ring's procedural art, and `moves.json` by per-fighter definitions ([ADR-0010](../adr/0010-fighter-content-format.md), [ADR-0011](../adr/0011-procedural-interim-art.md)). Only the kit's sound effects remain in `Assets/`. The table records the initial import.

| Kit path | Repository path | Notes |
| --- | --- | --- |
| `assets/stages/*.png` | `Assets/Stages/<Name>/` | unchanged |
| `assets/characters/{mara,rook}_pose_atlas.*` | `Assets/Fighters/<Name>/` | unchanged; JSON `image` is relative to the JSON |
| `assets/fx/impact_atlas.*` | `Assets/Resources/FX/` | 4×2 grid; top row used at runtime |
| `assets/ui/hud_layout.{json,svg}` | `Assets/Resources/UI/` | JSON drives the placeholder HUD |
| `assets/branding/logo.png` | `Assets/Resources/UI/` | not yet displayed |
| `assets/audio/*.wav` | `Assets/Resources/Audio/` | wired to hit/guard/break/bell events |
| `data/{roster,moves,stages}.json` | `Assets/Data/` | `stages.json` image paths rewritten to virtual paths |
| `assets/palettes/*` | `Assets/Data/` | reference |
| `prototype/src/combat.cpp` + tests | `Source/Simulation/`, `Tests/` | ported, bit-exact (ADR-0006) |
| `screenshots/*`, `cover_key_art.png`, `roster_board.png` | `docs/concept/*.jpg` | recompressed references (~3.4 MB vs 14 MB) |

Not imported: `prototype/src/main.cpp` and `generated_art.hpp` (superseded by the engine and by atlas JSON loaded at runtime), `replay.cpp` (replay tooling returns with M4 serialization), `docs/PROMPTS.txt` (generation prompts; keep with the original kit), the concept-book PDF (7.5 MB; keep outside the repo).

## Issues found

1. **Stray Windows shortcut** `assets/ui/Brass_Current_Game_Kit - Shortcut.lnk` pointing at a local Downloads folder. Harmless, but it leaks a machine name and path. Excluded; `.gitignore` and AssetPacker now ignore `*.lnk`.
2. **Frontend never compiled.** The kit's README says so. It was not reused; the new engine is built and smoke-tested in this commit.
3. **Atlas geometry.** Poses aren't on equal cells and some poses nearly touch neighbours; pivots vary per pose. Rendering uses the measured rectangles, which works, but placeholder sprites visibly overlap at close range because the art is wider than the 36 px push box. Resolved in M1: the new art is drawn to the hurt boxes and push width.
4. **Resolution mismatch.** Art is ~3× the 640×360 logical grid. Rendering with linear filtering at native resolution looks fine (ADR-0005), but final art must be authored on the grid.
5. **JSON is reference-only** in the kit (constants are duplicated in C++). Resolved in M1: fighter JSON is the runtime source, validated by AssetPacker.
6. **Tone vs. brief.** The kit's public, family-friendly festival differs from the Iron Ring brief (illegal underground fights). See ADR-0007 for the proposed reconciliation.

## Verified

- Kit combat tests compiled and passed here: `checksum 17886306297783925268`; the port reproduces the same value.
- All PNGs decode; stages and screenshots are 1920×1080; atlases are 1774×887 RGBA; the logo is 1536×1024 RGBA.
