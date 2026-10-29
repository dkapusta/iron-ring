#!/usr/bin/env python3
"""Checks the art tools (run in CI; also handy after editing Tools/Art).

1. The committed Assets/ match what generate.py produces (pixels and JSON),
   so nobody edits generated files by hand or forgets to regenerate.
2. Every fighter survives export -> import unchanged (pixels, pivots, partners),
   which proves the importer's keying, scaling and packing are lossless for
   art that is already game-ready.

PNG bytes are not compared: compression differs between Pillow/zlib versions.
"""

from __future__ import annotations

import json
import sys
import tempfile
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart import effects, fighters, font, palette, stages  # noqa: E402
from ironart.characters import CHARACTERS  # noqa: E402
from ironart.importer import (REFERENCE_ORIGIN, FighterImport, export_references,  # noqa: E402
                              import_fighter)

ROOT = HERE.parent.parent
ASSETS = ROOT / "Assets"


def same_png(a: Path, b: Path) -> bool:
    return np.array_equal(np.array(Image.open(a).convert("RGBA")), np.array(Image.open(b).convert("RGBA")))


def frames(atlas_json: Path) -> dict[str, tuple]:
    data = json.loads(atlas_json.read_text())
    sheet = np.array(Image.open(atlas_json.parent / data["image"]).convert("RGBA"))
    out = {}
    for f in data["frames"]:
        x, y, w, h = f["source_rect"]
        out[f["name"]] = (sheet[y:y + h, x:x + w].tobytes(), (w, h), tuple(f["pivot_pixels"]),
                          tuple(f.get("partner", ())))
    return out


def check_generated(work: Path) -> list[str]:
    palette.write_palette_files(work / "Data")
    fighters.generate(work)
    stages.generate(work)
    effects.generate(work)
    font.generate(work)
    errors = []
    for produced in sorted(work.rglob("*")):
        if produced.is_dir():
            continue
        relative = produced.relative_to(work)
        committed = ASSETS / relative
        if not committed.exists():
            errors.append(f"{relative}: generated but not committed")
        elif produced.suffix == ".png":
            if not same_png(produced, committed):
                errors.append(f"{relative}: pixels differ from generate.py output (regenerate)")
        elif produced.read_bytes() != committed.read_bytes():
            errors.append(f"{relative}: differs from generate.py output (regenerate)")
    return errors


def check_round_trip(work: Path) -> list[str]:
    errors = []
    scale = 4
    for fighter in CHARACTERS:
        refs = work / "refs" / fighter
        export_references(fighter, ASSETS, refs, scale)
        target = work / "assets"
        (target / "Fighters" / fighter.capitalize()).mkdir(parents=True, exist_ok=True)
        import_fighter(FighterImport(fighter=fighter, source=refs,
                                     origin=(REFERENCE_ORIGIN[0] * scale, REFERENCE_ORIGIN[1] * scale),
                                     scale=1 / scale, manifest=refs / "manifest.json"), target)
        original = frames(ASSETS / "Fighters" / fighter.capitalize() / f"{fighter}_atlas.json")
        imported = frames(target / "Fighters" / fighter.capitalize() / f"{fighter}_atlas.json")
        for name, value in original.items():
            if imported.get(name) != value:
                errors.append(f"{fighter}: frame '{name}' changed in the export/import round trip")
    return errors


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        errors = check_generated(Path(tmp) / "generated") + check_round_trip(Path(tmp) / "roundtrip")
    for e in errors:
        print("FAIL: " + e)
    print("art selftest: " + ("FAILED" if errors else "ok"))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
