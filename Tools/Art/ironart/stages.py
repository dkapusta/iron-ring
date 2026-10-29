"""The three Iron Ring venues as 640x360 backdrops (docs/design/setting.md).

Layout contract (docs/art/asset-spec.md): foot baseline y = 316, playable x
30..610, HUD bands at the top (0..52) and bottom (330..360) kept calm, floor
lower-contrast than the fighters, spectators only behind the barrier.
"""

from __future__ import annotations

import math
from pathlib import Path

from .font import glyph_mask
from .paint import Painter, chain_ring

W, H = 640, 360
FLOOR_Y = 316


# --- shared pieces --------------------------------------------------------------

def stars(p: Painter, count: int, y_max: int) -> None:
    for _ in range(count):
        x, y = p.rng.randrange(W), p.rng.randrange(y_max)
        p.pixel(x, y, p.rng.choice(["fog", "steel_2", "chalk"]))


def skyline(p: Painter, base: int, min_h: int, max_h: int, colour: str, window_colours: list[str],
            density: float, x0: int = 0, x1: int = W) -> None:
    x = x0
    while x < x1:
        w = p.rng.randrange(10, 34)
        h = p.rng.randrange(min_h, max_h)
        p.rect(x, base - h, x + w, base, colour)
        if p.rng.random() < 0.3:  # roof detail: water tank or antenna
            p.rect(x + w // 3, base - h - 4, x + w // 3 + 4, base - h, colour)
        for wy in range(base - h + 3, base - 2, 4):
            for wx in range(x + 2, x + w - 2, 3):
                if p.rng.random() < density:
                    p.pixel(wx, wy, p.rng.choice(window_colours))
        x += w + p.rng.randrange(0, 4)


def crowd(p: Painter, y_base: int, x0: int, x1: int, body: list[str], rim: str, density: float = 1.0,
          rim_from: tuple[float, float] | None = None) -> None:
    """Spectator silhouettes: shoulders and heads, some arms raised, rim-lit on top."""
    x = x0
    while x < x1:
        if p.rng.random() > density:
            x += p.rng.randrange(6, 14)
            continue
        colour = p.rng.choice(body)
        height = p.rng.randrange(30, 46)
        shoulders = p.rng.randrange(9, 13)
        top = y_base - height
        p.polygon([(x - shoulders, y_base), (x - shoulders + 2, top + 12), (x + shoulders - 2, top + 12),
                   (x + shoulders, y_base)], colour)
        p.ellipse(x, top + 6, 4, 5, colour)
        if p.rng.random() < 0.25:  # cap
            p.rect(x - 4, top + 1, x + 4, top + 3, colour)
            p.rect(x, top + 2, x + 6, top + 3, colour)
        if p.rng.random() < 0.3:   # raised arm
            side = p.rng.choice((-1, 1))
            p.line(x + side * (shoulders - 3), top + 13, x + side * (shoulders + 1), top - 6, colour, 3)
            p.ellipse(x + side * (shoulders + 1), top - 7, 2, 2, colour)
        lit = rim_from is None or abs(x - rim_from[0]) < rim_from[1]
        if lit:
            for dx in range(-3, 4):
                p.pixel(x + dx, top + 1 + (abs(dx) > 2), rim)
            p.pixel(x - shoulders + 2, top + 12, rim)
            p.pixel(x + shoulders - 3, top + 12, rim)
        x += p.rng.randrange(13, 22)


def lamp_post(p: Painter, x: int, top: int, base: int) -> None:
    p.rect(x - 1, top, x + 2, base, "ink")
    p.rect(x - 6, top - 2, x + 7, top + 1, "ink")
    p.rect(x - 5, top + 1, x + 6, top + 4, "sodium_4")
    p.glow(x, top + 6, 70, ["sodium_0", "sodium_1", "sodium_2"], 0.8, squash=1.0)
    p.rect(x - 5, top + 1, x + 6, top + 4, "sodium_4")


def plank_floor(p: Painter, top: int, colours: tuple[str, str, str]) -> None:
    """Side-on deck: horizontal planks that get taller towards the viewer."""
    y = top
    k = 0
    while y < H:
        thickness = 3 + k // 2
        p.rect(0, y, W, y + thickness, colours[1 + k % 2])
        p.rect(0, y + thickness - 1, W, y + thickness, colours[0])
        for _ in range(4 + k):
            jx = p.rng.randrange(W)
            p.rect(jx, y, jx + 1, y + thickness, colours[0])
        y += thickness
        k += 1


def tile_floor(p: Painter, top: int, colours: tuple[str, str, str]) -> None:
    y = top
    k = 0
    while y < H:
        thickness = 4 + k
        p.rect(0, y, W, y + thickness, colours[1])
        offset = (k % 2) * 8
        width = 16 + k * 2
        for x in range(-width + offset, W, width):
            if (x // width + k) % 3 == 0:
                p.rect(x + 1, y + 1, x + width, y + thickness, colours[2])
            p.rect(x, y, x + 1, y + thickness, colours[0])
        p.rect(0, y, W, y + 1, colours[0])
        y += thickness
        k += 1


def bulb_text(p: Painter, text: str, x: int, y: int, scale: int, lit: str, dead_every: int = 0) -> None:
    """Sign lettering made of bulbs on the 5x7 font grid."""
    cursor = x
    count = 0
    for ch in text:
        for gy, row in enumerate(glyph_mask(ch)):
            for gx, on in enumerate(row):
                if not on:
                    continue
                bx, by = cursor + gx * scale, y + gy * scale
                count += 1
                dead = dead_every and count % dead_every == 0
                p.rect(bx, by, bx + scale - 1, by + scale - 1, "steel_0" if dead else lit)
                if not dead:
                    p.pixel(bx, by, "chalk")
        cursor += 6 * scale


# --- venues ---------------------------------------------------------------------

def pier_nine() -> Painter:
    """Closed cargo pier after the night shift."""
    p = Painter(W, H, seed=9)
    p.gradient(0, 205, ["night_0", "night_0", "night_1", "night_2", "steel_0"])
    stars(p, 70, 120)
    p.glow(320, 205, 420, ["night_2", "sodium_0"], 0.9, squash=0.18)
    skyline(p, 205, 10, 46, "night_1", ["sodium_2", "sodium_3", "teal_2"], 0.10)
    # Distant lift bridge.
    for tx in (446, 548):
        p.rect(tx, 140, tx + 8, 205, "night_1")
        p.pixel(tx + 4, 138, "coral_3")
    p.rect(446, 168, 556, 172, "night_1")
    for i in range(12):
        p.line(446 + i * 9, 168, 446 + i * 9 + 9, 158, "night_1")
    p.rect(446, 157, 556, 159, "night_1")

    # Water with light reflections.
    p.gradient(205, 252, ["night_1", "teal_0", "night_1", "night_0"])
    for _ in range(140):
        y = p.rng.randrange(208, 250)
        x = p.rng.randrange(W)
        p.rect(x, y, x + p.rng.randrange(2, 9), y + 1, p.rng.choice(["sodium_1", "sodium_0", "teal_1", "steel_1"]))

    # Gantry crane, left: legs, bracing, boom, cab, warning light.
    for lx in (78, 132):
        p.rect(lx, 60, lx + 5, 252, "ink")
    for y in range(70, 250, 18):
        p.line(80, y, 134, y + 18, "ink")
        p.line(134, y, 80, y + 18, "ink")
    p.rect(20, 56, 330, 62, "ink")
    p.rect(20, 66, 300, 69, "ink")
    for x in range(20, 300, 12):
        p.line(x, 62, x + 6, 66, "ink")
    p.rect(150, 69, 172, 86, "ink")
    p.rect(154, 73, 168, 79, "sodium_2")
    p.line(240, 69, 240, 150, "ink")
    p.rect(232, 150, 250, 166, "ink")              # hanging load
    p.glow(105, 52, 16, ["coral_1", "coral_2", "coral_3"], 1.0)
    p.rect(104, 50, 107, 53, "coral_4")
    # Second crane, further away.
    p.rect(612, 92, 615, 205, "night_2")
    p.rect(588, 92, 640, 95, "night_2")
    p.rect(470, 100, 640, 102, "night_2")

    # Moored freighter with a floodlight on the pier.
    p.polygon([(372, 236), (392, 196), (640, 196), (640, 236)], "ink")
    p.rect(560, 150, 618, 196, "ink")
    for wx in range(566, 612, 7):
        p.rect(wx, 160, wx + 3, 163, "sodium_2")
    p.rect(578, 120, 581, 150, "ink")
    p.rect(570, 116, 590, 122, "ink")
    p.cone((574, 122), 26, 300, 145, ["sodium_0", "sodium_1"], 0.75)
    p.rect(572, 120, 584, 123, "sodium_4")

    # Spectators behind the pier rail, lit by the floodlight from the right.
    crowd(p, 262, 4, 236, ["night_1", "ink", "night_2"], "sodium_2", 0.9)
    crowd(p, 262, 410, 636, ["night_1", "ink", "night_2"], "sodium_3", 0.95, rim_from=(560, 140))
    # Bookmaker's chalkboard.
    p.rect(250, 214, 300, 246, "wood_1")
    p.rect(252, 216, 298, 244, "night_1")
    for row, y in enumerate(range(220, 242, 5)):
        for x in range(255, 296, 4):
            if p.rng.random() < 0.6:
                p.pixel(x, y, "chalk")
                p.pixel(x + 1, y, "bone")
    p.rect(256, 246, 258, 262, "wood_0")
    p.rect(292, 246, 294, 262, "wood_0")

    # Rail.
    p.rect(0, 238, W, 241, "steel_1")
    p.rect(0, 250, W, 252, "steel_0")
    for x in range(10, W, 46):
        p.rect(x, 236, x + 4, 268, "ink")
        p.rect(x, 236, x + 4, 237, "steel_2")

    # Deck, light pool, the ring.
    plank_floor(p, 266, ("wood_0", "wood_1", "wood_1"))
    p.glow(330, 318, 300, ["wood_2", "wood_3", "sodium_1"], 0.95, squash=0.22)
    p.glow(420, 300, 160, ["wood_2", "wood_3"], 0.6, squash=0.3)
    chain_ring(p, 320, 326, 282, 24, light="steel_2")
    lamp_post(p, 16, 124, 270)
    lamp_post(p, 624, 124, 270)
    return p


def lantern_row() -> Painter:
    """Night market after curfew."""
    p = Painter(W, H, seed=21)
    p.gradient(0, 230, ["ink", "night_0", "brick_0", "brick_0"])
    # Brick facade.
    for y in range(4, 230, 6):
        offset = 0 if (y // 6) % 2 else 8
        for x in range(-16 + offset, W, 16):
            if p.rng.random() < 0.55:
                p.rect(x + 1, y, x + 15, y + 5, "brick_1" if p.rng.random() < 0.12 else "brick_0")
    for wx in (40, 120, 480, 560):  # upper windows, a few lit
        p.rect(wx, 30, wx + 34, 74, "ink")
        lit = p.rng.random() < 0.6
        p.rect(wx + 3, 33, wx + 31, 71, "sodium_1" if lit else "night_1")
        p.rect(wx + 16, 33, wx + 18, 71, "ink")
        p.rect(wx + 3, 51, wx + 31, 53, "ink")
        if lit:
            p.glow(wx + 17, 52, 30, ["brick_1", "sodium_0"], 0.6)

    # Arches into the dark streets beyond.
    for ax in (140, 320, 500):
        p.rect(ax - 46, 128, ax + 46, 232, "ink")
        p.ellipse(ax, 128, 46, 30, "ink")
        skyline(p, 222, 8, 40, "night_0", ["coral_2", "teal_3", "sodium_3"], 0.12, ax - 44, ax + 44)
        p.rect(ax - 46, 222, ax + 46, 232, "night_0")
        for side in (-1, 1):
            p.rect(ax + side * 46 - 4, 100, ax + side * 46 + 4, 232, "brick_1")

    # Old station clock.
    p.glow(320, 66, 44, ["brick_1", "sodium_0", "sodium_1"], 0.9)
    p.ellipse(320, 66, 27, 27, "sodium_1")
    p.ellipse(320, 66, 23, 23, "bone", outline="wood_1")
    for i in range(12):
        a = i / 12 * 2 * math.pi
        p.pixel(round(320 + 19 * math.sin(a)), round(66 - 19 * math.cos(a)), "ink")
    p.line(320, 66, 320, 50, "ink", 2)        # minute hand: almost midnight
    p.line(320, 66, 318, 55, "ink", 2)
    p.rect(316, 92, 324, 104, "wood_0")

    # Shuttered stalls with striped awnings.
    for i, sx in enumerate(range(10, W, 106)):
        stripes = ("coral_2", "chalk") if i % 2 == 0 else ("teal_2", "bone")
        p.rect(sx, 178, sx + 92, 236, "steel_0")
        for y in range(182, 236, 4):
            p.rect(sx + 2, y, sx + 90, y + 1, "night_2")
        p.polygon([(sx - 4, 168), (sx + 96, 168), (sx + 92, 180), (sx, 180)], stripes[0])
        for x in range(sx, sx + 92, 12):
            p.polygon([(x, 168), (x + 6, 168), (x + 5, 180), (x - 1, 180)], stripes[1])

    # Strings of lanterns.
    for row, (y0, sag) in enumerate(((22, 40), (96, 26))):
        prev = None
        for x in range(0, W + 1, 4):
            y = y0 + sag * (1 - ((x - W / 2) / (W / 2)) ** 2)
            if prev:
                p.line(prev[0], prev[1], x, y, "ink")
            prev = (x, y)
            if x % 40 == 20 + row * 12:
                colour = "coral_3" if (x // 40) % 3 else "sodium_3"
                p.glow(x, y + 7, 22, ["brick_1", "sodium_0", "sodium_1"], 0.8)
                p.rect(x - 3, int(y) + 2, x + 4, int(y) + 12, colour)
                p.rect(x - 2, int(y) + 4, x + 3, int(y) + 10, "sodium_4" if colour == "sodium_3" else "coral_4")
                p.rect(x - 1, int(y), x + 2, int(y) + 2, "ink")

    # Crowd behind a rope line.
    crowd(p, 264, 0, W, ["night_1", "night_2", "brick_0"], "sodium_3", 0.92)
    for x in range(30, W, 120):
        p.rect(x, 238, x + 3, 266, "metal" if False else "steel_1")
        p.rect(x - 1, 236, x + 4, 239, "sodium_2")
    for x in range(0, W):
        y = 244 + 4 * math.sin((x % 120) / 120 * math.pi)
        p.pixel(x, round(y), "coral_2")
        p.pixel(x, round(y) + 1, "coral_1")

    tile_floor(p, 266, ("ink", "brick_0", "brick_1"))
    p.glow(320, 320, 300, ["brick_1", "coral_1", "sodium_0"], 0.85, squash=0.22)
    chain_ring(p, 320, 326, 282, 24, light="steel_2")
    return p


def the_crown() -> Painter:
    """Roof of the derelict Crown Hotel at first light: the final bout."""
    p = Painter(W, H, seed=33)
    p.gradient(0, 228, ["night_0", "night_1", "night_2", "steel_1", "coral_1", "coral_2", "sodium_2"])
    stars(p, 40, 70)
    for _ in range(9):  # thin lit cloud bands
        y = p.rng.randrange(110, 200)
        x = p.rng.randrange(-60, W)
        length = p.rng.randrange(60, 180)
        p.rect(x, y, x + length, y + 2, "coral_1" if y < 160 else "coral_2")
        p.rect(x + 8, y + 2, x + length - 12, y + 3, "coral_3" if y > 150 else "coral_2")
    p.glow(380, 236, 260, ["coral_2", "coral_3", "sodium_3"], 0.8, squash=0.25)
    skyline(p, 236, 20, 80, "night_1", ["sodium_3", "teal_3"], 0.05)
    skyline(p, 236, 6, 30, "night_2", ["sodium_2"], 0.04)
    for cx in (500, 560):  # harbour cranes far below
        p.rect(cx, 196, cx + 2, 236, "night_1")
        p.rect(cx - 16, 196, cx + 20, 198, "night_1")

    # Rooftop sign scaffold with bulb letters, a few dead.
    p.glow(320, 60, 170, ["night_2", "coral_1", "sodium_1"], 0.6, squash=0.35)
    p.rect(150, 70, 490, 73, "ink")
    for x in range(156, 490, 30):
        p.line(x, 73, x + 15, 104, "ink")
        p.line(x + 30, 73, x + 15, 104, "ink")
    p.rect(150, 104, 490, 107, "ink")
    for x in (166, 470):
        p.rect(x, 73, x + 4, 238, "ink")
    bulb_text(p, "CROWN", 320 - (5 * 6 * 5 - 5) // 2, 34, 5, "sodium_3", dead_every=7)
    bulb_text(p, "HOTEL", 320 - (5 * 6 * 3 - 3) // 2, 112, 3, "coral_3", dead_every=5)

    # Water tower on stilts.
    p.rect(30, 120, 96, 186, "wood_1")
    for y in range(124, 186, 8):
        p.rect(30, y, 96, y + 1, "wood_0")
    p.polygon([(26, 120), (63, 96), (100, 120)], "wood_0")
    for lx in (34, 58, 86):
        p.rect(lx, 186, lx + 4, 262, "ink")
    p.line(34, 200, 90, 250, "ink")
    p.line(90, 200, 34, 250, "ink")
    for x in range(30, 97):  # dawn rim light on the tank
        p.pixel(x, 121 + (x % 3 == 0), "coral_3")

    # Parapet and the few people who paid to watch the final.
    p.rect(0, 238, W, 268, "brick_1")
    for x in range(0, W, 14):
        p.rect(x, 240 + (x // 14 % 2) * 6, x + 13, 245 + (x // 14 % 2) * 6, "brick_0")
    p.rect(0, 234, W, 239, "bone")
    p.rect(0, 239, W, 240, "fog")
    crowd(p, 268, 160, 480, ["ink", "night_1", "night_2"], "coral_3", 0.55)

    # Tar-and-gravel roof.
    p.gradient(268, H, ["steel_0", "night_2", "steel_0", "night_1"])
    for _ in range(900):
        x, y = p.rng.randrange(W), p.rng.randrange(270, H)
        p.pixel(x, y, p.rng.choice(["steel_1", "night_1", "steel_2"]))
    p.glow(380, 300, 320, ["steel_1", "coral_1", "coral_2"], 0.7, squash=0.25)
    chain_ring(p, 320, 326, 282, 24, light="steel_2")
    return p


VENUES = {
    "PierNine/pier_nine": pier_nine,
    "LanternRow/lantern_row": lantern_row,
    "TheCrown/the_crown": the_crown,
}


def generate(assets: Path) -> None:
    for path, build in VENUES.items():
        target = assets / "Stages" / f"{path}.png"
        target.parent.mkdir(parents=True, exist_ok=True)
        build().image().convert("RGB").save(target, optimize=True)
        print(f"stages: {path}")
