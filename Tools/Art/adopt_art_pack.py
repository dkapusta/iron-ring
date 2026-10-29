#!/usr/bin/env python3
"""Re-registers the fighter sprites of an external art pack (Iron Ring Art Pack v1).

The pack fitted every raw sprite to the bounding box of the matching
procedural frame, which scales each frame differently (up to 37 % narrower,
-11..+18 % in height): characters pulse in size between frames. This script
re-imports the pack's isolated raw sprites (`Source/RawFrames/<fighter>/`)
the way docs/art/asset-spec.md asks instead:

- one scale per fighter, from the idle height (Dee 130 px, Bram 134 px);
- vertically, each frame keeps its old bottom edge relative to the ground
  (feet on the floor, airborne frames at their height);
- horizontally, grounded frames align the centre of their feet with the old
  frame's feet (no sliding); other frames keep the old proportional pivot.

Output: one 256x256 frame per pose with the origin at (128, 232) in
ArtSource/Fighters/<Name>/frames/ plus the partner manifest. Those frames are
the editable source; `import_art.py fighter ... --scale 1` packs them.

    python3 Tools/Art/adopt_art_pack.py path/to/IronRing_Art_Pack_v1
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart.characters import CHARACTERS  # noqa: E402
from ironart.fighters import render_frames  # noqa: E402
from ironart.importer import (REFERENCE_CANVAS, REFERENCE_ORIGIN, downscale, line_colours,  # noqa: E402
                              snap_to_palette)

ROOT = HERE.parent.parent
IDLE_HEIGHT = {"dee": 130, "bram": 134}
FEET_ROWS = 4  # rows above the bottom edge that count as "the feet"


def feet_centre(alpha: np.ndarray) -> float | None:
    """Horizontal centre of the opaque pixels in the bottom rows, or None if empty."""
    rows = alpha[-FEET_ROWS:]
    xs = np.nonzero(rows.any(axis=0))[0]
    return None if len(xs) == 0 else (xs.min() + xs.max()) / 2


def fighter_palette(frames: list[Image.Image], size: int) -> np.ndarray:
    """A limited palette fitted to one fighter's frames (median cut + k-means)."""
    pixels = np.concatenate([np.array(f)[..., :3][np.array(f)[..., 3] > 0] for f in frames])
    strip = Image.fromarray(pixels.reshape(1, -1, 3).astype(np.uint8), "RGB")
    quantized = strip.quantize(colors=size, method=Image.Quantize.MEDIANCUT, kmeans=8)
    flat = quantized.getpalette()[: size * 3]
    return np.array(sorted({tuple(flat[i:i + 3]) for i in range(0, len(flat), 3)}), dtype=np.int32)


def adopt(pack: Path, fighter: str, colours_per_fighter: int) -> Path:
    """colours_per_fighter: 0 keeps the generated colours, otherwise the palette size."""
    name = fighter.capitalize()
    contract = json.loads((pack / "Source" / "Contracts" / f"{fighter}_atlas.json").read_text())
    # The procedural frames are the poses the pack was painted over (pose references).
    reference = {f.name: f for f in render_frames(CHARACTERS[fighter])[0]}
    raw_dir = pack / "Source" / "RawFrames" / fighter
    box = Image.open(raw_dir / "idle_0.png").convert("RGBA").getbbox()
    scale = IDLE_HEIGHT[fighter] / (box[3] - box[1])

    smalls = {}
    for frame in contract["frames"]:
        raw = Image.open(raw_dir / f"{frame['name']}.png").convert("RGBA")
        smalls[frame["name"]] = downscale(raw.crop(raw.getbbox()), scale)
    if colours_per_fighter:
        colours = fighter_palette(list(smalls.values()), colours_per_fighter)
        outline = line_colours(fighter)
        smalls = {k: snap_to_palette(v, colours, outline) for k, v in smalls.items()}

    target = ROOT / "ArtSource" / "Fighters" / name / "frames"
    target.mkdir(parents=True, exist_ok=True)
    for frame in contract["frames"]:
        _, _, w, h = frame["source_rect"]
        px, py = frame["pivot_pixels"]
        small = smalls[frame["name"]]
        small = small.crop(small.getbbox())
        sw, sh = small.size
        # Vertical: keep the old bottom edge relative to the ground.
        new_py = py - h + sh
        # Horizontal: feet on the old feet when the old frame stood on the ground.
        new_px = px / w * sw
        if py == h:
            old_alpha = np.array(reference[frame["name"]].image)[..., 3]
            old_feet, new_feet = feet_centre(old_alpha), feet_centre(np.array(small)[..., 3])
            if old_feet is not None and new_feet is not None:
                new_px = px - old_feet + new_feet
        canvas = Image.new("RGBA", REFERENCE_CANVAS, (0, 0, 0, 0))
        ox, oy = round(REFERENCE_ORIGIN[0] - new_px), round(REFERENCE_ORIGIN[1] - new_py)
        if ox < 0 or oy < 0 or ox + sw > REFERENCE_CANVAS[0] or oy + sh > REFERENCE_CANVAS[1]:
            raise SystemExit(f"{fighter}/{frame['name']}: does not fit the {REFERENCE_CANVAS} canvas")
        canvas.alpha_composite(small, (ox, oy))
        canvas.save(target / f"{frame['name']}.png", optimize=True)

    partners = {f["name"]: {"partner": f["partner"]} for f in contract["frames"] if "partner" in f}
    (target / "manifest.json").write_text(json.dumps(partners, indent=2) + "\n")
    print(f"{fighter}: scale {scale:.4f}, {len(smalls)} frames, "
          f"{colours_per_fighter or 'all'} colours -> {target.relative_to(ROOT)}")
    return target


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("pack", type=Path, help="extracted art pack folder")
    parser.add_argument("--colours", type=int, default=48,
                        help="palette size per fighter, fitted to its frames (0 keeps all colours)")
    args = parser.parse_args()
    for fighter in IDLE_HEIGHT:
        adopt(args.pack, fighter, args.colours)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
