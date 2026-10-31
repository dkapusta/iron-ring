# ADR-0024: Adopt Art Pack v3A (the M3 fighters) as delivered, with a correction round

- **Status:** Accepted
- **Date:** 2026-10-31

## Context

The [Art Pack v3 brief](../art/art-pack-v3-brief.md) asked for production art to replace every M3 placeholder. The owner split the pack in two: v3A, the five fighters, and v3B, the three venues, to follow once the characters are accepted.

v3A delivers:
- Sable, Idris, Greta and Jun: 55 frames each, with portraits and select portraits;
- the Broker: 49 frames and a HUD portrait;
- the throw partner offsets of the new raised throws.

The review ([Pack-v3A/REVIEW.md](../../ArtSource/Pack-v3A/REVIEW.md)) found the pack technically complete. Every frame is present, the format rules hold, `review_frames.py` reports 0 findings, the self-test passes and the atlases round-trip exactly. Four art faults remain:
1. the Broker's skin is drawn with the gold ramp;
2. a flat slab joins Sable's legs in two walk frames;
3. Idris's Mast Kick is drawn about 22 px above its hitbox;
4. his Bowline Heel is drawn about 24 px above its hitbox.

## Decision

- **Adopt the frames, portraits and manifests unchanged.** The five fighter folders join `EXTERNAL` in `ownership.py`, so `generate.py` no longer draws any fighter. The four playable fighters get `selectPortrait`.
- **Correct through the art side, not the data.** The two kicks keep their boxes: the Mast Kick hits crouching opponents by design, and raising its box to the drawn leg would change the move. The fixes go back as the [v3A.1 correction brief](../art/art-pack-v3a.1-brief.md), as with Art Pack v2.1. Fixing them by hand here would fork the art from its authoring source.
- **Provenance** is kept as with v2 ([ADR-0020](0020-adopting-art-pack-v2.md)). `ArtSource/Pack-v3A/` holds the pack's notes, licence, validation and small source records. The 34 MB of generated studies and the previews stay outside the repository.

## Consequences

- No procedural fighter art is left in the game. `generate.py fighters` and the art self-test's generator comparison now cover no fighter. The procedural characters remain as pose references (`import_art.py refs`) for new frames.
- The simulation is untouched, so the golden checksum and the balance matrix hold.
- Until v3A.1 arrives, the Broker looks gold-faced, and Idris's two kicks read higher than they hit. Neither affects play.
