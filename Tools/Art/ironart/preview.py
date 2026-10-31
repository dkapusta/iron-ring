"""The 640 x 360 preview of a layered stage: its layers composed at the middle
camera position, as StageView draws them (asset spec §3.1).

The stage select, the title and the arcade ending show the preview, so it must
follow the layers: `selftest.py` checks it, `compose_previews` rewrites it.
"""

from __future__ import annotations

import json
import math
from pathlib import Path

from PIL import Image

SCREEN_W, SCREEN_H = 640, 360


def _paste(out: Image.Image, image: Image.Image, x: int, y: int) -> None:
    left = max(0, -x)
    right = min(image.width, SCREEN_W - x)
    if right > left:
        out.alpha_composite(image.crop((left, 0, right, image.height)), (x + left, y))


def compose(assets: Path, stage: dict) -> Image.Image:
    """The preview of a layered stage: frame 0 of every layer, no drift, camera at the middle."""
    camera = (stage["width"] - SCREEN_W) // 2
    out = Image.new("RGBA", (SCREEN_W, SCREEN_H), (0, 0, 0, 255))
    layers = stage["layers"]
    for layer in [l for l in layers if not l.get("front")] + [l for l in layers if l.get("front")]:
        image = Image.open(assets / layer["image"]).convert("RGBA")
        frame_h = image.height // layer.get("frames", 1)
        image = image.crop((0, 0, image.width, frame_h))
        x = math.floor(layer.get("x", 0) - camera * layer.get("parallax", 1.0))
        y = layer.get("y", 0)
        if layer.get("drift"):  # repeats horizontally
            x = int(math.fmod(x, image.width))
            if x > 0:
                x -= image.width
            for at in range(x, SCREEN_W, image.width):
                _paste(out, image, at, y)
        else:
            _paste(out, image, x, y)
    return out.convert("RGB")


def layered_stages(assets: Path) -> list[dict]:
    stages = json.loads((assets / "Data" / "stages.json").read_text(encoding="utf-8"))["stages"]
    return [s for s in stages if s.get("layers")]


def compose_previews(assets: Path) -> list[str]:
    """Rewrites every layered stage's preview; returns the ones that changed."""
    import numpy as np

    changed = []
    for stage in layered_stages(assets):
        path = assets / stage["image"]
        preview = compose(assets, stage)
        if not path.exists() or not np.array_equal(np.array(Image.open(path).convert("RGB")), np.array(preview)):
            preview.save(path, optimize=True)
            changed.append(stage["image"])
    return changed
