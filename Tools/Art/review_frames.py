#!/usr/bin/env python3
"""Reviews a fighter's frames against the animation rules in docs/art/asset-spec.md §2.6.

    python3 Tools/Art/review_frames.py                    # both fighters, report only
    python3 Tools/Art/review_frames.py --sheets build/review   # also per-animation sheets

Sheets show every animation on the ground line with its hurtboxes (cyan; a
move's per-phase boxes where it defines them), the active hitboxes (red) and
throw partner points (yellow), at 3x. The report
lists measurable problems; it complements, not replaces, an animator's eye.
Exit code is always 0: findings are notes for the next art pass, not build errors.
"""

from __future__ import annotations

import argparse
import json
import sys
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from ironart.importer import edge_mask  # noqa: E402

ROOT = HERE.parent.parent
ASSETS = ROOT / "Assets"

# Thresholds (asset-spec §2.6).
IDLE_BOB_MIN = 2            # px of head travel over the idle loop (spec: 0, -1, -2, -1)
LOOP_DRIFT_MAX = 3          # px the body centre may drift between loop frames
KEY_CHANGE_MIN = 0.12       # share of silhouette pixels that must change between key frames of a move
HIT_REACH_MIN = 6           # px the striking limb must reach into the hitbox
LIGHT_EDGE_MAX = 0.04       # share of silhouette edge pixels brighter than the outline rule allows
TEAL_EDGE_MAX = 0.05        # share of cyan rim pixels on a fighter without teal costume parts
CROUCH_SLACK = 4            # px of hair/skull allowed above the crouch hurtbox


@dataclass
class Frame:
    name: str
    mask: np.ndarray   # opaque pixels, trimmed
    rgba: np.ndarray
    px: int
    py: int
    partner: tuple[int, int] | None

    def world(self) -> tuple[np.ndarray, np.ndarray]:
        """Opaque pixel coordinates in world space: x forward from the origin, y up from the ground."""
        ys, xs = np.nonzero(self.mask)
        return xs - self.px, self.py - ys

    @property
    def height(self) -> int:
        return int(self.py - np.nonzero(self.mask)[0].min())


def load(fighter: str) -> tuple[dict, dict[str, Frame]]:
    folder = ASSETS / "Fighters" / fighter.capitalize()
    definition = json.loads((folder / f"{fighter}.fighter.json").read_text())
    atlas = json.loads((folder / f"{fighter}_atlas.json").read_text())
    sheet = np.array(Image.open(folder / atlas["image"]).convert("RGBA"))
    frames = {}
    for f in atlas["frames"]:
        x, y, w, h = f["source_rect"]
        rgba = sheet[y:y + h, x:x + w]
        frames[f["name"]] = Frame(f["name"], rgba[..., 3] > 0, rgba, *f["pivot_pixels"],
                                  tuple(f["partner"]) if "partner" in f else None)
    return definition, frames


def active_frames(definition: dict) -> dict[str, list[tuple[str, list[int]]]]:
    """frame name -> [(move key, hitbox)] for frames shown during a move's active window."""
    out: dict[str, list[tuple[str, list[int]]]] = {}
    for key, move in definition["moves"].items():
        t = 0
        for name, ticks, *_ in definition["animations"][move["animation"]]["frames"]:
            if t < move["startup"] + move["active"] and t + ticks > move["startup"]:
                out.setdefault(name, []).append((key, move["hitbox"]))
            t += ticks
    return out


def phase_hurtboxes(definition: dict) -> dict[tuple[str, int], list[list[int]]]:
    """(animation, frame index) -> the move's hurtboxes for the phase that frame is shown in."""
    out: dict[tuple[str, int], list[list[int]]] = {}
    for move in definition["moves"].values():
        hurt = move.get("hurt")
        if not hurt:
            continue
        t = 0
        for index, (_, ticks, *_) in enumerate(definition["animations"][move["animation"]]["frames"]):
            phase = ("startup" if t < move["startup"] else
                     "active" if t < move["startup"] + move["active"] else "recovery")
            if phase in hurt:
                out[(move["animation"], index)] = hurt[phase]
            t += ticks
    return out


def change(a: Frame, b: Frame) -> float:
    """Share of silhouette pixels that differ between two frames, aligned on the origin."""
    ax, ay = a.world()
    bx, by = b.world()
    sa, sb = set(zip(ax.tolist(), ay.tolist())), set(zip(bx.tolist(), by.tolist()))
    return len(sa ^ sb) / max(1, len(sa | sb))


