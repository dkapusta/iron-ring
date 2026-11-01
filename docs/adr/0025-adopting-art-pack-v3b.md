# ADR-0025: Adopt Art Pack v3B (the M3 venues) and the v3A.1 corrections

- **Status:** Accepted
- **Date:** 2026-11-01

## Context

Art Pack v3's second drop combines two deliveries:
- the three M3 venues (the [v3 brief](../art/art-pack-v3-brief.md)'s S1–S3);
- the four required fixes and one recommended fix from the [v3A.1 correction brief](../art/art-pack-v3a.1-brief.md) ([ADR-0024](0024-adopting-art-pack-v3a.md)).

The review ([Pack-v3B/REVIEW.md](../../ArtSource/Pack-v3B/REVIEW.md)) found:
- every correction fixed: the Broker's skin, Sable's walk frames, Idris's two kicks inside their boxes, Sable's Wire Kick lowered;
- all three venues meeting the layer contract and the crowd rules, with previews equal to their layers.

It left three polish notes: Dry Dock 4's floodlight beam, a dither band in its sky, and The Hold's dim lamp.

## Decision

- Adopt the corrected frames, the Broker's portrait, the 15 venue layers and the 3 previews unchanged.
- Dry Dock, The Hold and The Broker's Club join `EXTERNAL` in `ownership.py`, and the `venues` part joins `generate.py`'s `REPLACED` list. Like `foundry.py`, `venues.py` stays as a composition reference.
- The polish notes go to the owner's play-test rather than straight back as a correction round. They are about taste and emphasis, not faults the game would expose.
- Provenance is kept as with v3A: the editable people, barrier and spectator masters are in the repository; the generated studies and previews are not.

## Consequences

- No procedural placeholder art is left in the game. The procedural generator now owns only the palette, effects, fonts, the HUD layout and the icon, all of them final by decision.
- Every M3 roadmap item is done.
- The simulation is untouched; the golden checksum and the balance matrix hold.
- *Later (Art Pack v3.1, final polish):* the polish notes and a whole-set review were answered:
  - Greta's and Jun's skin out of their apron and shoe ramps;
  - lit chain rings on the M3 floors;
  - Dry Dock 4's beam and sky softened, and The Hold's lamp;
  - one shared portrait backdrop, applied by the project and checked by the art self-test ([Pack-v3.1/REVIEW.md](../../ArtSource/Pack-v3.1/REVIEW.md)).
