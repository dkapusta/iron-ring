#!/usr/bin/env python3
"""Recomposes every layered stage's 640 x 360 preview from its layers.

    python3 Tools/Art/compose_previews.py                   # Assets/
    python3 Tools/Art/compose_previews.py --assets <folder> # another Assets-shaped folder

Run it after editing a stage layer; `selftest.py` fails while a preview and its
layers disagree. How a preview is composed: `ironart/preview.py`.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart.preview import compose_previews  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--assets", type=Path, default=HERE.parent.parent / "Assets")
    args = parser.parse_args()
    changed = compose_previews(args.assets)
    for image in changed:
        print(f"{image}: recomposed")
    print(f"{len(changed)} preview(s) changed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
