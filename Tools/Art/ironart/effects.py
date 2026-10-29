"""Animated hit effects (impact sparks, guard flash, Breakaway ring, tech, slam dust).

Each effect is a short frame sequence; frame names are <effect>_<n> and the
pivot is the effect's centre (bottom centre for ground effects).
"""

from __future__ import annotations

import math
import random
from pathlib import Path

import numpy as np
from PIL import Image

from . import atlas, palette
from .paint import BAYER4


class Fx:
    def __init__(self, size: int, height: int | None = None) -> None:
        self.w, self.h = size, height or size
        self.img = np.zeros((self.h, self.w, 4), dtype=np.uint8)
        self.cx, self.cy = self.w / 2, self.h / 2

    def put(self, x: float, y: float, colour: str) -> None:
        xi, yi = int(math.floor(x)), int(math.floor(y))
        if 0 <= xi < self.w and 0 <= yi < self.h:
            self.img[yi, xi, :3] = palette.rgb(colour)
            self.img[yi, xi, 3] = 255

    def ray(self, angle: float, r0: float, r1: float, colours: list[str], width: float = 1.0) -> None:
        steps = int((r1 - r0) * 2) + 1
        for i in range(steps):
            r = r0 + (r1 - r0) * i / max(steps - 1, 1)
            colour = colours[min(int(i / steps * len(colours)), len(colours) - 1)]
            for w in np.arange(-width / 2 + 0.5, width / 2, 1.0) if width > 1 else (0.0,):
                x = self.cx + r * math.cos(angle) - w * math.sin(angle)
                y = self.cy + r * math.sin(angle) + w * math.cos(angle)
                self.put(x, y, colour)

    def disc(self, r: float, colour: str, dither: float = 1.0, cx: float | None = None, cy: float | None = None,
             squash: float = 1.0) -> None:
        cx = self.cx if cx is None else cx
        cy = self.cy if cy is None else cy
        for y in range(self.h):
            for x in range(self.w):
                if ((x + 0.5 - cx) ** 2 + ((y + 0.5 - cy) / squash) ** 2) <= r * r and BAYER4[y % 4, x % 4] < dither:
                    self.put(x, y, colour)

    def ring(self, r: float, thickness: float, colours: list[str], dither: float = 1.0) -> None:
        for y in range(self.h):
            for x in range(self.w):
                d = math.hypot(x + 0.5 - self.cx, y + 0.5 - self.cy)
                if r - thickness <= d <= r and BAYER4[y % 4, x % 4] < dither:
                    band = int((r - d) / thickness * len(colours))
                    self.put(x, y, colours[min(band, len(colours) - 1)])

    def image(self) -> Image.Image:
        return Image.fromarray(self.img, "RGBA")


def light_hit() -> list[Image.Image]:
    frames = []
    for i, (length, core) in enumerate(((5, 3), (9, 3), (11, 2), (12, 1))):
        f = Fx(28)
        for k in range(4):
            a = k * math.pi / 2 + math.pi / 4 * (i % 2) * 0.2
            f.ray(a, 1, length, ["chalk", "sodium_4", "sodium_3"] if i < 3 else ["sodium_2"], 1)
        for k in range(4):
            f.ray(k * math.pi / 2 + math.pi / 4, 1, length * 0.5, ["sodium_4", "sodium_2"])
        if i < 3:
            f.disc(core, "chalk")
        frames.append(f.image())
    return frames


def heavy_hit() -> list[Image.Image]:
    frames = []
    rng = random.Random(5)
    angles = [k * math.pi / 4 + rng.uniform(-0.15, 0.15) for k in range(8)]
    for i, (length, core, width) in enumerate(((8, 5, 3), (15, 6, 3), (19, 4, 2), (21, 2, 1), (22, 0, 1))):
        f = Fx(48)
        for k, a in enumerate(angles):
            l = length * (1.0 if k % 2 == 0 else 0.65)
            colours = ["chalk", "sodium_4", "coral_3", "coral_2"] if i < 3 else ["coral_3", "coral_2"]
            f.ray(a, max(core - 1, 0), l, colours, width if k % 2 == 0 else 1)
        if core:
            f.disc(core + 1, "coral_3")
            f.disc(core, "sodium_4")
            f.disc(max(core - 2, 1), "chalk")
        if i >= 2:
            for k in range(10):
                a = rng.uniform(0, 2 * math.pi)
                r = rng.uniform(length * 0.6, length * 1.1)
                f.put(f.cx + r * math.cos(a), f.cy + r * math.sin(a), "sodium_3")
        frames.append(f.image())
    return frames


