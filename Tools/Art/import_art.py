#!/usr/bin/env python3
"""Imports externally generated art into Assets/ (docs/art/asset-spec.md).

Pose references from the current atlas (1024x1024 per frame, green key, fixed origin):
    python3 Tools/Art/import_art.py refs dee build/refs/dee

Fighter frames (one PNG per frame, named after the frame, e.g. idle_0.png):
    python3 Tools/Art/import_art.py fighter dee path/to/frames --origin 512 940 --height 130

Stage backdrop or portrait (any size; scaled and centre-cropped):
    python3 Tools/Art/import_art.py image path/to/pier.png Assets/Stages/PierNine/pier_nine.png --size 640 360
    python3 Tools/Art/import_art.py image path/to/dee_face.png Assets/Fighters/Dee/dee_portrait.png --size 32 32

After importing, run the build (AssetPacker validates the result) or
`ctest -R IronRingTests`; open the game with -training to check hitboxes.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart.importer import FighterImport, export_references, import_fighter, import_image  # noqa: E402

ROOT = HERE.parent.parent


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--assets", type=Path, default=ROOT / "Assets")
    sub = parser.add_subparsers(dest="kind", required=True)

    f = sub.add_parser("fighter", help="frames -> <id>_atlas.png/.json")
    f.add_argument("fighter", help="fighter id, e.g. dee")
    f.add_argument("source", type=Path, help="folder of <frame>.png images")
    f.add_argument("--origin", type=float, nargs=2, required=True, metavar=("X", "Y"),
                   help="registration point (feet on the ground) in source pixels, same for every frame")
    group = f.add_mutually_exclusive_group(required=True)
    group.add_argument("--height", type=int, help="game-pixel height of the reference frame (idle_0)")
    group.add_argument("--scale", type=float, help="explicit source->game scale factor")
    f.add_argument("--manifest", type=Path, help="JSON with per-frame origin/partner overrides")
    f.add_argument("--no-quantize", action="store_true", help="keep colours instead of snapping to the palette")
    f.add_argument("--tolerance", type=int, default=40, help="background key tolerance")

    r = sub.add_parser("refs", help="atlas -> per-frame pose references for outside generation")
    r.add_argument("fighter")
    r.add_argument("target", type=Path)
    r.add_argument("--scale", type=int, default=4, help="upscale factor (default 4: 1024x1024 images)")

    i = sub.add_parser("image", help="one image -> exact size (stage, portrait)")
    i.add_argument("source", type=Path)
    i.add_argument("target", type=Path)
    i.add_argument("--size", type=int, nargs=2, required=True, metavar=("W", "H"))
    i.add_argument("--no-quantize", action="store_true")

    args = parser.parse_args()
    if args.kind == "fighter":
        import_fighter(FighterImport(fighter=args.fighter, source=args.source, origin=tuple(args.origin),
                                     scale=args.scale, height=args.height, quantize=not args.no_quantize,
                                     tolerance=args.tolerance, manifest=args.manifest), args.assets)
    elif args.kind == "refs":
        export_references(args.fighter, args.assets, args.target, args.scale)
    else:
        import_image(args.source, args.target, tuple(args.size), not args.no_quantize)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
