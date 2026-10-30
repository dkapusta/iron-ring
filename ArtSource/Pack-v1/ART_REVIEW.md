# Review and update rationale

## Existing state

The snapshot has a useful and unusually clear technical art contract: fixed 640×360 presentation, material ramps, complete fighter frame names, registration points, move timings and paired throw offsets. Its external-art importer and pose exporter are suitable foundations. The current `textureFilter` is already `pixelart`.

The main visual limitation is the procedural figure construction. Both starters have narrow capsule-like limbs, small facial features, and similar body language. Bram's bulk is not strongly separated from Dee's lean boxer silhouette. The procedural stages establish the intended venues and chain signature, but their flat geometry, repeated spectators and broad simple color fields leave them looking like placeholders.

The small effects are deliberately simple and technically precise: sparks, guard crescents, rings and dust use the exact prescribed counts and dimensions. The bitmap fonts, HUD structure and icon likewise have strict small-grid requirements. `generation-prompts.md` explicitly recommends keeping or hand-authoring those assets. This pack retains them through the existing game installation; it does not label unchanged files as new art.

## Updated direction

| Area | Change | Reason |
| --- | --- | --- |
| Dee | Cropped curls, readable face, defined shoulders, wrap bands, layered teal fabric and striped track pants | Stronger identity and readable striking limbs |
| Bram | Bald head and full beard, barrel chest, thick forearms, worn charcoal shirt, folded rust overalls and heavy boots | Distinct grappler mass and dockworker costume |
| Animation | All 49 / 50 poses repainted from registered pose guides | Preserve existing gameplay coverage and identifiers |
| Portraits | Dedicated head-and-shoulders art imported at 32×32 | Better recognition than a full-body model-sheet crop |
| Pier 9 | Riveted crane framing, freighter deck detail, harbor depth, betting boards and wet timber | Industrial scale and spatial depth |
| Lantern Row | Worn shutters, layered brickwork, clock, restrained practical lighting and crowds behind the rope | Specific after-curfew market atmosphere |
| The Crown | Water tower, bulb sign, parapet and layered dawn skyline | A distinct final-bout venue |
| Stage floor / HUD | Registered chain band and dark palette subsets | Readability under fighters, health bars and Heat meters |

The 1992 Grayhaven setting and costume identities follow the supplied bible. No franchise reference images or logos were used as inputs. Sable, Idris, Greta and Jun are still design placeholders in the source snapshot; they are outside this starter-roster replacement pack.

## What the review does not establish

A generated frame can fit its pose envelope and intersect its hitbox while still needing an animator's adjustment. The throw pair composites are supplied expressly for that judgment. Native previews were inspected, but no SDL renderer capture, training-mode playthrough, Windows build or CI run was performed here.
