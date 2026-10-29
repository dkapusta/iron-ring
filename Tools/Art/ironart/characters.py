"""Fighter builds, costumes and the skeleton -> primitives mapping.

Character designs follow docs/design/setting.md and docs/art/asset-spec.md.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field, replace

import numpy as np

from .figure import (FAR_ARM, FAR_LEG, HEAD, NEAR_ARM, NEAR_LEG, TORSO, Canvas, Vec, add, normalise, rot, scale,
                     sub, two_bone_ik)


@dataclass(frozen=True)
class Build:
    ankle: float          # sole to ankle joint
    shin: float
    thigh: float
    spine: float          # hip joint to shoulder line
    neck: float
    upper_arm: float
    forearm: float
    head: tuple[float, float]           # radii
    thigh_r: tuple[float, float]
    shin_r: tuple[float, float]
    upper_r: tuple[float, float]
    fore_r: tuple[float, float]
    neck_r: float
    waist_r: float
    chest_r: float
    pelvis: tuple[float, float]
    fist: tuple[float, float]
    foot: float           # ankle to toe
    deltoid: float

    @property
    def hip_height(self) -> float:
        return self.ankle + self.shin + self.thigh


@dataclass
class Pose:
    pelvis: Vec                       # hip joint centre
    near_foot: Vec                    # ankle positions
    far_foot: Vec
    near_hand: Vec                    # fist / wrist centre
    far_hand: Vec
    lean: float = 0.0                 # torso, degrees, + leans forward
    head: float = 0.0                 # extra head tilt, + looks down
    near_toe: float = 0.0             # foot pitch, degrees, + toe up
    far_toe: float = 0.0
    near_knee: float = 1.0            # +1 bends forward
    far_knee: float = 1.0
    near_elbow: float = -1.0          # -1 bends down
    far_elbow: float = -1.0
    near_open: bool = False           # open hand instead of fist
    far_open: bool = False
    rotate: float = 0.0               # whole-body rotation about the pelvis (falls, lifts)
    partner: Vec | None = None        # where a held opponent's origin goes
    expression: str = "neutral"       # neutral | hurt | shout


@dataclass
class Skeleton:
    pelvis: Vec
    chest: Vec
    neck_base: Vec
    head: Vec
    head_angle: float
    lean: float
    near_hip: Vec
    far_hip: Vec
    near_knee: Vec
    far_knee: Vec
    near_ankle: Vec
    far_ankle: Vec
    near_toe: Vec
    far_toe: Vec
    near_shoulder: Vec
    far_shoulder: Vec
    near_elbow: Vec
    far_elbow: Vec
    near_wrist: Vec
    far_wrist: Vec


@dataclass
class Character:
    id: str
    build: Build
    skin: str
    materials: dict[str, str] = field(default_factory=dict)

    # --- skeleton -------------------------------------------------------

    def solve(self, p: Pose) -> Skeleton:
        b = self.build
        up = rot((0.0, 1.0), -p.lean)          # torso axis
        fwd = rot((1.0, 0.0), -p.lean)
        pelvis = p.pelvis
        shoulder_mid = add(pelvis, scale(up, b.spine))
        chest = add(pelvis, scale(up, b.spine * 0.62))
        neck_base = add(pelvis, scale(up, b.spine + 1.0))
        head_dir = rot(up, -p.head * 0.5)
        head = add(neck_base, scale(head_dir, b.neck + b.head[1] * 0.8))

        near_hip = add(pelvis, scale(fwd, 1.5))
        far_hip = add(pelvis, scale(fwd, -1.5))
        near_shoulder = add(add(shoulder_mid, scale(fwd, 2.0)), scale(up, -1.0))
        far_shoulder = add(add(shoulder_mid, scale(fwd, -2.5)), scale(up, -1.5))

        near_knee, near_ankle = two_bone_ik(near_hip, p.near_foot, b.thigh, b.shin, p.near_knee)
        far_knee, far_ankle = two_bone_ik(far_hip, p.far_foot, b.thigh, b.shin, p.far_knee)
        near_toe = add(near_ankle, rot((b.foot, 0.0), p.near_toe))
        far_toe = add(far_ankle, rot((b.foot, 0.0), p.far_toe))
        # Elbow bend sign is relative to the shoulder->hand direction.
        near_elbow, near_wrist = two_bone_ik(near_shoulder, p.near_hand, b.upper_arm, b.forearm, p.near_elbow)
        far_elbow, far_wrist = two_bone_ik(far_shoulder, p.far_hand, b.upper_arm, b.forearm, p.far_elbow)

        s = Skeleton(pelvis, chest, neck_base, head, p.lean + p.head, p.lean, near_hip, far_hip, near_knee,
                     far_knee, near_ankle, far_ankle, near_toe, far_toe, near_shoulder, far_shoulder, near_elbow,
                     far_elbow, near_wrist, far_wrist)
        if p.rotate:
            s = _rotate_skeleton(s, p.pelvis, p.rotate)
        return s

    # --- drawing ----------------------------------------------------------

    def draw(self, canvas: Canvas, pose: Pose) -> Skeleton:
        s = self.solve(pose)
        self.draw_arm(canvas, s, pose, near=False)
        self.draw_leg(canvas, s, near=False)
        self.draw_torso(canvas, s, pose)
        self.draw_leg(canvas, s, near=True)
        self.draw_head(canvas, s, pose)
        self.draw_arm(canvas, s, pose, near=True)
        return s

    def mat(self, canvas: Canvas, role: str) -> int:
        return canvas.material_id(self.materials[role])

    def draw_leg(self, canvas: Canvas, s: Skeleton, near: bool) -> None:
        b = self.build
        group = NEAR_LEG if near else FAR_LEG
        dark = 0.0 if near else 0.14
        hip, knee, ankle, toe = ((s.near_hip, s.near_knee, s.near_ankle, s.near_toe) if near else
                                 (s.far_hip, s.far_knee, s.far_ankle, s.far_toe))
        canvas.capsule(hip, knee, b.thigh_r[0], b.thigh_r[1], self.thigh_material(canvas, near), group, dark)
        canvas.capsule(knee, ankle, b.shin_r[0], b.shin_r[1], self.shin_material(canvas), group, dark)
        boot = self.materials["boots"]
        heel = add(ankle, scale(normalise(sub(ankle, toe)), 2.0))
        sole = (0.0, -(b.ankle - (b.shin_r[1] + 0.6)))  # sole rests exactly on the ground
        canvas.capsule(add(heel, sole), add(toe, sole), b.shin_r[1] + 0.6, b.shin_r[1] - 0.4, boot, group, dark)
        canvas.capsule(heel, add(ankle, (0, 1.5)), b.shin_r[1] + 0.5, b.shin_r[1], boot, group, dark)

    def draw_arm(self, canvas: Canvas, s: Skeleton, pose: Pose, near: bool) -> None:
        b = self.build
        group = NEAR_ARM if near else FAR_ARM
        dark = 0.0 if near else 0.14
        shoulder, elbow, wrist = ((s.near_shoulder, s.near_elbow, s.near_wrist) if near else
                                  (s.far_shoulder, s.far_elbow, s.far_wrist))
        canvas.ellipse(shoulder, b.deltoid, b.deltoid, 0.0, self.materials["deltoid"], group, dark)
        canvas.capsule(shoulder, elbow, b.upper_r[0], b.upper_r[1], self.upper_arm_material(canvas), group, dark)
        canvas.capsule(elbow, wrist, b.fore_r[0], b.fore_r[1], self.forearm_material(canvas), group, dark)
        direction = normalise(sub(wrist, elbow))
        angle = math.degrees(math.atan2(direction[1], direction[0]))
        open_hand = pose.near_open if near else pose.far_open
        centre = add(wrist, scale(direction, b.fist[0] * 0.6))
        if open_hand:
            canvas.ellipse(add(centre, scale(direction, 1.5)), b.fist[0] + 1.5, b.fist[1] - 1.0, angle,
                           self.materials["hand_open"], group, dark)
        else:
            canvas.ellipse(centre, b.fist[0], b.fist[1], angle, self.materials["hand"], group, dark)

    def draw_torso(self, canvas: Canvas, s: Skeleton, pose: Pose) -> None:
        b = self.build
        up = normalise(sub(s.chest, s.pelvis))
        canvas.ellipse(s.pelvis, b.pelvis[0], b.pelvis[1], -s.lean, self.materials["pelvis"], TORSO)
        waist = add(s.pelvis, scale(up, b.pelvis[1] * 0.6))
        canvas.capsule(waist, s.chest, b.waist_r, b.chest_r, self.torso_material(canvas), TORSO)
        # Chest front and back mass, so the torso isn't a tube.
        fwd = (up[1], -up[0])
        canvas.ellipse(add(s.chest, scale(fwd, b.chest_r * 0.35)), b.chest_r * 0.95, b.chest_r * 1.05, -s.lean,
                       self.torso_material(canvas, top=True), TORSO)
        canvas.capsule(s.chest, s.neck_base, b.chest_r * 0.9, b.neck_r + 1.0, self.torso_material(canvas, top=True),
                       TORSO)
        self.torso_details(canvas, s)

    def draw_head(self, canvas: Canvas, s: Skeleton, pose: Pose) -> None:
        b = self.build
        canvas.capsule(s.neck_base, s.head, b.neck_r, b.neck_r, self.skin, HEAD)
        rx, ry = b.head
        angle = -s.head_angle
        canvas.ellipse(s.head, rx, ry, angle, self.head_material(canvas), HEAD)
        # Jaw and chin, slightly forward.
        jaw = add(s.head, rot((rx * 0.35, -ry * 0.55), angle))
        canvas.ellipse(jaw, rx * 0.62, ry * 0.42, angle, self.jaw_material(canvas), HEAD)
        self.face(canvas, s, pose)

    def local(self, s: Skeleton, x: float, y: float) -> Vec:
        """Point on the head, in head radii (x forward, y up)."""
        rx, ry = self.build.head
        return add(s.head, rot((x * rx, y * ry), -s.head_angle))

    # --- per-character hooks ---------------------------------------------

    def thigh_material(self, canvas: Canvas, near: bool):
        return self.materials["legs"]

    def shin_material(self, canvas: Canvas):
        legs, boots = self.mat(canvas, "legs"), self.mat(canvas, "boots")
        return lambda t, side: np.where(t > 0.86, boots, legs).astype(np.int16)

    def upper_arm_material(self, canvas: Canvas):
        return self.skin

    def forearm_material(self, canvas: Canvas):
        return self.skin

    def torso_material(self, canvas: Canvas, top: bool = False):
        return self.materials["top"]

    def head_material(self, canvas: Canvas):
        return self.skin

    def jaw_material(self, canvas: Canvas):
        return self.skin

    def torso_details(self, canvas: Canvas, s: Skeleton) -> None:
        pass

    def face(self, canvas: Canvas, s: Skeleton, pose: Pose) -> None:
        pass


def _rotate_skeleton(s: Skeleton, centre: Vec, degrees: float) -> Skeleton:
    values = {}
    for name, value in s.__dict__.items():
        if isinstance(value, tuple):
            values[name] = add(centre, rot(sub(value, centre), degrees))
        else:
            values[name] = value
    values["head_angle"] = s.head_angle - degrees
    values["lean"] = s.lean - degrees
    return Skeleton(**values)


# --------------------------------------------------------------------------
# Dee Marlowe: boxer. Teal sleeveless top, charcoal track pants with a chalk
# side stripe, taped hands and forearms, black boxing boots, cropped curls.

class Dee(Character):
    def __init__(self) -> None:
        super().__init__(
            id="dee",
            build=Build(ankle=5, shin=32, thigh=32, spine=44, neck=4, upper_arm=24, forearm=22,
                        head=(7.8, 9.8), thigh_r=(8.0, 5.6), shin_r=(5.4, 3.6), upper_r=(4.6, 3.8),
                        fore_r=(3.8, 3.2), neck_r=3.6, waist_r=8.6, chest_r=11.0, pelvis=(10.5, 8.0),
                        fist=(4.4, 4.0), foot=13, deltoid=5.0),
            skin="dee_skin",
            materials={"legs": "dee_pants", "boots": "dee_boots", "top": "dee_top", "pelvis": "dee_pants",
                       "deltoid": "dee_skin", "hand": "dee_wraps", "hand_open": "dee_wraps"},
        )

    def thigh_material(self, canvas: Canvas, near: bool):
        pants, stripe = self.mat(canvas, "legs"), canvas.material_id("dee_stripe")
        if not near:
            return lambda t, side: np.full(t.shape, pants, dtype=np.int16)
        return lambda t, side: np.where(np.abs(side + 0.05) < 0.16, stripe, pants).astype(np.int16)

    def shin_material(self, canvas: Canvas):
        pants, boots = self.mat(canvas, "legs"), self.mat(canvas, "boots")
        stripe = canvas.material_id("dee_stripe")
        return lambda t, side: np.where(t > 0.80, boots, np.where(np.abs(side + 0.05) < 0.18, stripe, pants)
                                        ).astype(np.int16)

    def forearm_material(self, canvas: Canvas):
        skin, wraps = canvas.material_id("dee_skin"), canvas.material_id("dee_wraps")
        return lambda t, side: np.where(t > 0.45, wraps, skin).astype(np.int16)

    def torso_material(self, canvas: Canvas, top: bool = False):
        shirt, pants = canvas.material_id("dee_top"), canvas.material_id("dee_pants")
        if top:
            return "dee_top"
        return lambda t, side: np.where(t < 0.12, pants, shirt).astype(np.int16)

    def head_material(self, canvas: Canvas):
        skin, hair = canvas.material_id("dee_skin"), canvas.material_id("dee_hair")
        # Hairline slants from low at the back to high at the front.
        return lambda ux, uy: np.where(uy > 0.30 + 0.55 * ux, hair, skin).astype(np.int16)

    def face(self, canvas: Canvas, s: Skeleton, pose: Pose) -> None:
        # Cropped curls: bumps along the top and back of the skull.
        for i in range(9):
            a = math.radians(70 + i * 17)
            p = self.local(s, math.cos(a) * 0.98, math.sin(a) * 1.0)
            canvas.ellipse(p, 2.3, 2.1, 0.0, "dee_hair", HEAD)
        canvas.ellipse(self.local(s, -0.15, 0.05), 1.3, 1.9, 0.0, "dee_skin", HEAD, 0.2)  # ear
        eye = self.local(s, 0.52, 0.12)
        canvas.pixel(eye, "eye", 0, HEAD)
        canvas.pixel(add(eye, (-1, 0)), "dee_skin", 1, HEAD)
        canvas.pixel(self.local(s, 0.48, 0.34), "dee_hair", 1, HEAD)  # brow
        canvas.pixel(add(self.local(s, 0.48, 0.34), (1, 0)), "dee_hair", 1, HEAD)
        mouth = self.local(s, 0.62, -0.50)
        canvas.pixel(mouth, "dee_skin", 1, HEAD)
        if pose.expression == "shout":
            canvas.pixel(add(mouth, (0, -1)), "eye", 0, HEAD)
        canvas.pixel(self.local(s, 1.02, -0.05), "dee_skin", 3, HEAD)  # nose tip
        canvas.pixel(self.local(s, 0.05, -0.10), "metal", 4, HEAD)   # earring


# --------------------------------------------------------------------------
# Bram Hollis: grappler. Shaved head, full dark beard, charcoal T-shirt, rust
# work overalls with the bib folded down, leather wrist straps, work boots.

class Bram(Character):
    def __init__(self) -> None:
        super().__init__(
            id="bram",
            build=Build(ankle=6, shin=35, thigh=36, spine=47, neck=3, upper_arm=26, forearm=24,
                        head=(8.6, 10.2), thigh_r=(10.0, 7.0), shin_r=(7.0, 4.8), upper_r=(6.6, 5.4),
                        fore_r=(5.6, 4.6), neck_r=5.6, waist_r=12.0, chest_r=15.0, pelvis=(13.5, 9.5),
                        fist=(5.4, 5.0), foot=15, deltoid=7.0),
            skin="bram_skin",
            materials={"legs": "bram_overall", "boots": "bram_boots", "top": "bram_shirt", "pelvis": "bram_overall",
                       "deltoid": "bram_shirt", "hand": "bram_skin", "hand_open": "bram_skin"},
        )

    def upper_arm_material(self, canvas: Canvas):
        shirt, skin = canvas.material_id("bram_shirt"), canvas.material_id("bram_skin")
        return lambda t, side: np.where(t < 0.40, shirt, skin).astype(np.int16)

    def forearm_material(self, canvas: Canvas):
        skin, strap = canvas.material_id("bram_skin"), canvas.material_id("bram_straps")
        return lambda t, side: np.where(t > 0.68, strap, skin).astype(np.int16)

    def torso_material(self, canvas: Canvas, top: bool = False):
        shirt, overall = canvas.material_id("bram_shirt"), canvas.material_id("bram_overall")
        belt = canvas.material_id("bram_straps")
        if top:
            return "bram_shirt"
        return lambda t, side: np.where(t < 0.16, overall, np.where(t < 0.24, belt, shirt)).astype(np.int16)

    def jaw_material(self, canvas: Canvas):
        skin, beard = canvas.material_id("bram_skin"), canvas.material_id("bram_beard")
        return lambda ux, uy: np.where(uy < 0.55, beard, skin).astype(np.int16)

    def head_material(self, canvas: Canvas):
        skin, beard = canvas.material_id("bram_skin"), canvas.material_id("bram_beard")
        return lambda ux, uy: np.where((uy < -0.18 - 0.25 * ux) & (ux > -0.55), beard, skin).astype(np.int16)

    def torso_details(self, canvas: Canvas, s: Skeleton) -> None:
        # Overall straps hanging from the folded bib, down the hip.
        b = self.build
        up = normalise(sub(s.chest, s.pelvis))
        fwd = (up[1], -up[0])
        waist = add(s.pelvis, scale(up, b.pelvis[1] * 0.9))
        for offset in (b.pelvis[0] * 0.55, -b.pelvis[0] * 0.65):
            top = add(waist, scale(fwd, offset))
            bottom = add(add(top, scale(up, -16)), scale(fwd, 1.5))
            canvas.capsule(top, bottom, 1.4, 1.2, "bram_straps", TORSO)

    def face(self, canvas: Canvas, s: Skeleton, pose: Pose) -> None:
        canvas.ellipse(self.local(s, -0.18, 0.02), 1.5, 2.1, 0.0, "bram_skin", HEAD, 0.2)  # ear
        eye = self.local(s, 0.50, 0.14)
        canvas.pixel(eye, "eye", 0, HEAD)
        for dx in (-1, 0, 1, 2):  # heavy brow
            canvas.pixel(add(self.local(s, 0.48, 0.32), (dx, 0)), "bram_beard", 1, HEAD)
        canvas.pixel(self.local(s, 1.03, -0.02), "bram_skin", 3, HEAD)  # nose
        canvas.pixel(add(self.local(s, 1.03, -0.02), (0, -1)), "bram_skin", 2, HEAD)
        if pose.expression == "shout":
            canvas.pixel(self.local(s, 0.70, -0.45), "eye", 0, HEAD)


CHARACTERS = {"dee": Dee(), "bram": Bram()}


def mirrored(pose: Pose, **changes) -> Pose:
    return replace(pose, **changes)
