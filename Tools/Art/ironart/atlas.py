"""Sprite trimming, atlas packing and atlas JSON (shared by the generator and the importer).

Atlas JSON schema (docs/art/asset-spec.md, section "Sprite atlas"):
{
  "image": "<file>.png",
  "frames": [ {"name": "idle_0", "source_rect": [x, y, w, h], "pivot_pixels": [px, py],
               "partner": [dx, dy]  (optional, throws)} ]
}
pivot_pixels is the figure origin (centre of the feet on the ground) relative
to the frame's source rectangle.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image

PADDING = 2
FLOOR_TOLERANCE = 3  # px a frame may reach below its pivot (sole outlines, cloth)


@dataclass
class Frame:
    name: str
    image: Image.Image        # trimmed RGBA
    pivot: tuple[int, int]    # origin inside the trimmed image
    partner: tuple[int, int] | None = None


def trim(name: str, image: Image.Image, origin: tuple[int, int], partner=None) -> Frame:
    box = image.getbbox()
    if box is None:
        raise ValueError(f"frame '{name}' is empty")
    x0, y0, x1, y1 = box
    return Frame(name, image.crop(box), (origin[0] - x0, origin[1] - y0),
                 None if partner is None else (round(partner[0]), round(partner[1])))


def pack(frames: list[Frame], max_width: int = 1024) -> tuple[Image.Image, dict[str, tuple[int, int, int, int]]]:
    """Shelf packing, tallest first. Deterministic for a given frame list."""
    order = sorted(frames, key=lambda f: (-f.image.height, f.name))
    rects: dict[str, tuple[int, int, int, int]] = {}
    x = y = shelf = 0
    width = 0
    for f in order:
        w, h = f.image.size
        if x + w + PADDING > max_width:
            x = 0
            y += shelf + PADDING
            shelf = 0
        rects[f.name] = (x + PADDING, y + PADDING, w, h)
        x += w + PADDING
        shelf = max(shelf, h)
        width = max(width, x + PADDING)
    height = y + shelf + 2 * PADDING
    atlas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    for f in frames:
        rx, ry, _, _ = rects[f.name]
        atlas.paste(f.image, (rx, ry))
    return atlas, rects


def below_floor(frames: list[Frame]) -> list[str]:
    """Frames that reach further below the ground line than FLOOR_TOLERANCE.

    Meaningless for effects (centred pivots); callers check fighter atlases only.
    """
    return [f"{f.name} ({f.image.height - f.pivot[1]} px)" for f in frames
            if f.image.height - f.pivot[1] > FLOOR_TOLERANCE]


def write(frames: list[Frame], png: Path, extra: dict | None = None) -> dict:
    atlas, rects = pack(frames)
    png.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(png, optimize=True)
    entries = []
    for f in frames:  # keep authoring order in the JSON
        entry = {"name": f.name, "source_rect": list(rects[f.name]), "pivot_pixels": list(f.pivot)}
        if f.partner is not None:
            entry["partner"] = list(f.partner)
        entries.append(entry)
    data = {"image": png.name, **(extra or {}), "frames": entries}
    # One frame per line keeps diffs of regenerated art readable.
    header = {k: v for k, v in data.items() if k != "frames"}
    lines = ["{"] + [f"  {json.dumps(k)}: {json.dumps(v)}," for k, v in header.items()] + ['  "frames": [']
    lines += [f"    {json.dumps(e)}," for e in entries]
    lines[-1] = lines[-1].rstrip(",")
    lines += ["  ]", "}"]
    png.with_suffix(".json").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return data
