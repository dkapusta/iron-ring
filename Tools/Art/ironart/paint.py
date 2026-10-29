"""Small pixel-painting toolkit for backgrounds and effects: palette-locked
gradients and glows with ordered (Bayer) dithering, aliased shapes."""

from __future__ import annotations

import math
import random

import numpy as np
from PIL import Image, ImageDraw

from . import palette

BAYER4 = (np.array([[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]], dtype=np.float32) + 0.5) / 16


def c(name: str) -> tuple[int, int, int]:
    return palette.rgb(name)


class Painter:
    def __init__(self, width: int, height: int, seed: int) -> None:
        self.w, self.h = width, height
        self.px = np.zeros((height, width, 3), dtype=np.uint8)
        self.alpha = np.full((height, width), 255, dtype=np.uint8)
        self.rng = random.Random(seed)
        ys, xs = np.mgrid[0:height, 0:width]
        self.xs, self.ys = xs, ys
        self.threshold = BAYER4[ys % 4, xs % 4]

    # --- fills ----------------------------------------------------------

    def gradient(self, y0: int, y1: int, stops: list[str]) -> None:
        """Vertical gradient through palette colours, dithered between neighbours."""
        y0, y1 = max(y0, 0), min(y1, self.h)
        n = len(stops) - 1
        for y in range(y0, y1):
            t = (y - y0) / max(y1 - y0 - 1, 1) * n
            i = min(int(t), n - 1)
            frac = t - i
            row_threshold = self.threshold[y]
            pick = np.where(row_threshold < frac, i + 1, i)
            for k in (i, i + 1):
                self.px[y, pick == k] = c(stops[k])

    def rect(self, x0: int, y0: int, x1: int, y1: int, colour: str) -> None:
        x0, y0 = max(x0, 0), max(y0, 0)
        x1, y1 = min(x1, self.w), min(y1, self.h)
        if x0 < x1 and y0 < y1:
            self.px[y0:y1, x0:x1] = c(colour)

    def polygon(self, points: list[tuple[float, float]], colour: str) -> None:
        mask = Image.new("1", (self.w, self.h), 0)
        ImageDraw.Draw(mask).polygon([(round(x), round(y)) for x, y in points], fill=1)
        self.px[np.array(mask, dtype=bool)] = c(colour)

    def line(self, x0: float, y0: float, x1: float, y1: float, colour: str, width: int = 1) -> None:
        mask = Image.new("1", (self.w, self.h), 0)
        ImageDraw.Draw(mask).line([(round(x0), round(y0)), (round(x1), round(y1))], fill=1, width=width)
        self.px[np.array(mask, dtype=bool)] = c(colour)

    def ellipse(self, cx: float, cy: float, rx: float, ry: float, colour: str, outline: str | None = None) -> None:
        mask = Image.new("1", (self.w, self.h), 0)
        box = [round(cx - rx), round(cy - ry), round(cx + rx), round(cy + ry)]
        ImageDraw.Draw(mask).ellipse(box, fill=1)
        self.px[np.array(mask, dtype=bool)] = c(colour)
        if outline:
            m2 = Image.new("1", (self.w, self.h), 0)
            ImageDraw.Draw(m2).ellipse(box, outline=1)
            self.px[np.array(m2, dtype=bool)] = c(outline)

    def pixel(self, x: int, y: int, colour: str) -> None:
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y, x] = c(colour)

    # --- light ------------------------------------------------------------

    def glow(self, cx: float, cy: float, radius: float, ramp: list[str], strength: float = 1.0,
             squash: float = 1.0, mask: np.ndarray | None = None) -> None:
        """Dithered radial light: each ramp step covers pixels whose falloff passes the Bayer threshold.

        Pixels are only ever brightened to a ramp colour, never darkened.
        """
        r = int(radius) + 1
        x0, x1 = max(int(cx) - r, 0), min(int(cx) + r + 1, self.w)
        y0, y1 = max(int(cy - r * squash), 0), min(int(cy + r * squash) + 1, self.h)
        if x0 >= x1 or y0 >= y1:
            return
        xs, ys = self.xs[y0:y1, x0:x1], self.ys[y0:y1, x0:x1]
        d = np.sqrt((xs - cx) ** 2 + ((ys - cy) / squash) ** 2) / radius
        intensity = np.clip(1 - d, 0, 1) ** 1.6 * strength * len(ramp)
        thr = self.threshold[y0:y1, x0:x1]
        level = np.floor(intensity + thr - 0.5).astype(int)
        region = self.px[y0:y1, x0:x1]
        lum = region.astype(np.int32).sum(axis=2)
        for k, name in enumerate(ramp):
            col = np.array(c(name))
            sel = (level >= k + 1) & (lum < col.sum())
            if mask is not None:
                sel &= mask[y0:y1, x0:x1]
            region[sel] = col
            lum = region.astype(np.int32).sum(axis=2)

    def cone(self, apex: tuple[float, float], spread: float, length: float, direction: float, ramp: list[str],
             strength: float = 0.8) -> None:
        """Dithered light cone (floodlight beam). direction in degrees, 90 = straight down."""
        ax, ay = apex
        dx, dy = self.xs - ax, self.ys - ay
        dist = np.sqrt(dx * dx + dy * dy)
        ang = np.degrees(np.arctan2(dy, dx))
        off = np.abs((ang - direction + 180) % 360 - 180)
        inside = (off < spread) & (dist < length)
        falloff = np.clip(1 - dist / length, 0, 1) * np.clip(1 - off / spread, 0, 1) ** 0.7
        intensity = falloff * strength * len(ramp)
        level = np.floor(intensity + self.threshold - 0.5).astype(int)
        lum = self.px.astype(np.int32).sum(axis=2)
        for k, name in enumerate(ramp):
            col = np.array(c(name))
            sel = inside & (level >= k + 1) & (lum < col.sum())
            self.px[sel] = col
            lum = self.px.astype(np.int32).sum(axis=2)

    def image(self) -> Image.Image:
        rgba = np.dstack([self.px, self.alpha])
        return Image.fromarray(rgba, "RGBA")


def chain_ring(p: Painter, cx: float, cy: float, rx: float, ry: float, light: str = "steel_2") -> None:
    """The Iron Ring itself: welded chain links laid on the floor as an ellipse.

    Links alternate flat (an open 7x4 oval) and edge-on (a 5x2 bar); the far
    half of the ring is drawn darker than the near half.
    """
    circumference = math.pi * (3 * (rx + ry) - math.sqrt((3 * rx + ry) * (rx + 3 * ry)))
    links = int(circumference / 7)
    for i in range(links):
        a = 2 * math.pi * i / links
        x = round(cx + rx * math.cos(a))
        y = round(cy + ry * math.sin(a))
        near = math.sin(a) > -0.15
        top, mid, low = (light, "steel_1", "ink") if near else ("steel_1", "steel_0", "ink")
        if i % 2 == 0:
            for dx in range(-2, 3):
                p.pixel(x + dx, y - 2, top)
                p.pixel(x + dx, y + 1, low)
            for dy in (-1, 0):
                p.pixel(x - 3, y + dy, mid)
                p.pixel(x + 3, y + dy, mid)
        else:
            for dx in range(-2, 3):
                p.pixel(x + dx, y - 1, top)
                p.pixel(x + dx, y, mid)
            for dx in range(-2, 3):
                p.pixel(x + dx, y + 1, low)
