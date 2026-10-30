"""The Foundry: the first layered venue (placeholder art).

Greta Voll's shut-down foundry floor (docs/design/setting.md), drawn as six
layers over an 800 px wide world so the camera scrolls 160 px and the
parallax shows. The layer sizes follow asset-spec §3.1: a layer with
parallax p must be 640 + ceil(160 * p) - x px wide.

    foundry_wall.png     parallax 0.25   680 x 360  opaque   back wall, tall windows, the works sign
    foundry_furnace.png  parallax 0.55   728 x 360  alpha    furnaces, a hanging ladle, gantry
    foundry_steam.png    parallax 0.55   640 x 360  alpha    drifting steam (repeats)
    foundry_crowd.png    parallax 0.80   768 x 84   alpha    rail and spectators, 8 frames (4 rest, 4 cheering)
    foundry_floor.png    parallax 1.00   800 x 98   alpha    steel-plate floor and the chain ring (top at y 262)
    foundry_front.png    parallax 1.25   850 x 360  alpha    foreground columns at x -10, drawn over the fighters
    foundry.png                          640 x 360  opaque   preview: the layers composed at the centre

Black (0, 0, 0) is not a palette colour; it marks transparency while painting.
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
from PIL import Image

from .font import glyph_mask
from .paint import Painter, c, chain_ring

WORLD = 800
SCREEN = 640
TRAVEL = WORLD - SCREEN
H = 360
CROWD_Y, CROWD_H, CROWD_FRAMES = 186, 84, 8
FLOOR_Y = 262


def layer_width(parallax: float) -> int:
    return SCREEN + int(np.ceil(TRAVEL * parallax))


def transparent(p: Painter) -> Image.Image:
    img = np.array(p.image())
    img[..., 3] = np.where(np.all(p.px == 0, axis=2), 0, 255)
    return Image.fromarray(img, "RGBA")


def blank(width: int, height: int, seed: int) -> Painter:
    p = Painter(width, height, seed)
    p.px[:] = 0
    return p


# --- layers ---------------------------------------------------------------------------

def wall() -> Image.Image:
    w = layer_width(0.25)
    p = Painter(w, H, seed=41)
    p.gradient(0, H, ["ink", "night_0", "night_1", "night_1", "night_0"])
    sign_y = 56
    # Brick courses: faint mortar lines, offset every row.
    for y in range(60, 250, 6):
        p.rect(0, y, w, y + 1, "night_0")
        for x in range((y // 6 % 2) * 9, w, 18):
            p.pixel(x, y + 2, "night_0")
    # Tall arched windows with cold light behind cracked panes.
    for x in range(40, w - 40, 120):
        p.rect(x, 78, x + 44, 200, "steel_0")
        p.ellipse(x + 22, 78, 22, 16, "steel_0")
        for gx in range(x + 3, x + 42, 10):
            p.rect(gx, 70, gx + 8, 197, "teal_0")
        for gy in range(84, 196, 14):
            p.rect(x + 3, gy, x + 41, gy + 1, "steel_0")
        p.glow(x + 22, 120, 40, ["teal_0", "teal_1"], 0.5, squash=1.6)
        for _ in range(4):  # broken panes
            bx, by = p.rng.randrange(x + 4, x + 38), p.rng.randrange(86, 190)
            p.rect(bx, by, bx + 4, by + 5, "night_0")
    # Girders between the windows.
    for x in range(10, w, 120):
        p.rect(x, 52, x + 10, 260, "night_0")
        p.rect(x + 2, 52, x + 3, 260, "steel_0")
    # The works sign, half its letters gone dark.
    text, scale = "VOLL WORKS", 3
    x0 = (w - len(text) * 6 * scale) // 2
    for i, ch in enumerate(text):
        dead = i in (2, 7)
        for gy, row in enumerate(glyph_mask(ch)):
            for gx, on in enumerate(row):
                if on:
                    bx, by = x0 + i * 6 * scale + gx * scale, sign_y + gy * scale
                    p.rect(bx, by, bx + scale, by + scale, "steel_1" if dead else "coral_2")
    return p.image()


def furnace() -> Image.Image:
    w = layer_width(0.55)
    p = blank(w, H, seed=42)
    for i, x in enumerate(range(60, w - 60, 200)):
        # Brick furnace body with a glowing mouth.
        p.polygon([(x - 46, 262), (x - 38, 120), (x + 38, 120), (x + 46, 262)], "brick_0")
        p.rect(x - 30, 96, x + 30, 122, "brick_0")
        p.rect(x - 12, 93, x + 12, 98, "night_0")  # the flue, rising into the gantry
        for y in range(128, 260, 8):
            p.rect(x - 40, y, x + 40, y + 1, "night_0")
        p.ellipse(x, 214, 22, 16, "coral_1")
        p.ellipse(x, 216, 16, 11, "coral_3")
        p.ellipse(x, 218, 9, 6, "sodium_4")
        p.rect(x - 24, 228, x + 24, 262, "brick_0")
        body = np.zeros((H, w), dtype=bool)
        body[110:262, max(0, x - 50):min(w, x + 50)] = True
        p.glow(x, 216, 60, ["brick_1", "coral_1", "coral_2"], 0.9, squash=0.8,
               mask=body & np.any(p.px != 0, axis=2))
    # Gantry beam and a ladle hanging from it.
    p.rect(0, 86, w, 93, "night_0")
    p.rect(0, 86, w, 87, "steel_1")
    lx = w // 2 + 20
    p.line(lx - 10, 93, lx - 4, 130, "steel_0", 2)
    p.line(lx + 10, 93, lx + 4, 130, "steel_0", 2)
    p.polygon([(lx - 26, 130), (lx + 26, 130), (lx + 18, 168), (lx - 18, 168)], "steel_0")
    p.rect(lx - 26, 128, lx + 26, 132, "steel_1")
    p.rect(lx - 22, 131, lx + 22, 133, "sodium_3")  # metal still glowing at the rim
    return transparent(p)


def steam() -> Image.Image:
    p = blank(SCREEN, H, seed=43)
    for _ in range(9):
        cx, cy = p.rng.randrange(SCREEN), p.rng.randrange(120, 230)
        rx, ry = p.rng.randrange(30, 70), p.rng.randrange(8, 16)
        for y in range(cy - ry, cy + ry):
            for x in range(cx - rx, cx + rx):
                d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
                # Sparse dithered puffs: a 1-bit alpha, density falling to the edge.
                if d < 1 and p.threshold[y % H, x % SCREEN] < 0.22 * (1 - d):
                    p.pixel(x % SCREEN, y, "fog")
    return transparent(p)


def spectator(p: Painter, x: int, base: int, height: int, cheer: bool, rng_arm: int, colour: str) -> None:
    shoulders = 10
    top = base - height
    p.polygon([(x - shoulders, base), (x - shoulders + 2, top + 12), (x + shoulders - 2, top + 12),
               (x + shoulders, base)], colour)
    p.ellipse(x, top + 6, 4, 5, colour)
    if cheer or rng_arm == 0:
        side = 1 if rng_arm % 2 else -1
        reach = -10 if cheer else 0
        p.line(x + side * 7, top + 13, x + side * 11, top + reach - 2, colour, 3)
        p.ellipse(x + side * 11, top + reach - 3, 2, 2, colour)
        if cheer and rng_arm % 3 == 0:  # both arms up
            p.line(x - side * 7, top + 13, x - side * 11, top - 12, colour, 3)
            p.ellipse(x - side * 11, top - 13, 2, 2, colour)
    for dx in range(-3, 4):  # warm rim light from the furnaces behind
        p.pixel(x + dx, top + 1 + (abs(dx) > 2), "sodium_1")


def crowd() -> Image.Image:
    w = layer_width(0.8)
    frames = []
    people = []
    rng = Painter(1, 1, seed=44).rng
    for x in range(8, w - 4, 15):
        people.append((x + rng.randrange(-3, 4), rng.randrange(32, 44), rng.randrange(6),
                       rng.choice(["night_0", "ink", "night_1"]), rng.randrange(4)))
    for f in range(CROWD_FRAMES):
        cheer = f >= CROWD_FRAMES // 2
        p = blank(w, CROWD_H, seed=45)
        base = 66
        for x, height, arm, colour, phase in people:
            # Each spectator bobs on their own beat: one pixel at rest, three when cheering.
            bob = ((f + phase) % 4 in (1, 2)) * (3 if cheer else 1)
            spectator(p, x, base, height + bob, cheer and (arm + f) % 2 == 0, arm, colour)
        # The rail in front of them.
        p.rect(0, 60, w, 63, "steel_1")
        p.rect(0, 60, w, 61, "steel_2")
        p.rect(0, 74, w, 76, "steel_0")
        for x in range(0, w, 48):
            p.rect(x, 60, x + 3, CROWD_H, "steel_0")
        frames.append(transparent(p))
    sheet = Image.new("RGBA", (w, CROWD_H * CROWD_FRAMES))
    for f, frame in enumerate(frames):
        sheet.paste(frame, (0, f * CROWD_H))
    return sheet


def floor() -> Image.Image:
    p = Painter(WORLD, H - FLOOR_Y, seed=46)
    p.gradient(0, H - FLOOR_Y, ["steel_0", "night_2", "night_1", "night_0"])
    # Steel deck plates: seams that widen towards the viewer, rivets, slag stains.
    y, k = 0, 0
    while y < H - FLOOR_Y:
        p.rect(0, y, WORLD, y + 1, "night_0")
        step = 4 + k * 2
        for x in range((k % 2) * 30, WORLD, 60 + k * 6):
            p.rect(x, y, x + 1, y + step, "night_0")
            p.pixel(x + 3, y + 2, "steel_1")
        y += step
        k += 1
    for _ in range(14):
        sx, sy = p.rng.randrange(WORLD), p.rng.randrange(20, H - FLOOR_Y)
        p.ellipse(sx, sy, p.rng.randrange(6, 16), p.rng.randrange(2, 4), "night_0")
    # Furnace light pooling on the deck behind the ring.
    p.glow(WORLD / 2, 10, 300, ["night_2", "brick_0"], 0.7, squash=0.12)
    chain_ring(p, WORLD / 2, 326 - FLOOR_Y, 282, 24, light="steel_2")
    img = np.array(p.image())
    img[0, :, :3] = c("steel_1")  # the floor's far edge catches the light
    return Image.fromarray(img, "RGBA")


FRONT_X = -10  # the columns start just off the left edge


def front() -> Image.Image:
    w = layer_width(1.25) - FRONT_X
    p = blank(w, H, seed=47)
    # Two riveted columns that frame the stage ends; the HUD bands stay clear in the middle.
    for x in (0, w - 26):
        p.rect(x, 0, x + 26, H, "ink")
        p.rect(x + 4, 0, x + 5, H, "night_1")
        for y in range(10, H, 22):
            p.pixel(x + 13, y, "steel_0")
    return transparent(p)


LAYERS = (("foundry_wall.png", wall), ("foundry_furnace.png", furnace), ("foundry_steam.png", steam),
          ("foundry_crowd.png", crowd), ("foundry_floor.png", floor), ("foundry_front.png", front))


def preview(images: dict[str, Image.Image]) -> Image.Image:
    """The layers composed with the camera in the middle (x = 80), crowd at rest."""
    camera = TRAVEL / 2
    out = Image.new("RGBA", (SCREEN, H), (0, 0, 0, 255))
    placement = {"foundry_wall.png": (0.25, 0), "foundry_furnace.png": (0.55, 0), "foundry_steam.png": (0.55, 0),
                 "foundry_crowd.png": (0.8, CROWD_Y), "foundry_floor.png": (1.0, FLOOR_Y),
                 "foundry_front.png": (1.25, 0)}
    for name, (parallax, y) in placement.items():
        img = images[name]
        if name == "foundry_crowd.png":
            img = img.crop((0, 0, img.width, CROWD_H))
        x = int(np.floor(-camera * parallax))
        canvas = Image.new("RGBA", (SCREEN, H), (0, 0, 0, 0))
        canvas.paste(img, (x + (FRONT_X if name == "foundry_front.png" else 0), y))
        out.alpha_composite(canvas)
    return out.convert("RGB")


def generate(assets: Path) -> None:
    folder = assets / "Stages" / "Foundry"
    folder.mkdir(parents=True, exist_ok=True)
    images = {name: build() for name, build in LAYERS}
    for name, img in images.items():
        img.save(folder / name, optimize=True)
    preview(images).save(folder / "foundry.png", optimize=True)
    print("foundry: 6 layers + preview")
