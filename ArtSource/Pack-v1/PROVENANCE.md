# Sources and production record

- Input: the user-supplied `iron-ring-main.zip`, snapshot commit `6582a0a25a9d3b887bc1643abe7b72e92d037998`.
- Direction: the snapshot's art specification, generation prompts, palette, setting bible, procedural pose guides and stage layouts.
- Image creation and creative revision: built-in image generation. Complete prompt sets are stored under `Source/`.
- Technical import: per-frame isolation, fixed pose-envelope registration, area resampling, binary alpha, character-ramp quantization, colored silhouette-edge quantization, trimming and two-pixel shelf packing. Native stage rows are registered to the floor contract, then palette constrained.
- The original procedural atlas sizes and pivots are reference metadata, not replacement image coordinates. All source frames share origin (128,232). Trimmed runtime pivots are derived from that canonical origin; minor trim differences do not change the world origin.
- Editable source format: individual PNG frames and PNG stage/portrait sources with JSON registration. No layered Aseprite/PSD masters or model seeds are claimed. Image-generation outputs are not deterministic; the supplied PNG sources are the reproduction anchor.
- Snapshot metadata and pose guides originate from Dmitry Kapusta's Iron Ring project. Its zlib notice is reproduced in `SOURCE_LICENSE.txt`. The new rebuild and validation helpers are external-pack utilities, not modifications to the game's original tools.
- No franchise screenshots, characters, logos, third-party game assets or real-world brand marks were included as generation inputs.

The generated atlases contain all requested starter poses; their presentation checks do not replace a final animator/gameplay review in training mode. No source-code build, SDL runtime test or cross-platform CI result is claimed.
