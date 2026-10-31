# Venue authoring notes

The three venues are recovered V3 artwork, reviewed and revalidated for this drop. Native stage layers remain byte-identical to the recovered production masters; their 640 × 360 previews were recomposed with the supplied project compositor.

| Venue | Unique spectators | Moving at rest | Cheering | Solid barrier base from row |
| --- | ---: | ---: | ---: | ---: |
| dry_dock | 42 | 14 | 21 | 68 |
| ship_hold | 42 | 14 | 21 | 72 |
| brokers_club | 40 | 13 | 20 | 68 |

`Spectators/` contains the individual native people; `PeopleLayers/` contains their animation without the barrier, and `Barriers/` contains the static barrier overlay. These are authoring files only. Rest frames are 0–3 and cheering frames 4–7.

The Hold uses two 728 × 360 frames in its mid layer, at 30 ticks each. Its static structure is shared; the work lamp and its light move between the two frames. All original layer sizes, positions and parallax values are retained.

`Generated/Venues/` contains the 21 large original layer/crowd studies. These may have more colours and different dimensions than the finished native layers. They are not runtime assets.
