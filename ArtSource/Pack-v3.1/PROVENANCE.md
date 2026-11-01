# Provenance — v3.1 polish

This is an altered version of the accepted Iron Ring art, licensed on the same zlib terms as V3. The inputs were `final-polish-brief.md`, the earlier `iron-ring-main(4).zip`, the delivered V3A character masters, and the V3B/V3A.1 integrated assets. The exact P1 portrait templates and a newer current-source ZIP were not available.

This pass used deterministic native pixel corrections in JavaScript with Sharp: hand-reviewed per-pose region masks, direct palette replacement, native chain-link motifs, ordered-dither lighting and a five-pixel ankle cleanup. No image-generation call, external reference image or new character/venue design was used for this polish pass. The original generation history remains that recorded in the V3B provenance.

Python 3.12.14, Pillow 12.3.0 and NumPy 2.3.5 were used for analysis, review compositions and the project's own importer, compositor, reviewer and art self-test. Review images are made from the actual native assets. No source code or JSON is part of this delivery.

Base V3B/V3A.1 archive SHA-256: `a8255fe43877a660e81bcac0da89bada84018734244f35502f8eafbd8222fffe`.
