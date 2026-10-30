"""Turns externally generated art into game-ready assets (docs/art/asset-spec.md).

Fighters: one image per frame (any size), all drawn on the same canvas with
the character's origin -- the point on the ground between the feet -- at the
same registration point. The importer keys out a flat background, scales
every frame by one factor (so the character never changes size), hardens
edges, optionally snaps colours to the palette, and packs the atlas with
pivots and partner offsets.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

from . import atlas, palette


@dataclass
class FighterImport:
    fighter: str                 # id, e.g. "dee"
    source: Path                 # folder with <frame>.png files
    origin: tuple[float, float]  # registration point in source pixels (feet on the ground)
    scale: float | None = None   # source px -> game px; default: derived from --height
    height: int | None = None    # standing height in game px of the reference frame
    reference: str = "idle_0"
    quantize: bool = True
    tolerance: int = 40          # background key tolerance (RGB distance)
    manifest: Path | None = None # optional per-frame overrides: {"frame": {"origin": [x,y], "partner": [dx,dy]}}


def key_background(img: Image.Image, tolerance: int) -> Image.Image:
    """Keys a flat background colour into transparency.

    Everything connected to the border within `tolerance` of the key colour is
    removed (anti-aliased fringes, noisy generator backgrounds); enclosed pockets
    -- the gap between an arm and the torso -- are removed only when they are
    within half the tolerance, so a costume colour near the key survives.
    The key is the median corner colour: use one far from the palette (#00FF00).
    Images that already carry transparency are returned unchanged.
    """
    rgba = np.array(img.convert("RGBA"))
    if (rgba[..., 3] < 255).any():
        return Image.fromarray(rgba, "RGBA")
    h, w = rgba.shape[:2]
    corners = np.array([rgba[0, 0, :3], rgba[0, -1, :3], rgba[-1, 0, :3], rgba[-1, -1, :3]], dtype=np.int32)
    key = np.median(corners, axis=0)
    distance = np.sqrt(((rgba[..., :3].astype(np.int32) - key) ** 2).sum(axis=2))
    near = distance <= tolerance
    # Flood fill from the border: keep the "near" regions that touch an edge.
    labels, _ = ndimage.label(near)
    border = np.unique(np.concatenate([labels[0], labels[-1], labels[:, 0], labels[:, -1]]))
    seen = np.isin(labels, border[border > 0])
    rgba[seen | (distance <= tolerance / 2), 3] = 0
    return Image.fromarray(rgba, "RGBA")


def downscale(img: Image.Image, factor: float) -> Image.Image:
    """Area-averaged resize with hard alpha: no semi-transparent pixel-art edges."""
    w = max(1, round(img.width * factor))
    h = max(1, round(img.height * factor))
    arr = np.array(img, dtype=np.float32) / 255.0
    arr[..., :3] *= arr[..., 3:4]  # premultiply so the background doesn't bleed in
    small = np.array(Image.fromarray((arr * 255).astype(np.uint8), "RGBA").resize((w, h), Image.BOX),
                     dtype=np.float32) / 255.0
    alpha = small[..., 3:4]
    rgb = np.where(alpha > 0, small[..., :3] / np.maximum(alpha, 1e-6), 0)
    out = np.zeros((h, w, 4), dtype=np.uint8)
    out[..., :3] = np.clip(rgb * 255 + 0.5, 0, 255).astype(np.uint8)
    out[..., 3] = np.where(small[..., 3] >= 0.5, 255, 0)
    out[out[..., 3] == 0] = 0
    return Image.fromarray(out, "RGBA")


def _ramps(fighter: str) -> list[str]:
    return [n for n in palette.RAMPS if n.startswith(fighter + "_") or n in ("metal", "eye")]


def palette_colours(fighter: str) -> np.ndarray:
    colours = [palette.hex_rgb(c) for n in _ramps(fighter) for c in palette.RAMPS[n]]
    colours += [palette.hex_rgb(c) for c in palette.COLOURS.values()]
    return np.array(sorted(set(colours)), dtype=np.int32)


def line_colours(fighter: str) -> np.ndarray:
    """Outline colours: the first entry of every material ramp, plus ink."""
    colours = {palette.hex_rgb(palette.RAMPS[n][0]) for n in _ramps(fighter)} | {palette.hex_rgb(palette.COLOURS["ink"])}
    return np.array(sorted(colours), dtype=np.int32)


_WEIGHTS = np.array([3, 4, 2])  # weighted RGB: close enough to perceptual for limited palettes
_CLEAN = 3 * 9 * 12 ** 2        # within ~12 per channel of a palette colour = not a blended fringe


def _nearest(rgb: np.ndarray, colours: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Nearest palette colour per pixel, and the weighted squared distance to it."""
    unique, inverse = np.unique(rgb.reshape(-1, 3), axis=0, return_inverse=True)
    distances = (((unique[:, None, :].astype(np.int32) - colours[None, :, :]) ** 2) * _WEIGHTS).sum(axis=2)
    index = distances.argmin(axis=1)
    inverse = inverse.reshape(-1)
    return (colours[index][inverse].reshape(rgb.shape).astype(np.uint8),
            distances[np.arange(len(index)), index][inverse].reshape(rgb.shape[:-1]))


def edge_mask(alpha: np.ndarray) -> np.ndarray:
    """Opaque pixels with a transparent (or out-of-image) 4-neighbour."""
    solid = np.pad(alpha > 0, 1)
    inner = solid[1:-1, 1:-1]
    return inner & ~(solid[:-2, 1:-1] & solid[2:, 1:-1] & solid[1:-1, :-2] & solid[1:-1, 2:])


def snap_to_palette(img: Image.Image, colours: np.ndarray, outline: np.ndarray | None = None) -> Image.Image:
    """Snaps every pixel to `colours`; silhouette edges snap to `outline` colours when given.

    Only edge pixels that are off-palette (blends with the background key) are
    redirected: green blended into a dark outline would otherwise snap to teal.
    """
    source = np.array(img)
    arr = source.copy()
    arr[..., :3], distance = _nearest(source[..., :3], colours)
    if outline is not None:
        fringe = edge_mask(arr[..., 3]) & (distance > _CLEAN)
        arr[fringe, :3] = _nearest(source[fringe][:, :3], outline)[0]
    arr[arr[..., 3] == 0] = 0
    return Image.fromarray(arr, "RGBA")


def import_fighter(job: FighterImport, assets: Path) -> dict:
    files = sorted(job.source.glob("*.png"))
    if not files:
        raise SystemExit(f"no PNG frames in {job.source}")
    overrides = json.loads(job.manifest.read_text()) if job.manifest else {}

    keyed = {f.stem: key_background(Image.open(f), job.tolerance) for f in files}
    factor = job.scale
    if factor is None:
        if job.height is None or job.reference not in keyed:
            raise SystemExit(f"pass --scale, or --height together with a '{job.reference}' frame")
        box = keyed[job.reference].getbbox()
        if box is None:
            raise SystemExit(f"'{job.reference}' is empty after keying the background")
        factor = job.height / (job.origin[1] - box[1])

    colours = palette_colours(job.fighter) if job.quantize else None
    outline = line_colours(job.fighter)
    frames: list[atlas.Frame] = []
    for name, image in keyed.items():
        o = overrides.get(name, {})
        origin = o.get("origin", job.origin)
        small = downscale(image, factor)
        if colours is not None:
            small = snap_to_palette(small, colours, outline)
        partner = o.get("partner")
        frames.append(atlas.trim(name, small, (round(origin[0] * factor), round(origin[1] * factor)),
                                 tuple(partner) if partner else None))

    folder = assets / "Fighters" / job.fighter.capitalize()
    data = atlas.write(frames, folder / f"{job.fighter}_atlas.png",
                       extra={"$comment": f"Imported by Tools/Art/import_art.py from {job.source.name} "
                                          f"(scale {factor:.4f})."})
    missing = sorted(required_frames(folder / f"{job.fighter}.fighter.json") - set(keyed))
    print(f"imported {len(frames)} frames at scale {factor:.4f} into {folder}")
    if missing:
        print("missing frames referenced by the fighter definition: " + ", ".join(missing))
    if sunk := atlas.below_floor(frames):
        print("frames reaching below the floor (check --origin or the manifest): " + ", ".join(sunk))
    return data


def required_frames(fighter_json: Path) -> set[str]:
    if not fighter_json.exists():
        return set()
    definition = json.loads(fighter_json.read_text())
    names = {"held_0", "held_1"}
    for animation in definition.get("animations", {}).values():
        for frame in animation["frames"]:
            names.add(frame[0])
            if len(frame) > 2:
                names.add(frame[2])
    return names


def import_image(source: Path, target: Path, size: tuple[int, int], quantize: bool) -> None:
    """Stages, portraits: fit to the exact size, then optionally snap to the palette."""
    img = Image.open(source).convert("RGBA")
    scale = max(size[0] / img.width, size[1] / img.height)
    img = img.resize((round(img.width * scale), round(img.height * scale)), Image.BOX)
    left, top = (img.width - size[0]) // 2, (img.height - size[1]) // 2
    img = img.crop((left, top, left + size[0], top + size[1]))
    if quantize:
        all_colours = np.array(sorted({palette.hex_rgb(c) for c in palette.COLOURS.values()}), dtype=np.int32)
        img = snap_to_palette(img, all_colours)
    target.parent.mkdir(parents=True, exist_ok=True)
    img.save(target, optimize=True)
    print(f"imported {source.name} -> {target} ({size[0]}x{size[1]})")


REFERENCE_CANVAS = (256, 256)   # game px; every fighter frame fits
REFERENCE_ORIGIN = (128, 232)   # game px; feet on the ground


def export_references(fighter: str, assets: Path, target: Path, scale: int = 4,
                      key: tuple[int, int, int] = (0, 255, 0)) -> None:
    """Writes every frame of a fighter's atlas as a pose reference for outside generation.

    Each frame is placed on the same canvas with its origin at the same point and
    upscaled without smoothing, so images generated from these references import
    back with `--origin {REFERENCE_ORIGIN * scale} --scale {1 / scale}` and the
    written manifest (partner offsets).
    """
    folder = assets / "Fighters" / fighter.capitalize()
    data = json.loads((folder / f"{fighter}_atlas.json").read_text())
    sheet = Image.open(folder / data["image"]).convert("RGBA")
    target.mkdir(parents=True, exist_ok=True)
    manifest = {}
    for frame in data["frames"]:
        x, y, w, h = frame["source_rect"]
        px, py = frame["pivot_pixels"]
        canvas = Image.new("RGBA", REFERENCE_CANVAS, key + (255,))
        canvas.alpha_composite(sheet.crop((x, y, x + w, y + h)), (REFERENCE_ORIGIN[0] - px, REFERENCE_ORIGIN[1] - py))
        canvas.convert("RGB").resize((REFERENCE_CANVAS[0] * scale, REFERENCE_CANVAS[1] * scale), Image.NEAREST) \
            .save(target / f"{frame['name']}.png")
        if "partner" in frame:
            manifest[frame["name"]] = {"partner": frame["partner"]}
    # Frames the art doesn't have yet (new animations): the procedural pose, on the same canvas.
    from .characters import CHARACTERS
    from .figure import Canvas
    from .poses import PoseBook

    existing = {f["name"] for f in data["frames"]}
    new = []
    if fighter in CHARACTERS:
        character = CHARACTERS[fighter]
        for name, pose in PoseBook(character).frames().items():
            if name in existing:
                continue
            canvas = Canvas(REFERENCE_CANVAS[0], REFERENCE_CANVAS[1], REFERENCE_ORIGIN)
            character.draw(canvas, pose)
            image = Image.new("RGBA", REFERENCE_CANVAS, key + (255,))
            image.alpha_composite(canvas.render())
            image.convert("RGB").resize((REFERENCE_CANVAS[0] * scale, REFERENCE_CANVAS[1] * scale), Image.NEAREST) \
                .save(target / f"{name}.png")
            new.append(name)
    (target / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    ox, oy = REFERENCE_ORIGIN[0] * scale, REFERENCE_ORIGIN[1] * scale
    print(f"exported {len(data['frames'])} references to {target}")
    if new:
        print(f"  plus {len(new)} procedural references for frames not drawn yet: {', '.join(new)}")
    print(f"import back with: import_art.py fighter {fighter} <frames> --origin {ox} {oy} --scale {1 / scale:g} "
          f"--manifest {target / 'manifest.json'}")