def guard_flash() -> list[Image.Image]:
    """A crescent of teal light on the defender's side (art faces right: attacker on the left)."""
    frames = []
    for i, (r, dither) in enumerate(((5, 1.0), (8, 1.0), (10, 0.75), (11, 0.4))):
        f = Fx(28)
        for y in range(f.h):
            for x in range(f.w):
                dx, dy = x + 0.5 - f.cx, y + 0.5 - f.cy
                d = math.hypot(dx, dy)
                if r - 2.5 <= d <= r and dx < r * 0.3 and BAYER4[y % 4, x % 4] < dither:
                    f.put(x, y, "teal_4" if d > r - 1 else "teal_3")
        if i < 2:
            f.disc(2, "chalk")
        frames.append(f.image())
    return frames


def breakaway_ring() -> list[Image.Image]:
    frames = []
    for i in range(6):
        f = Fx(80)
        r = 8 + i * 6
        f.ring(r, 4 - i * 0.4, ["teal_4", "teal_3", "teal_2"], dither=1.0 - i * 0.13)
        if i < 2:
            f.disc(6 - i * 2, "teal_4")
            f.disc(3 - i, "chalk")
        frames.append(f.image())
    return frames


def throw_tech() -> list[Image.Image]:
    frames = []
    for i, length in enumerate((5, 9, 11, 11)):
        f = Fx(28)
        colours = ["chalk", "teal_4"] if i < 3 else ["teal_3"]
        for k in range(4):
            f.ray(math.pi / 4 + k * math.pi / 2, 0, length, colours, 2 if i < 2 else 1)
        frames.append(f.image())
    return frames


def slam_dust() -> list[Image.Image]:
    """Ground impact: dust puffs spreading sideways, pivot at the bottom centre."""
    frames = []
    rng = random.Random(9)
    puffs = [(rng.uniform(-1, 1), rng.uniform(0.6, 1.0)) for _ in range(9)]
    for i in range(5):
        f = Fx(80, 32)
        spread = 10 + i * 8
        for side, size in puffs:
            x = f.cx + side * spread
            r = size * (6 + i * 2.2)
            dither = 1.0 - i * 0.15
            f.disc(r, "steel_2", dither, cx=x, cy=f.h - r * 0.7, squash=0.7)
            f.disc(r * 0.6, "fog", dither, cx=x - 1, cy=f.h - r * 0.9, squash=0.7)
        if i < 2:
            for k in (-1, 1):
                f.ray(math.pi if k < 0 else 0, 6, 30 + i * 8, ["chalk", "bone"], 1)
        frames.append(f.image())
    return frames


EFFECTS = {
    "light_hit": light_hit,
    "heavy_hit": heavy_hit,
    "guard": guard_flash,
    "breakaway": breakaway_ring,
    "tech": throw_tech,
    "slam": slam_dust,
}


def generate(assets: Path) -> None:
    frames: list[atlas.Frame] = []
    for name, build in EFFECTS.items():
        for i, image in enumerate(build()):
            pivot = (image.width // 2, image.height if name == "slam" else image.height // 2)
            frames.append(atlas.Frame(f"{name}_{i}", image, pivot))
    atlas.write(frames, assets / "Resources" / "FX" / "effects_atlas.png",
                extra={"$comment": "Generated by Tools/Art/generate.py. Effect frames <effect>_<n>, pivot at the "
                                   "effect centre (slam: bottom centre)."})
    print(f"effects: {len(frames)} frames")
