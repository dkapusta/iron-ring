#!/usr/bin/env python3
"""One-off: repairs the clipped spectator groups in Art Pack v2's crowd layers.

Each crowd layer is 768 px wide: six 128 px groups side by side, with 8 frames
of 84 px stacked vertically (asset spec §3.1). In the pack, groups 2, 3 and 5
came out clipped in every venue: a flat band of shoulders with the heads cut
off. The barrier in front of them (rail, rope, bench) is fine.

Each clipped group is rebuilt from an intact group of the same venue and the
same frame, so the rest and cheer animation stays in step:

- **The bodies.** Above the clipped band's lower gap, the donor group is used.
  From the gap down to the barrier, the original pixels (rail, rope) go over
  the donor's bodies. Below that, the original barrier is unchanged.
- **The air above the heads.** This is redone for the whole frame: each raised
  arm or head top belongs to the group where it meets the band below, and moves
  with that group. An arm reaching into a neighbouring group comes along whole,
  and the clipped groups' own arms disappear, as does an arm left hanging in
  the air without a body under it.

The donors give the order 0 1 4 0 4 1, so no group stands next to itself.
The stage previews are recomposed from the repaired layers. Layers already
repaired are left alone, so running the script twice changes
nothing.

    python3 Tools/Art/repair_crowds.py            # repair Assets/ in place
    python3 Tools/Art/repair_crowds.py --check    # exit 1 if a layer still needs it
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ironart.preview import compose_previews  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
GROUPS, GROUP_W, FRAME_H, FRAMES = 6, 128, 84, 8
DONORS = {2: 4, 3: 0, 5: 1}  # clipped group -> intact group
# First row of the barrier that no spectator reaches (the people end above it).
BARRIER = {
    "Stages/Foundry/foundry_crowd.png": 66,
    "Stages/PierNine/pier_nine_crowd.png": 66,
    "Stages/LanternRow/lantern_row_crowd.png": 65,
    "Stages/TheCrown/the_crown_crowd.png": 60,
}
HEAD_ROWS = 37  # an intact group has spectator pixels above this row
AIR = 34        # rows above this are arms and head tops, owned by the group where they meet row AIR


def group(frame: np.ndarray, index: int) -> np.ndarray:
    return frame[:, index * GROUP_W:(index + 1) * GROUP_W]


def clipped(tile: np.ndarray) -> bool:
    return not (tile[:HEAD_ROWS, :, 3] > 0).any()


def rebuild_air(source: np.ndarray, frame: np.ndarray, sources: list[int]) -> None:
    """Redraws rows 0..AIR of `frame`; every piece of `source` there moves with its group."""
    frame[:AIR] = 0
    labels, _ = ndimage.label(source[:AIR, :, 3] > 0, structure=np.ones((3, 3)))
    width = frame.shape[1]
    for index, piece in enumerate(ndimage.find_objects(labels), start=1):
        foot = np.where(labels[AIR - 1] == index)[0]
        if not foot.size:
            continue  # attached to nothing below
        owner = int(foot.mean()) // GROUP_W
        ys, xs = np.nonzero(labels[piece] == index)
        ys, xs = ys + piece[0].start, xs + piece[1].start
        for target, origin in enumerate(sources):
            if origin != owner:
                continue
            tx = xs + (target - owner) * GROUP_W
            inside = (tx >= 0) & (tx < width)
            frame[ys[inside], tx[inside]] = source[ys[inside], xs[inside]]


def repair(pixels: np.ndarray, barrier: int) -> tuple[np.ndarray, int]:
    out = pixels.copy()
    fixed = 0
    for f in range(FRAMES):
        frame = out[f * FRAME_H:(f + 1) * FRAME_H]
        source = pixels[f * FRAME_H:(f + 1) * FRAME_H]
        rest = pixels[(f % (FRAMES // 2)) * FRAME_H:][:FRAME_H]  # cheering arms would hide a clipped top
        sources = list(range(GROUPS))
        for bad, donor in DONORS.items():
            if not clipped(group(rest, bad)) or clipped(group(rest, donor)):
                continue
            tile = group(source, bad)
            opaque_rows = (tile[:, :, 3] > 0).any(1)
            band = np.where(opaque_rows[AIR:barrier])[0] + AIR
            gap = next(y for y in range(int(band.min()), barrier) if not opaque_rows[y])
            result = group(source, donor).copy()
            keep = tile[gap:barrier, :, 3] > 0
            result[gap:barrier][keep] = tile[gap:barrier][keep]
            result[barrier:] = tile[barrier:]
            group(frame, bad)[:] = result
            sources[bad] = donor
            fixed += 1
        if sources != list(range(GROUPS)):
            rebuild_air(source, frame, sources)
            drop_stray_arms(frame, barrier)
    return out, fixed


def drop_stray_arms(frame: np.ndarray, barrier: int) -> None:
    """Clears raised arms left hanging: an arm drawn at a group's edge that
    belonged to the clipped neighbour's (cut-off) body, and now reaches no one."""
    labels, _ = ndimage.label(frame[:barrier, :, 3] > 0, structure=np.ones((3, 3)))
    for index, piece in enumerate(ndimage.find_objects(labels), start=1):
        if piece[0].start < AIR and piece[0].stop <= HEAD_ROWS + 4:
            frame[:barrier][labels == index] = 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--assets", type=Path, default=ROOT / "Assets")
    parser.add_argument("--check", action="store_true", help="report only; exit 1 if any group is clipped")
    args = parser.parse_args()
    pending = 0
    for relative, barrier in BARRIER.items():
        path = args.assets / relative
        pixels = np.array(Image.open(path).convert("RGBA"))
        if pixels.shape[:2] != (FRAME_H * FRAMES, GROUP_W * GROUPS):
            print(f"{relative}: unexpected size {pixels.shape[1]}x{pixels.shape[0]}", file=sys.stderr)
            return 2
        repaired, fixed = repair(pixels, barrier)
        pending += fixed
        if fixed and not args.check:
            Image.fromarray(repaired, "RGBA").save(path, optimize=True)
        state = "clipped" if args.check or not fixed else "repaired"
        print(f"{relative}: {fixed} group frame(s) {state}")
    if pending and not args.check:
        for image in compose_previews(args.assets):
            print(f"{image}: preview recomposed")
    return 1 if args.check and pending else 0


if __name__ == "__main__":
    sys.exit(main())
