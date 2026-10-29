"""Skeleton-driven pixel-art figure renderer.

A figure is a set of shaded primitives (tapered capsules and ellipses) placed
by a skeleton. Each pixel stores a material, a light value and the body group
it belongs to; the final image is quantised to 4-tone material ramps with
coloured outlines and inner contour lines between overlapping body groups,
which is what makes a small sprite read clearly.

Coordinates: figure space, pixels, origin at the ground between the feet,
x forward (the figure faces right), y up.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field
from typing import Callable

import numpy as np
from PIL import Image

from . import palette

Vec = tuple[float, float]

# Light from the upper front-left, cool rim light from behind.
LIGHT = np.array([-0.45, 0.62, 0.64])
LIGHT = LIGHT / np.linalg.norm(LIGHT)
RIM = np.array([0.85, 0.35])
RIM = RIM / np.linalg.norm(RIM)

# Body groups, back to front. Inner lines are drawn where a group overlaps a
# group further back.
FAR_ARM, FAR_LEG, TORSO, NEAR_LEG, HEAD, NEAR_ARM, PROP = range(7)


def rot(v: Vec, degrees: float) -> Vec:
    a = math.radians(degrees)
    c, s = math.cos(a), math.sin(a)
    return (v[0] * c - v[1] * s, v[0] * s + v[1] * c)


def add(a: Vec, b: Vec) -> Vec:
    return (a[0] + b[0], a[1] + b[1])


def sub(a: Vec, b: Vec) -> Vec:
    return (a[0] - b[0], a[1] - b[1])


def scale(a: Vec, k: float) -> Vec:
    return (a[0] * k, a[1] * k)


def length(a: Vec) -> float:
    return math.hypot(a[0], a[1])


def normalise(a: Vec) -> Vec:
    n = length(a) or 1.0
    return (a[0] / n, a[1] / n)


def two_bone_ik(root: Vec, target: Vec, l1: float, l2: float, bend: float) -> tuple[Vec, Vec]:
    """Returns (joint, end). bend = +1 bends to the left of root->target, -1 to the right."""
    d = sub(target, root)
    dist = length(d)
    dist = min(max(dist, abs(l1 - l2) + 0.01), l1 + l2 - 0.01)
    u = normalise(d) if length(d) > 1e-6 else (0.0, -1.0)
    end = add(root, scale(u, dist))
    a = (l1 * l1 - l2 * l2 + dist * dist) / (2 * dist)
    h = math.sqrt(max(l1 * l1 - a * a, 0.0))
    perp = (-u[1], u[0])
    joint = add(add(root, scale(u, a)), scale(perp, h * bend))
    return joint, end


MaterialFn = Callable[[np.ndarray, np.ndarray], np.ndarray]  # (t, s) -> material index per pixel


@dataclass
class Canvas:
    width: int
    height: int
    origin: tuple[int, int]  # pixel position of the figure origin
    materials: list[str] = field(default_factory=list)

    def __post_init__(self) -> None:
        self.mat = np.full((self.height, self.width), -1, dtype=np.int16)
        self.light = np.zeros((self.height, self.width), dtype=np.float32)
        self.group = np.full((self.height, self.width), -1, dtype=np.int8)
        self.flat = np.zeros((self.height, self.width), dtype=np.int8)  # >0: fixed tone, no shading
        ys, xs = np.mgrid[0:self.height, 0:self.width]
        self.fx = (xs + 0.5) - self.origin[0]
        self.fy = self.origin[1] - (ys + 0.5)

    def material_id(self, name: str) -> int:
        if name not in self.materials:
            self.materials.append(name)
        return self.materials.index(name)

    # --- primitives -------------------------------------------------------

    def _bbox(self, xmin: float, xmax: float, ymin: float, ymax: float) -> tuple[slice, slice] | None:
        ox, oy = self.origin
        c0 = max(int(math.floor(xmin + ox)) - 1, 0)
        c1 = min(int(math.ceil(xmax + ox)) + 1, self.width)
        r0 = max(int(math.floor(oy - ymax)) - 1, 0)
        r1 = min(int(math.ceil(oy - ymin)) + 1, self.height)
        if c0 >= c1 or r0 >= r1:
            return None
        return slice(r0, r1), slice(c0, c1)

    def _write(self, region, inside, light, materials, group, flat=0) -> None:
        rows, cols = region
        m = self.mat[rows, cols]
        m[inside] = materials[inside] if isinstance(materials, np.ndarray) else materials
        self.light[rows, cols][inside] = light[inside]
        self.group[rows, cols][inside] = group
        self.flat[rows, cols][inside] = flat

    @staticmethod
    def _shade(nx: np.ndarray, ny: np.ndarray, rim_strength: float = 1.0) -> np.ndarray:
        nz = np.sqrt(np.clip(1.0 - nx * nx - ny * ny, 0.0, 1.0))
        lam = np.clip(nx * LIGHT[0] + ny * LIGHT[1] + nz * LIGHT[2], 0.0, 1.0)
        value = 0.22 + 0.88 * lam
        rim = (nx * RIM[0] + ny * RIM[1]) > 0.78
        value = np.where(rim & (value < 0.7), 0.7 * rim_strength + value * (1 - rim_strength), value)
        return value

    def capsule(self, a: Vec, b: Vec, r0: float, r1: float, material: str | MaterialFn, group: int,
                darken: float = 0.0) -> None:
        region = self._bbox(min(a[0], b[0]) - max(r0, r1), max(a[0], b[0]) + max(r0, r1),
                            min(a[1], b[1]) - max(r0, r1), max(a[1], b[1]) + max(r0, r1))
        if region is None:
            return
        px, py = self.fx[region], self.fy[region]
        dx, dy = b[0] - a[0], b[1] - a[1]
        dd = dx * dx + dy * dy or 1e-6
        t = np.clip(((px - a[0]) * dx + (py - a[1]) * dy) / dd, 0.0, 1.0)
        cx, cy = a[0] + t * dx, a[1] + t * dy
        r = r0 + (r1 - r0) * t
        ox, oy = px - cx, py - cy
        dist = np.sqrt(ox * ox + oy * oy)
        inside = dist <= r
        nx, ny = ox / r, oy / r
        s = (dx * oy - dy * ox) / (math.sqrt(dd) * r)  # signed side, -1..1
        light = self._shade(nx, ny) - darken
        if callable(material):
            mats = material(t, s)
        else:
            mats = np.full(t.shape, self.material_id(material), dtype=np.int16)
        self._write(region, inside, light, mats, group)

    def ellipse(self, c: Vec, rx: float, ry: float, angle: float, material: str | MaterialFn, group: int,
                darken: float = 0.0) -> None:
        rmax = max(rx, ry)
        region = self._bbox(c[0] - rmax, c[0] + rmax, c[1] - rmax, c[1] + rmax)
        if region is None:
            return
        px, py = self.fx[region] - c[0], self.fy[region] - c[1]
        a = math.radians(-angle)
        lx = px * math.cos(a) - py * math.sin(a)
        ly = px * math.sin(a) + py * math.cos(a)
        ux, uy = lx / rx, ly / ry
        inside = ux * ux + uy * uy <= 1.0
        # Normals back in screen orientation.
        b = math.radians(angle)
        nx = ux * math.cos(b) - uy * math.sin(b)
        ny = ux * math.sin(b) + uy * math.cos(b)
        light = self._shade(nx, ny) - darken
        if callable(material):
            mats = material(ux, uy)
        else:
            mats = np.full(ux.shape, self.material_id(material), dtype=np.int16)
        self._write(region, inside, light, mats, group)

    def polygon(self, points: list[Vec], material: str, group: int, tone: int = 0, light: float = 0.6) -> None:
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        region = self._bbox(min(xs), max(xs), min(ys), max(ys))
        if region is None:
            return
        px, py = self.fx[region], self.fy[region]
        inside = np.zeros(px.shape, dtype=bool)
        n = len(points)
        for i in range(n):  # even-odd rule
            x0, y0 = points[i]
            x1, y1 = points[(i + 1) % n]
            cond = (y0 > py) != (y1 > py)
            xint = x0 + (py - y0) * (x1 - x0) / ((y1 - y0) or 1e-9)
            inside ^= cond & (px < xint)
        lights = np.full(px.shape, light, dtype=np.float32)
        self._write(region, inside, lights, self.material_id(material), group, flat=tone)

    def pixel(self, p: Vec, material: str, tone: int, group: int = PROP) -> None:
        col = int(math.floor(p[0] + self.origin[0]))
        row = int(math.floor(self.origin[1] - p[1]))
        if 0 <= row < self.height and 0 <= col < self.width:
            self.mat[row, col] = self.material_id(material)
            self.group[row, col] = group
            self.flat[row, col] = tone

    # --- output -------------------------------------------------------------

    def tones(self) -> np.ndarray:
        v = self.light
        t = np.where(v < 0.40, 1, np.where(v < 0.64, 2, np.where(v < 0.88, 3, 4))).astype(np.int8)
        t = np.where(self.flat > 0, self.flat, t)
        return np.where(self.mat >= 0, t, 0)

    def render(self, outline: bool = True, inner_lines: bool = True) -> Image.Image:
        tone = self.tones()
        mat = self.mat
        filled = mat >= 0

        tone = _clean_orphans(tone, mat)

        if outline or inner_lines:
            line = np.zeros_like(filled)
            g = self.group
            for dr, dc in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nm = np.roll(mat, (dr, dc), axis=(0, 1))
                ng = np.roll(g, (dr, dc), axis=(0, 1))
                if dr == 1:
                    nm[0, :] = -1
                if dr == -1:
                    nm[-1, :] = -1
                if dc == 1:
                    nm[:, 0] = -1
                if dc == -1:
                    nm[:, -1] = -1
                if outline:
                    line |= filled & (nm < 0)
                if inner_lines:
                    line |= filled & (nm >= 0) & (ng >= 0) & (ng < g) & (g != PROP)
            tone = np.where(line & (self.flat == 0), 0, tone)

        rgba = np.zeros((self.height, self.width, 4), dtype=np.uint8)
        for index, name in enumerate(self.materials):
            colours = palette.ramp(name)
            sel = mat == index
            for level in range(5):
                m = sel & (tone == level)
                if m.any():
                    rgba[m, :3] = colours[level]
                    rgba[m, 3] = 255
        return Image.fromarray(rgba, "RGBA")


def _clean_orphans(tone: np.ndarray, mat: np.ndarray) -> np.ndarray:
    """Replaces single pixels whose tone differs from 3+ same-material neighbours that agree."""
    out = tone.copy()
    h, w = tone.shape
    padded_t = np.pad(tone, 1, constant_values=-1)
    padded_m = np.pad(mat, 1, constant_values=-1)
    neigh_t = [padded_t[1 + dr:h + 1 + dr, 1 + dc:w + 1 + dc] for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1))]
    neigh_m = [padded_m[1 + dr:h + 1 + dr, 1 + dc:w + 1 + dc] for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1))]
    for candidate in range(1, 5):
        votes = sum(((nt == candidate) & (nm == mat)).astype(np.int8) for nt, nm in zip(neigh_t, neigh_m))
        same = sum(((nt == tone) & (nm == mat)).astype(np.int8) for nt, nm in zip(neigh_t, neigh_m))
        replace = (mat >= 0) & (tone != candidate) & (votes >= 3) & (same == 0)
        out = np.where(replace, candidate, out)
    return out
