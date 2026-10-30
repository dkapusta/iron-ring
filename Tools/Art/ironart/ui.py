"""Front-end art: the title logo and the menu backdrop (placeholders).

Both follow the same rules as the rest of the procedural art: master
palette only, ordered dithering, hard pixel edges. The final versions will
be drawn outside the generator (docs/art/generation-prompts.md, "Front end");
until then these define the sizes and the layout the screens are built for.

- title_logo.png   432x96, transparent: "IRON RING" in brass block letters
                    on a chain ring. Drawn at 1x, centred, top at y = 34.
- menu_backdrop.png 640x360, opaque: behind the select and options screens.
                    Calm in the centre band where the menus sit.
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

from .font import glyph_mask
from .paint import Painter, c, chain_ring

LOGO_W, LOGO_H = 432, 96
BLOCK = 7            # one glyph cell of the 5x7 font, in pixels
ADVANCE = 6 * BLOCK  # letter pitch
TEXT = "IRON RING"
# Brass, top to bottom of a letter.
BRASS = ["sodium_4", "sodium_3", "sodium_3", "sodium_2", "sodium_2", "sodium_1"]


def _letters_mask() -> np.ndarray:
    """Boolean mask of the block letters, BLOCK px per glyph cell.

    Diagonal steps in the 5x7 glyphs (the leg of the R, the shoulder of the
    N) are joined by a 45-degree band, so strokes stay connected at this size.
    """
    width = len(TEXT) * ADVANCE - BLOCK
    canvas = Image.new("1", (width, 7 * BLOCK), 0)
    draw = ImageDraw.Draw(canvas)
    b = BLOCK
    for i, ch in enumerate(TEXT):
        if ch == " ":
            continue
        g = glyph_mask(ch)
        on = lambda gx, gy: 0 <= gy < 7 and 0 <= gx < 5 and g[gy][gx]  # noqa: E731
        for gy in range(7):
            for gx in range(5):
                x, y = i * ADVANCE + gx * b, gy * b
                if on(gx, gy):
                    draw.rectangle([x, y, x + b - 1, y + b - 1], fill=1)
                for dx in (-1, 1):  # a diagonal neighbour below with both corner cells empty
                    if on(gx, gy) and on(gx + dx, gy + 1) and not on(gx + dx, gy) and not on(gx, gy + 1):
                        cx, cy = x + (b if dx > 0 else 0), y + b  # the shared corner
                        draw.polygon([(cx - b, cy), (cx, cy - b), (cx + b - 1, cy), (cx, cy + b - 1)], fill=1)
    return np.array(canvas, dtype=bool)


def _grow(mask: np.ndarray, n: int) -> np.ndarray:
    out = mask.copy()
    for _ in range(n):
        grown = out.copy()
        grown[1:] |= out[:-1]
        grown[:-1] |= out[1:]
        grown[:, 1:] |= out[:, :-1]
        grown[:, :-1] |= out[:, 1:]
        out = grown
    return out


def logo() -> Image.Image:
    p = Painter(LOGO_W, LOGO_H, seed=1992)
    p.px[:] = 0  # (0, 0, 0) is not a palette colour: it marks transparency below

    # The ring behind the letters: welded chain, wider than the word.
    chain_ring(p, LOGO_W / 2, LOGO_H / 2 + 6, LOGO_W / 2 - 6, LOGO_H / 2 - 12, light="steel_2")

    letters = _letters_mask()
    lh, lw = letters.shape
    ox, oy = (LOGO_W - lw) // 2, (LOGO_H - lh) // 2 - 2
    full = np.zeros((LOGO_H, LOGO_W), dtype=bool)
    full[oy:oy + lh, ox:ox + lw] = letters

    # Drop shadow down-right, then a 2 px ink outline.
    shadow = np.zeros_like(full)
    shadow[3:, 3:] = _grow(full, 2)[:-3, :-3]
    p.px[shadow] = c("night_0")
    p.px[_grow(full, 2)] = c("ink")

    # Brass face: a vertical ramp across the letter height, dithered between steps.
    for y in range(oy, oy + lh):
        t = (y - oy) / max(lh - 1, 1) * (len(BRASS) - 1)
        k = min(int(t), len(BRASS) - 2)
        row = full[y]
        upper = p.threshold[y] < (t - k)
        p.px[y, row & upper] = c(BRASS[k + 1])
        p.px[y, row & ~upper] = c(BRASS[k])
    # Bevel: a light top edge and a dark bottom edge on every block run.
    above = np.zeros_like(full)
    above[1:] = full[:-1]
    below = np.zeros_like(full)
    below[:-1] = full[1:]
    p.px[full & ~above] = c("sodium_4")
    p.px[full & ~below] = c("sodium_0")
    left = np.zeros_like(full)
    left[:, 1:] = full[:, :-1]
    p.px[full & ~left & above & below] = c("sodium_3")
    # Rivets: one per letter, upper left.
    for i, ch in enumerate(TEXT):
        if ch != " ":
            x, y = ox + i * ADVANCE + 2, oy + 2
            if full[y, x]:
                p.pixel(x, y, "chalk")

    img = p.image()
    alpha = np.where(np.all(p.px == 0, axis=2), 0, 255).astype(np.uint8)
    arr = np.array(img)
    arr[..., 3] = alpha
    return Image.fromarray(arr, "RGBA")


def backdrop() -> Image.Image:
    w, h = 640, 360
    p = Painter(w, h, seed=316)
    p.gradient(0, h, ["ink", "night_0", "night_1", "night_1", "night_0"])
    # Two lamps over the ring, out of frame at the top.
    p.cone((150, -20), 18, 330, 75, ["night_2", "steel_0"], strength=0.7)
    p.cone((490, -20), 18, 330, 105, ["night_2", "steel_0"], strength=0.7)
    p.glow(320, 300, 300, ["night_2"], strength=0.6, squash=0.25)
    # The ring on the floor, far below the menus.
    chain_ring(p, 320, 300, 290, 30, light="steel_1")
    # Crowd: two ragged rows of heads and shoulders along the bottom edge.
    for row, (base, colour) in enumerate(((334, "night_0"), (342, "ink"))):
        for x in range(-10 + row * 5, w + 10, 10):
            top = base + p.rng.randrange(-4, 5)
            cx = x + p.rng.randrange(-2, 3)
            p.ellipse(cx, top, 4, 5, colour)
            p.rect(cx - 7, top + 4, cx + 8, h, colour)
    # Brass rules framing the header and footer bands.
    for y in (60, 336):
        for x in range(24, w - 24):
            if x % 4 != 3:
                p.pixel(x, y, "sodium_1")
    return p.image()


def generate(assets: Path) -> None:
    folder = assets / "Resources" / "UI"
    folder.mkdir(parents=True, exist_ok=True)
    logo().save(folder / "title_logo.png", optimize=True)
    backdrop().save(folder / "menu_backdrop.png", optimize=True)
    print("ui: title_logo.png, menu_backdrop.png")

