#!/usr/bin/env python3
"""Generates Iron Ring's procedural art into Assets/ (and the Windows icon).

    python3 Tools/Art/generate.py                 # every part the generator still owns
    python3 Tools/Art/generate.py effects font    # selected parts
    python3 Tools/Art/generate.py --sheet out.png # also write a procedural fighter contact sheet

Fighters and stages are externally authored now (ironart/ownership.py); their
procedural versions remain as pose references and are written only with
--assets pointing somewhere other than Assets/ (or with --overwrite-external).

Deterministic: the same code produces the same pixels. See docs/art/asset-spec.md
for the contract these assets follow, and Tools/Art/README.md for workflow.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart import effects, fighters, font, icon, palette, stages, ui  # noqa: E402

ROOT = HERE.parent.parent
PARTS = ("palette", "fighters", "stages", "effects", "font", "ui", "icon")
REPLACED = ("fighters", "stages")  # externally authored in Assets/


def contact_sheet(path: Path) -> None:
    from PIL import Image, ImageDraw

    from ironart.characters import CHARACTERS
    from ironart.figure import Canvas
    from ironart.poses import PoseBook

    cell_w, cell_h, cols = 200, 200, 10
    sheets = []
    for character in CHARACTERS.values():
        frames = PoseBook(character).frames()
        rows = (len(frames) + cols - 1) // cols
        sheet = Image.new("RGBA", (cell_w * cols, cell_h * rows), (34, 40, 62, 255))
        draw = ImageDraw.Draw(sheet)
        for i, (name, pose) in enumerate(frames.items()):
            canvas = Canvas(cell_w, cell_h - 12, (90, cell_h - 18))
            character.draw(canvas, pose)
            x, y = (i % cols) * cell_w, (i // cols) * cell_h
            sheet.alpha_composite(canvas.render(), (x, y))
            draw.text((x + 3, y + cell_h - 12), name, fill=(230, 220, 190))
        sheets.append(sheet)
    out = Image.new("RGBA", (max(s.width for s in sheets), sum(s.height for s in sheets)))
    y = 0
    for s in sheets:
        out.paste(s, (0, y))
        y += s.height
    out.save(path)
    print(f"contact sheet: {path}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("parts", nargs="*", choices=PARTS, help="parts to generate (default: all)")
    parser.add_argument("--assets", type=Path, default=ROOT / "Assets")
    parser.add_argument("--sheet", type=Path, help="write a fighter contact sheet PNG")
    parser.add_argument("--overwrite-external", action="store_true",
                        help="allow writing procedural fighters/stages over the external art in Assets/")
    args = parser.parse_args()
    into_assets = args.assets.resolve() == (ROOT / "Assets").resolve()
    parts = args.parts or [p for p in PARTS if not (into_assets and p in REPLACED)]
    blocked = [p for p in parts if p in REPLACED and into_assets and not args.overwrite_external]
    if blocked:
        parser.error(f"{', '.join(blocked)}: Assets/ holds external art there; use --assets <folder> "
                     "for the procedural versions, or --overwrite-external")

    if "palette" in parts:
        palette.write_palette_files(args.assets / "Data")
    if "fighters" in parts:
        fighters.generate(args.assets)
    if "stages" in parts:
        stages.generate(args.assets)
    if "effects" in parts:
        effects.generate(args.assets)
    if "font" in parts:
        font.generate(args.assets)
    if "ui" in parts:
        ui.generate(args.assets)
    if "icon" in parts:
        icon.generate(args.assets, ROOT / "Source" / "Launcher" / "Resources" / "IronRing.ico")
    if args.sheet:
        contact_sheet(args.sheet)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
