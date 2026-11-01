#!/usr/bin/env python3
"""Puts portraits on the shared backdrop (docs/art/final-polish-brief.md §2).

    python3 Tools/Art/portrait_backdrop.py Assets/Fighters/Dee/dee_select.png ...
    python3 Tools/Art/portrait_backdrop.py --all        # every <id>_portrait.png and <id>_select.png

Every pixel outside the figure becomes the template's pixel at that position
(docs/art/templates/portrait_backdrop_<size>.png). The figure is what the
backdrop does not reach:
- the backdrop is the `night_1` area connected to the top and side edges;
- plus any small decoration inside it (the old dot grid, a corner smudge);
- plus single decoration pixels left against the outline.
The side-colour band at the bottom of a select portrait is kept. Review the
result at 4x: a figure whose outline touches `night_1` can lose pixels.
"""

from __future__ import annotations

import argparse
import collections
import sys
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

ROOT = Path(__file__).resolve().parents[2]
TEMPLATES = ROOT / "docs" / "art" / "templates"
NIGHT_1 = (0x14, 0x1A, 0x30)
CORNER = {32: 4, 48: 6}  # the top-left square every portrait must share with its template


def template(size: int) -> np.ndarray:
    return np.asarray(Image.open(TEMPLATES / f"portrait_backdrop_{size}.png").convert("RGBA"))


def band_rows(rgb: np.ndarray) -> int:
    """Rows of one colour at the bottom: a select portrait's side-colour band."""
    h, w = rgb.shape[:2]
    rows = 0
    for y in range(h - 1, h - 8, -1):
        _, count = collections.Counter(tuple(p) for p in rgb[y]).most_common(1)[0]
        if count < w * 0.9:
            break
        rows += 1
    return rows


def backdrop_mask(a: np.ndarray, select: bool) -> np.ndarray:
    rgb = a[..., :3]
    h, w = rgb.shape[:2]
    labels, _ = ndimage.label(np.all(rgb == NIGHT_1, axis=2))
    edge = set(labels[0][labels[0] > 0]) | set(labels[:, 0][labels[:, 0] > 0]) | set(labels[:, -1][labels[:, -1] > 0])
    base = np.isin(labels, list(edge))
    mask = base.copy()
    decoration: set[tuple[int, ...]] = set()
    rest, count = ndimage.label(~base)
    corner = int(w * 0.45)
    for i in range(1, count + 1):
        comp = rest == i
        ys, xs = np.nonzero(comp)
        ring = ndimage.binary_dilation(comp) & ~comp
        enclosed = comp.sum() <= 40 and ring.any() and base[ring].mean() > 0.99
        corner_blob = (ys.min() == 0 or xs.min() == 0) and ys.max() < corner and xs.max() < corner
        if enclosed or corner_blob:
            mask |= comp
            decoration |= {tuple(c) for c in rgb[comp]}
    # Decoration pixels left against the outline: at least two backdrop neighbours.
    for y, x in zip(*np.nonzero(~mask)):
        if tuple(rgb[y, x]) not in decoration:
            continue
        near = [mask[yy, xx] for yy, xx in ((y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)) if 0 <= yy < h and 0 <= xx < w]
        if sum(near) >= 2:
            mask[y, x] = True
    if select:
        rows = band_rows(rgb)
        if rows:
            mask[h - rows:] = False
    return mask


def apply(path: Path) -> int:
    a = np.asarray(Image.open(path).convert("RGBA")).copy()
    t = template(a.shape[0])
    mask = backdrop_mask(a, path.stem.endswith("_select"))
    a[mask] = t[mask]
    Image.fromarray(a).save(path)
    return int(mask.sum())


def portraits() -> list[Path]:
    fighters = ROOT / "Assets" / "Fighters"
    return sorted(list(fighters.glob("*/*_portrait.png")) + list(fighters.glob("*/*_select.png")))


def check() -> list[str]:
    """Portraits whose top-left corner differs from the template (a different backdrop)."""
    errors = []
    for path in portraits():
        a = np.asarray(Image.open(path).convert("RGBA"))
        size = a.shape[0]
        if size not in CORNER:
            errors.append(f"{path.relative_to(ROOT)}: {size} px portraits have no backdrop template")
            continue
        k = CORNER[size]
        if not np.array_equal(a[:k, :k], template(size)[:k, :k]):
            errors.append(f"{path.relative_to(ROOT)}: backdrop differs from docs/art/templates "
                          f"(python3 Tools/Art/portrait_backdrop.py {path.relative_to(ROOT)})")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("files", nargs="*", type=Path)
    parser.add_argument("--all", action="store_true", help="every HUD and select portrait under Assets/Fighters")
    parser.add_argument("--check", action="store_true", help="report portraits that are not on the template")
    args = parser.parse_args()
    if args.check:
        errors = check()
        for e in errors:
            print(e)
        return 1 if errors else 0
    files = portraits() if args.all else args.files
    if not files:
        parser.error("name portraits or pass --all")
    for path in files:
        print(f"{path}: {apply(path)} backdrop pixels")
    return 0


if __name__ == "__main__":
    sys.exit(main())
