# Provenance — Iron Ring Art Pack v3A: Characters

## Recovery and current work

The character artwork was recovered from the previously saved `IronRing_ArtPack_v3.zip`. Its archive hash is recorded in `Previews/Validation/report.json`. The current pass compared the recovered artwork with the latest supplied brief and source snapshot, inspected the character sheets and key poses, preserved all 269 native frame PNGs and nine portraits, rebuilt the atlases with the current importer, reran validation, and packaged the character-only scope. No new image-generation calls were necessary in this recovery pass.

The new documentation marks this as a recovered and revalidated subset, rather than representing the earlier artwork as newly generated. Earlier generation receipts and registration/repair notes remain in `Source/`; paths in those records refer to the original authoring session.

## Earlier artwork creation

The recovered provenance records OpenAI's built-in `image_gen` tool, followed by pixel authoring and cleanup using Python, Pillow, NumPy and SciPy. That tool did not expose a model/version identifier, so none is asserted here. Character prompts, individual action-generation jobs and repair prompts are included.

Recorded inputs were the project's own Iron Ring v3 brief, art specifications and setting, exported pose guides, earlier finished Iron Ring artwork, and character identity sheets and studies generated for this pack. The recovered workflow records separate generated action keys and native edits for idle/walk phases, champion breathing, chain props, registration, palettes, hands and silhouettes.

The earlier provenance states that no franchise screenshots, franchise characters or logos, real brands, real people, or external reference images were used. This recovery pass likewise introduced no such inputs. The cast is fictional. Dee and Bram appear only as unchanged comparison references in a preview; no replacement assets for them are delivered.

## Current tools and licence

Current validation used Python, Pillow, NumPy and the supplied Iron Ring importer, frame reviewer and art self-test. Exact installed versions and limitations are in `Previews/Validation/report.json`. Preview composition uses existing sprite pixels; it is not new generated character artwork.

The character artwork and its native editable masters are contributed under the project's **zlib licence**, included as `LICENSE`. No venue deliverables are part of this ZIP.
