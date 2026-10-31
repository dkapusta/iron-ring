"""The M3 venues: Dry Dock 4, the Hold and the Broker's Club (placeholder art).

Each is a layered stage over an 800 px world (asset-spec §3.1), built like The
Foundry's placeholder (foundry.py): a back wall or sky, a structure layer, a
crowd behind a barrier (8 frames: 4 at rest, 4 cheering), the floor with the
chain ring, and foreground pieces at the stage ends. The preview is the
layers composed by ironart/preview.py, exactly as the game draws them.

    <id>_back.png    parallax 0.25   680 x 360  opaque
    <id>_mid.png     parallax 0.55   728 x 360  alpha   (Hold: 2 frames, the lamp swinging)
    <id>_crowd.png   parallax 0.80   768 x 672  alpha   8 frames of 84 px, at y 186
    <id>_floor.png   parallax 1.00   800 x 98   alpha   at y 262
    <id>_front.png   parallax 1.25   850 x 360  alpha   at x -10, over the fighters
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

import numpy as np
from PIL import Image

from . import preview
from .foundry import CROWD_FRAMES, CROWD_H, CROWD_Y, FLOOR_Y, FRONT_X, H, WORLD, blank, layer_width, transparent
from .paint import Painter, c, chain_ring

MID_FRAMES = {"ship_hold": 2}
MID_TICKS = 30


# --- shared pieces ---------------------------------------------------------------------

@dataclass(frozen=True)
class CrowdStyle:
    colours: tuple[str, ...]  # coat colours
    rim: str                  # the venue's key light on heads and shoulders
    hats: float               # share of spectators wearing a hat
    seed: int


def spectator(p: Painter, x: int, base: int, height: int, raise_arms: int, colour: str, rim: str,
              hat: bool) -> None:
    """A spectator from behind: shoulders, head, an optional hat, 0-2 raised arms with fists."""
    top = base - height
    p.polygon([(x - 10, base), (x - 8, top + 12), (x + 8, top + 12), (x + 10, base)], colour)
    p.ellipse(x, top + 6, 4, 5, colour)
    if hat:
        p.rect(x - 6, top + 1, x + 7, top + 3, colour)
        p.rect(x - 4, top - 2, x + 5, top + 2, colour)
    for side in ((1,), (1, -1))[raise_arms - 1] if raise_arms else ():
        p.line(x + side * 7, top + 13, x + side * 11, top - 8, colour, 3)
        p.ellipse(x + side * 11, top - 10, 2, 2, rim)  # the fist catches the light
    for dx in range(-3, 4):
        p.pixel(x + dx, top + (-2 if hat else 1) + (abs(dx) > 2), rim)


def crowd(style: CrowdStyle, barrier: Callable[[Painter, int], None]) -> Image.Image:
    w = layer_width(0.8)
    rng = Painter(1, 1, seed=style.seed).rng
    people = [(x + rng.randrange(-3, 4), rng.randrange(32, 44), rng.randrange(6), rng.choice(style.colours),
               rng.randrange(4), rng.random() < style.hats) for x in range(8, w - 4, 15)]
    sheet = Image.new("RGBA", (w, CROWD_H * CROWD_FRAMES))
    for f in range(CROWD_FRAMES):
        cheer = f >= CROWD_FRAMES // 2
        p = blank(w, CROWD_H, seed=style.seed + 1)
        for x, height, arm, colour, phase, hat in people:
            bob = ((f + phase) % 4 in (1, 2)) * (3 if cheer else 1)
            arms = (1 + arm % 2) if cheer and (arm + f) % 2 == 0 else 0
            spectator(p, x, 66, height + bob, arms, colour, style.rim, hat)
        barrier(p, w)
        sheet.paste(transparent(p), (0, f * CROWD_H))
    return sheet


def floor_base(stops: list[str], seed: int) -> Painter:
    p = Painter(WORLD, H - FLOOR_Y, seed=seed)
    p.gradient(0, H - FLOOR_Y, stops)
    return p


def finish_floor(p: Painter, edge: str, glow: list[str], ring_light: str) -> Image.Image:
    p.glow(WORLD / 2, 10, 300, glow, 0.6, squash=0.12)
    chain_ring(p, WORLD / 2, 326 - FLOOR_Y, 282, 24, light=ring_light)
    img = np.array(p.image())
    img[0, :, :3] = c(edge)  # the far edge catches the light
    return Image.fromarray(img, "RGBA")


def stars(p: Painter, y1: int, count: int) -> None:
    for _ in range(count):
        p.pixel(p.rng.randrange(p.w), p.rng.randrange(4, y1), p.rng.choice(["fog", "steel_2", "chalk"]))


# --- Dry Dock 4 --------------------------------------------------------------------------

def dry_dock_back() -> Image.Image:
    w = layer_width(0.25)
    p = Painter(w, H, seed=61)
    p.gradient(0, 200, ["ink", "night_0", "night_1", "night_2"])
    stars(p, 120, 40)
    p.rect(0, 200, w, H, "night_0")
    # Distant cranes and the city's lights along the horizon.
    for x in range(30, w, 110):
        p.rect(x, 96, x + 4, 200, "ink")
        p.line(x + 2, 100, x + 70, 108, "ink", 3)
        p.line(x + 2, 100, x - 24, 106, "ink", 2)
        p.pixel(x + 70, 108, "coral_3")  # warning lamp
    for x in range(0, w, 7):
        top = 176 + p.rng.randrange(0, 18)
        p.rect(x, top, x + 6, 200, "night_1")
        if p.rng.random() < 0.4:
            p.pixel(x + 2, top + 4, "sodium_2")
    # The dock's far wall: stepped concrete altars down to black water.
    for k, y in enumerate(range(200, 262, 12)):
        p.rect(0, y, w, y + 12, ["steel_0", "night_2", "steel_0", "night_2", "night_1", "night_1"][k])
        p.rect(0, y, w, y + 1, "steel_1")
    return p.image()


def dry_dock_mid() -> Image.Image:
    w = layer_width(0.55)
    p = blank(w, H, seed=62)
    # The bow of a laid-up freighter on its keel blocks, rust running down the plates.
    cx = w // 2 + 30
    hull = [(cx - 230, 250), (cx - 200, 110), (cx + 120, 96), (cx + 190, 120), (cx + 160, 250)]
    p.polygon(hull, "night_2")
    for y in range(120, 250, 14):
        p.line(cx - 210, y, cx + 175, y - 8, "night_1")
    for _ in range(26):
        x = p.rng.randrange(cx - 190, cx + 150)
        p.line(x, 120, x + 2, 120 + p.rng.randrange(20, 90), "sodium_0")
    p.rect(cx - 30, 104, cx + 60, 112, "coral_1")  # the boot-topping stripe, faded
    for x in range(cx - 170, cx + 140, 70):
        p.rect(x, 250, x + 26, 262, "wood_1")
        p.rect(x, 250, x + 26, 252, "wood_2")
    # A floodlight tower on the left, its beam over the dock.
    p.rect(40, 60, 46, 262, "ink")
    p.rect(30, 56, 58, 64, "steel_0")
    p.cone((44, 64), 16, 260, 70, ["night_2", "steel_0", "steel_1"], 0.6)
    p.pixel(44, 60, "sodium_4")
    return transparent(p)


def chain_barrier(p: Painter, w: int) -> None:
    """Stanchions with a chain sagging between them."""
    for x in range(0, w, 64):
        p.rect(x, 54, x + 4, CROWD_H, "steel_0")
        p.rect(x, 54, x + 4, 56, "steel_2")
        for i in range(0, 64, 4):
            sag = 6 * math.sin(math.pi * i / 64)
            p.rect(x + i, int(58 + sag), x + i + 3, int(60 + sag), "steel_1" if i % 8 else "steel_2")
    p.rect(0, 66, w, CROWD_H, "night_1")  # the dock edge they stand on
    p.rect(0, 66, w, 67, "steel_1")


def dry_dock_crowd() -> Image.Image:
    return crowd(CrowdStyle(("night_0", "ink", "night_1"), "steel_2", 0.35, 63), chain_barrier)


def dry_dock_floor() -> Image.Image:
    p = floor_base(["steel_0", "night_2", "night_1", "night_0"], 64)
    # Wet concrete slabs; puddles hold the floodlight.
    for y in range(0, H - FLOOR_Y, 16):
        p.rect(0, y, WORLD, y + 1, "night_0")
    for x in range(0, WORLD, 90):
        p.line(x, 0, x - 20, H - FLOOR_Y, "night_0")
    for _ in range(12):
        px, py = p.rng.randrange(WORLD), p.rng.randrange(14, H - FLOOR_Y - 6)
        rx = p.rng.randrange(14, 40)
        p.ellipse(px, py, rx, 3, "night_2")
        p.rect(px - rx // 2, py, px + rx // 2, py + 1, "steel_1")
    return finish_floor(p, "steel_1", ["night_2", "steel_0"], "steel_2")


def dry_dock_front() -> Image.Image:
    w = layer_width(1.25) - FRONT_X
    p = blank(w, H, seed=65)
    for x in (4, w - 30):  # mooring bollards with a hawser chain hanging down
        p.rect(x, 300, x + 26, H, "ink")
        p.ellipse(x + 13, 300, 15, 7, "ink")
        p.rect(x + 3, 302, x + 4, H, "night_1")
        for y in range(0, 300, 6):
            p.rect(x + 11, y, x + 16, y + 4, "ink")
            p.pixel(x + 12, y + 1, "steel_0")
    return transparent(p)


# --- The Hold ----------------------------------------------------------------------------

def ship_hold_back() -> Image.Image:
    w = layer_width(0.25)
    p = Painter(w, H, seed=71)
    p.gradient(0, H, ["ink", "night_0", "night_1", "night_0"])
    # The open hatch far above: night sky and a shaft of moonlight.
    hx = w // 2
    p.rect(hx - 70, 0, hx + 70, 40, "night_1")
    stars(p, 36, 0)
    for _ in range(10):
        p.pixel(p.rng.randrange(hx - 66, hx + 66), p.rng.randrange(4, 36), "fog")
    p.rect(hx - 74, 40, hx + 74, 46, "steel_0")
    p.cone((hx, 20), 14, 300, 90, ["night_1", "night_2", "steel_0"], 0.5)
    # Riveted bulkhead: frames (ribs) every 60 px, plate seams, rivet rows.
    for x in range(0, w, 60):
        p.rect(x, 46, x + 8, H, "night_0")
        p.rect(x + 1, 46, x + 2, H, "steel_0")
        for y in range(56, 250, 12):
            p.pixel(x + 5, y, "steel_1")
    for y in range(90, 250, 44):
        p.rect(0, y, w, y + 1, "night_0")
    p.rect(0, 44, w, 48, "steel_1")
    return p.image()


def ship_hold_mid() -> Image.Image:
    w = layer_width(0.55)
    sheet = Image.new("RGBA", (w, H * MID_FRAMES["ship_hold"]))
    for f in range(MID_FRAMES["ship_hold"]):
        p = blank(w, H, seed=72)
        # Crates and a container stacked at both ends.
        for x0, stack in ((0, (3, 2, 1)), (w - 190, (1, 3, 2))):
            for i, n in enumerate(stack):
                for j in range(n):
                    x, y = x0 + i * 62, 262 - (j + 1) * 46
                    p.rect(x, y, x + 58, y + 44, "wood_1")
                    p.rect(x, y, x + 58, y + 2, "wood_2")
                    p.line(x, y, x + 58, y + 44, "wood_0")
                    p.line(x + 58, y, x, y + 44, "wood_0")
        p.rect(w // 2 - 160, 196, w // 2 - 40, 262, "coral_0")  # a container, its doors shut
        for x in range(w // 2 - 156, w // 2 - 40, 8):
            p.rect(x, 198, x + 2, 262, "coral_1")
        # A cargo net hanging from the hatch coaming, and a work lamp that swings.
        for x in range(w // 2 + 40, w // 2 + 150, 10):
            p.line(x, 46, x + 6, 110, "wood_2")
        for y in range(54, 110, 10):
            p.line(w // 2 + 40, y, w // 2 + 156, y + 2, "wood_2")
        swing = (-8, 8)[f]
        lx, ly = w // 2 + swing, 150
        p.line(w // 2, 46, lx, ly - 6, "steel_0")
        p.polygon([(lx - 8, ly), (lx + 8, ly), (lx + 5, ly - 7), (lx - 5, ly - 7)], "steel_1")
        p.ellipse(lx, ly + 2, 3, 2, "sodium_4")
        p.glow(lx, ly + 10, 70, ["sodium_0", "sodium_1"], 0.7, squash=0.9,
               mask=np.any(p.px != 0, axis=2))
        sheet.paste(transparent(p), (0, f * H))
    return sheet


def net_barrier(p: Painter, w: int) -> None:
    """Cargo netting strung between deck stanchions."""
    for x in range(0, w, 56):
        p.rect(x, 50, x + 3, CROWD_H, "steel_0")
    for y in range(54, 72, 6):
        p.rect(0, y, w, y + 1, "wood_2")
    for x in range(0, w, 6):
        p.line(x, 54, x + 3, 72, "wood_1")
    p.rect(0, 72, w, CROWD_H, "night_0")  # the crates they stand on
    p.rect(0, 72, w, 73, "wood_1")


def ship_hold_crowd() -> Image.Image:
    return crowd(CrowdStyle(("night_0", "ink", "wood_0"), "sodium_1", 0.25, 73), net_barrier)


def ship_hold_floor() -> Image.Image:
    p = floor_base(["night_2", "night_1", "night_0", "ink"], 74)
    y, k = 0, 0
    while y < H - FLOOR_Y:
        p.rect(0, y, WORLD, y + 1, "night_0")
        step = 5 + k * 2
        for x in range((k % 2) * 40, WORLD, 80):
            p.rect(x, y, x + 1, y + step, "night_0")
            p.pixel(x + 4, y + 2, "steel_1")
        y += step
        k += 1
    for x in (200, 560):  # drain grates
        p.rect(x, 30, x + 40, 40, "ink")
        for gx in range(x + 2, x + 40, 4):
            p.rect(gx, 31, gx + 1, 39, "steel_0")
    for _ in range(10):  # oil
        p.ellipse(p.rng.randrange(WORLD), p.rng.randrange(20, H - FLOOR_Y), p.rng.randrange(8, 20), 2, "ink")
    return finish_floor(p, "steel_0", ["night_2", "sodium_0"], "steel_2")


def ship_hold_front() -> Image.Image:
    w = layer_width(1.25) - FRONT_X
    p = blank(w, H, seed=75)
    for x in (2, w - 28):  # a steel ladder up the hull at each end
        p.rect(x, 0, x + 4, H, "ink")
        p.rect(x + 22, 0, x + 26, H, "ink")
        for y in range(8, H, 14):
            p.rect(x, y, x + 26, y + 3, "ink")
            p.pixel(x + 12, y, "night_2")
    return transparent(p)


# --- The Broker's Club ---------------------------------------------------------------------

def brokers_club_back() -> Image.Image:
    w = layer_width(0.25)
    p = Painter(w, H, seed=81)
    p.gradient(0, H, ["ink", "teal_0", "night_1", "ink"])
    # Damask wallpaper over dark wood panelling, gilt-framed paintings, sconces.
    for y in range(56, 200, 12):
        for x in range((y // 12 % 2) * 12, w, 24):
            p.ellipse(x, y, 3, 4, "teal_0")
    p.rect(0, 200, w, 262, "wood_0")
    for x in range(0, w, 56):
        p.rect(x, 206, x + 50, 256, "wood_1")
        p.rect(x + 2, 208, x + 48, 254, "wood_0")
    p.rect(0, 198, w, 202, "sodium_1")  # the dado rail
    for i, x in enumerate(range(60, w - 60, 150)):
        fw, fh = (60, 76) if i % 2 else (84, 60)
        p.rect(x, 90, x + fw, 90 + fh, "sodium_1")
        p.rect(x + 3, 93, x + fw - 3, 87 + fh, ["coral_0", "night_2", "wood_1"][i % 3])
        p.rect(x + 3, 93, x + fw - 3, 94, "sodium_2")
    for x in range(130, w, 150):
        p.rect(x, 140, x + 3, 154, "sodium_1")
        p.ellipse(x + 1, 138, 3, 4, "sodium_4")
        p.glow(x + 1, 140, 34, ["teal_1", "sodium_0", "sodium_1"], 0.6, squash=1.2)
    return p.image()


def brokers_club_mid() -> Image.Image:
    w = layer_width(0.55)
    p = blank(w, H, seed=82)
    # A mezzanine with a brass rail, red velvet drapes, and two chandeliers.
    p.rect(0, 40, w, 50, "wood_0")
    p.rect(0, 30, w, 32, "sodium_1")  # in the HUD band: kept dim
    for x in range(0, w, 14):
        p.rect(x, 32, x + 2, 40, "sodium_0")
    for x in (0, w - 70):
        p.polygon([(x, 50), (x + 70, 50), (x + 52, 262), (x + 14, 262)], "coral_1")
        for fx in range(x + 8, x + 66, 10):
            p.line(fx, 52, fx + (fx - x - 35) // 5, 262, "coral_0", 2)
    for cx in (w // 3, 2 * w // 3):
        p.line(cx, 50, cx, 76, "sodium_1")
        p.ellipse(cx, 84, 26, 8, "sodium_1")
        for dx in range(-24, 25, 8):
            p.line(cx + dx, 84, cx + dx, 96, "sodium_2")
            p.pixel(cx + dx, 80, "sodium_4")
        p.glow(cx, 84, 46, ["sodium_0", "sodium_1"], 0.5, squash=0.6, mask=np.any(p.px != 0, axis=2))
    return transparent(p)


def rope_barrier(p: Painter, w: int) -> None:
    """Brass stanchions with a red velvet rope."""
    for x in range(0, w, 72):
        p.rect(x, 52, x + 4, CROWD_H, "sodium_1")
        p.ellipse(x + 2, 52, 3, 3, "sodium_3")
        for i in range(0, 72, 3):
            sag = 7 * math.sin(math.pi * i / 72)
            p.rect(x + i, int(56 + sag), x + i + 3, int(59 + sag), "coral_1" if i % 6 else "coral_2")
    p.rect(0, 70, w, CROWD_H, "wood_0")


def brokers_club_crowd() -> Image.Image:
    return crowd(CrowdStyle(("ink", "night_0", "coral_0", "wood_0"), "sodium_2", 0.5, 83), rope_barrier)


def brokers_club_floor() -> Image.Image:
    p = floor_base(["wood_1", "wood_0", "wood_0", "ink"], 84)
    # Herringbone parquet, polished: blocks alternate in two shades.
    for y in range(0, H - FLOOR_Y, 6):
        for x in range(0, WORLD, 12):
            if (x // 12 + y // 6) % 2:
                p.rect(x, y, x + 12, y + 6, "wood_1" if y < 40 else "wood_0")
            p.pixel(x, y, "ink")
    return finish_floor(p, "sodium_1", ["wood_1", "sodium_0"], "sodium_3")


def brokers_club_front() -> Image.Image:
    w = layer_width(1.25) - FRONT_X
    p = blank(w, H, seed=85)
    for x0, x1 in ((0, 30), (w - 30, w)):  # velvet curtain edges, gathered with a gold tie
        p.rect(x0, 0, x1, H, "coral_0")
        for fx in range(x0 + 3, x1, 6):
            p.rect(fx, 0, fx + 2, H, "coral_1")
        p.rect(x0, 210, x1, 216, "sodium_2")
    return transparent(p)


# --- registry ------------------------------------------------------------------------------

@dataclass(frozen=True)
class Venue:
    id: str
    folder: str
    back: Callable[[], Image.Image]
    mid: Callable[[], Image.Image]
    crowd: Callable[[], Image.Image]
    floor: Callable[[], Image.Image]
    front: Callable[[], Image.Image]


VENUES = (
    Venue("dry_dock", "DryDock", dry_dock_back, dry_dock_mid, dry_dock_crowd, dry_dock_floor, dry_dock_front),
    Venue("ship_hold", "ShipHold", ship_hold_back, ship_hold_mid, ship_hold_crowd, ship_hold_floor, ship_hold_front),
    Venue("brokers_club", "BrokersClub", brokers_club_back, brokers_club_mid, brokers_club_crowd,
          brokers_club_floor, brokers_club_front),
)


def layers(v: Venue) -> list[dict]:
    """The venue's stages.json layer list."""
    folder = f"Stages/{v.folder}/{v.id}"
    mid = {"image": f"{folder}_mid.png", "parallax": 0.55}
    if v.id in MID_FRAMES:
        mid.update(frames=MID_FRAMES[v.id], frameTicks=MID_TICKS)
    return [
        {"image": f"{folder}_back.png", "parallax": 0.25},
        mid,
        {"image": f"{folder}_crowd.png", "parallax": 0.8, "y": CROWD_Y, "frames": CROWD_FRAMES, "frameTicks": 14,
         "crowd": True},
        {"image": f"{folder}_floor.png", "parallax": 1.0, "y": FLOOR_Y},
        {"image": f"{folder}_front.png", "parallax": 1.25, "x": FRONT_X, "front": True},
    ]


def generate(assets: Path) -> None:
    for v in VENUES:
        folder = assets / "Stages" / v.folder
        folder.mkdir(parents=True, exist_ok=True)
        for part, build in (("back", v.back), ("mid", v.mid), ("crowd", v.crowd), ("floor", v.floor),
                            ("front", v.front)):
            build().save(folder / f"{v.id}_{part}.png", optimize=True)
        stage = {"width": WORLD, "layers": layers(v)}
        preview.compose(assets, stage).save(folder / f"{v.id}.png", optimize=True)
        print(f"venues: {v.id}: 5 layers + preview")
