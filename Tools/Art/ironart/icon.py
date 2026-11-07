"""Application icon: a ring of chain around "IR", drawn on a 32x32 grid."""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image

from . import palette
from .font import glyph_mask


def icon32() -> Image.Image:
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    put = lambda x, y, name: img.putpixel((x, y), palette.rgb(name) + (255,))  # noqa: E731
    for y in range(32):              # rounded dark tile
        for x in range(32):
            corner = min(x, 31 - x) + min(y, 31 - y)
            if corner >= 3:
                put(x, y, "night_1" if y < 16 else "night_0")
    for i in range(20):              # chain ring
        a = 2 * math.pi * i / 20
        x, y = 15.5 + 12.5 * math.cos(a), 15.5 + 12.5 * math.sin(a)
        put(round(x), round(y), "sodium_3" if i % 2 else "sodium_1")
        if i % 2 == 0:
            put(round(15.5 + 11 * math.cos(a)), round(15.5 + 11 * math.sin(a)), "sodium_0")
    for j, ch in enumerate("IR"):    # letters
        ox, oy = 8 + j * 9, 9
        for gy, row in enumerate(glyph_mask(ch)):
            for gx, on in enumerate(row):
                if on:
                    put(ox + gx, oy + gy * 2, "chalk")
                    put(ox + gx, oy + gy * 2 + 1, "sodium_3")
    return img


def generate(assets: Path, windows_icon: Path, mac_icon: Path | None = None) -> None:
    base = icon32()
    large = base.resize((256, 256), Image.NEAREST)
    folder = assets / "Resources" / "UI"
    folder.mkdir(parents=True, exist_ok=True)
    large.save(folder / "icon.png", optimize=True)
    windows_icon.parent.mkdir(parents=True, exist_ok=True)
    large.save(windows_icon, sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (256, 256)])
    if mac_icon is not None:  # the macOS bundle's icon: every size up to 1024, scaled without smoothing
        sizes = [16, 32, 64, 128, 256, 512, 1024]
        images = [base.resize((n, n), Image.NEAREST) for n in sizes]
        mac_icon.parent.mkdir(parents=True, exist_ok=True)
        images[-1].save(mac_icon, format="ICNS", append_images=images[:-1])
    print("icon: done")