def review(fighter: str, teal_costume: bool) -> list[str]:
    d, frames = load(fighter)
    notes: list[str] = []
    anims = d["animations"]

    idle = [frames[n] for n, *_ in anims["idle"]["frames"]]
    heights = [f.height for f in idle]
    if max(heights) - min(heights) < IDLE_BOB_MIN:
        notes.append(f"idle: no breathing bob (head height {heights}); spec asks 0, -1, -2, -1 px")
    for anim in ("idle", "walk"):
        loop = [frames[n] for n, *_ in anims[anim]["frames"]]
        centres = [float(f.world()[0].mean()) for f in loop]
        for a, b, ca, cb in zip(loop, loop[1:] + loop[:1], centres, centres[1:] + centres[:1]):
            if abs(cb - ca) > LOOP_DRIFT_MAX:
                notes.append(f"{anim}: body centre jumps {cb - ca:+.1f} px from {a.name} to {b.name} "
                             f"(max {LOOP_DRIFT_MAX})")
    walk = [frames[n] for n, *_ in anims["walk"]["frames"]]
    if max(f.height for f in walk) - min(f.height for f in walk) < 1:
        notes.append("walk: no vertical bob; the body should dip on contact frames")

    for key, move in d["moves"].items():
        names = []
        for n, *_ in anims[move["animation"]]["frames"]:
            if n not in names:
                names.append(n)
        for a, b in zip(names, names[1:]):
            c = change(frames[a], frames[b])
            if c < KEY_CHANGE_MIN:
                notes.append(f"{key}: {a} -> {b} changes only {100 * c:.0f}% of the silhouette "
                             f"(key poses should differ by {100 * KEY_CHANGE_MIN:.0f}%+)")
    for name, hits in active_frames(d).items():
        xs, ys = frames[name].world()
        for key, (bx, by, bw, bh) in hits:
            if d["moves"][key].get("kind") == "throw":
                continue
            inside = (xs >= bx) & (xs < bx + bw) & (ys >= by) & (ys < by + bh)
            reach = int(xs[inside].max() - bx) if inside.any() else -1
            if reach < HIT_REACH_MIN:
                notes.append(f"{key}: active frame {name} reaches only {reach} px into the hitbox "
                             f"[{bx}, {by}, {bw}, {bh}] (min {HIT_REACH_MIN})")

    body = d["body"]
    for name in ("crouch", "guard_crouch", "hit_crouch"):
        if frames[name].height > body["crouchHurt"][3] + CROUCH_SLACK:
            notes.append(f"{name}: {frames[name].height} px tall, crouch hurtbox is {body['crouchHurt'][3]} "
                         f"(+{CROUCH_SLACK} allowed)")
    for name, a, b in (("hit", "hit_0", "hit_1"), ("win", "win_0", "win_1")):
        c = change(frames[a], frames[b])
        if c < KEY_CHANGE_MIN:
            notes.append(f"{name}: {a} and {b} differ by only {100 * c:.0f}%")

    edges = np.concatenate([f.rgba[edge_mask(f.rgba[..., 3])][:, :3] for f in frames.values()]).astype(int)
    r, g, b = edges.T
    light = (0.3 * r + 0.59 * g + 0.11 * b) > 90
    teal = (b > r + 40) & (g > r + 30)
    if light.mean() > LIGHT_EDGE_MAX:
        notes.append(f"outline: {100 * light.mean():.0f}% of silhouette edge pixels are light (max "
                     f"{100 * LIGHT_EDGE_MAX:.0f}%); the outline should be the material's dark line colour")
    if not teal_costume and teal.mean() > TEAL_EDGE_MAX:
        notes.append(f"outline: {100 * teal.mean():.0f}% of edge pixels are a cyan rim from the generator "
                     f"(max {100 * TEAL_EDGE_MAX:.0f}%)")
    return notes


def sheets(fighter: str, target: Path, zoom: int = 3) -> None:
    d, frames = load(fighter)
    hits = active_frames(d)
    rows = [(name, [f[0] for f in anim["frames"]]) for name, anim in d["animations"].items()]
    phase_hurt = phase_hurtboxes(d)
    cw, ch, ox, oy = 180, 200, 60, 180
    target.mkdir(parents=True, exist_ok=True)
    for page in range(0, len(rows), 7):
        part = rows[page:page + 7]
        width = max(len(r[1]) for r in part)
        sheet = Image.new("RGBA", (cw * width * zoom, ch * len(part) * zoom), (34, 38, 58, 255))
        for r, (anim, names) in enumerate(part):
            for c, name in enumerate(names):
                hurt_boxes = phase_hurt.get((anim, c))
                f = frames[name]
                tile = Image.new("RGBA", (cw, ch), (34, 38, 58, 255))
                draw = ImageDraw.Draw(tile)
                draw.line([(0, oy), (cw, oy)], fill=(90, 100, 130, 255))
                default = d["body"]["crouchHurt" if "crouch" in name else "standHurt"]
                for hb in hurt_boxes or [default]:
                    draw.rectangle([ox + hb[0], oy - hb[1] - hb[3], ox + hb[0] + hb[2], oy - hb[1]],
                                   outline=(60, 200, 200))
                tile.alpha_composite(Image.fromarray(f.rgba), (ox - f.px, oy - f.py))
                for _, (bx, by, bw, bh) in hits.get(name, []):
                    draw.rectangle([ox + bx, oy - by - bh, ox + bx + bw, oy - by], outline=(240, 70, 60))
                if f.partner:
                    draw.ellipse([ox + f.partner[0] - 2, oy - f.partner[1] - 2, ox + f.partner[0] + 2,
                                  oy - f.partner[1] + 2], fill=(255, 220, 0))
                draw.text((3, 2), f"{anim}:{name}", fill=(240, 230, 200))
                sheet.alpha_composite(tile.resize((cw * zoom, ch * zoom), Image.NEAREST), (c * cw * zoom, r * ch * zoom))
        sheet.save(target / f"{fighter}_{page // 7}.png")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("fighters", nargs="*", default=["dee", "bram"])
    parser.add_argument("--sheets", type=Path, help="write per-animation review sheets here")
    args = parser.parse_args()
    teal = {"dee": True}
    for fighter in args.fighters:
        notes = review(fighter, teal.get(fighter, False))
        print(f"== {fighter}: {len(notes)} finding(s)")
        for note in notes:
            print("  - " + note)
        if args.sheets:
            sheets(fighter, args.sheets)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
