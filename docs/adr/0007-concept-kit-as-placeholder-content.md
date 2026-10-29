# ADR-0007: Concept kit as visual reference and placeholder content; Iron Ring setting

- **Status:** Accepted
- **Date:** 2026-10-28 (proposed), 2026-10-29 (accepted)

## Context

Three inputs shape the content:

1. The project brief: *early-90s arcade fighter themed around illegal underground fight clubs and a tournament.*
2. The **Fatal Fury 2** cover (1992) as the original inspiration — see [research note](../research/fatal-fury-2.md).
3. The **Brass Current: Floodlight Circuit** concept kit and book: six fighters, three harbor-city arenas, pose atlases, effects, HUD layout, placeholder sounds and a tested combat core. Its tone is a public, family-friendly festival.

The first version of this ADR proposed reconciling the two settings. The owner decided instead (2026-10-29):

- the setting is the Iron Ring brief as written;
- the kit is a **visual reference**; actual art assets will be generated later;
- names are renamed for Iron Ring.

## Decision

- The setting, roster, venues and terms are defined in [docs/design/setting.md](../design/setting.md): Grayhaven, 1992; the Iron Ring; Dee Marlowe, Bram Hollis and four further fighters; Pier 9, Lantern Row, The Crown; **Heat** meter and **Breakaway** (formerly *Current* and *Current Break*).
- Kit images and sounds stay in `Assets/` as **placeholders** under the new names and paths (e.g. `Fighters/Dee/dee_pose_atlas.png` still shows the kit's Mara). Their measured frame data remains valid for the placeholder art only.
- Kit mockups and key art stay in `docs/concept/` as reference.
- The simulation port keeps its behaviour; only identifiers and comments changed (golden checksum unchanged).
- The kit's provenance is kept where it is factual (research notes, "ported from" comments).

## Consequences

- Engineering is unblocked; the art pass (M1) replaces files under the same virtual paths, so code and data references survive.
- Placeholder art visibly contradicts the setting (sunset festival vs. night ring). Accepted until generated art lands.
- Generated art must be produced against `docs/design/setting.md`, not the kit's prompts.

## Alternatives considered

- **Reconcile both settings** (the original proposal): rejected by the owner.
- **Remove kit art now**: leaves nothing to render or test the pipeline with.
