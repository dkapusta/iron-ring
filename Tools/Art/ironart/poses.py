"""Key poses for every animation frame, per fighter.

Frame names are the contract with the fighter definitions
(Assets/Fighters/<Name>/<id>.fighter.json); docs/art/asset-spec.md lists the
complete frame set an external artist must deliver.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, replace

from .characters import Character, Pose
from .figure import Vec, add, rot


@dataclass(frozen=True)
class Style:
    front: float          # lead foot x
    back: float           # rear foot x
    crouch: float         # pelvis drop in stance (knee bend)
    lean: float           # torso lean in stance
    guard_near: Vec       # hands relative to the shoulder line centre
    guard_far: Vec
    open_hands: bool      # grappler: open palms in stance
    # Active-frame strike points (absolute, figure space) the art must hit.
    jab: Vec
    straight: Vec
    kick: Vec
    sweep: Vec


STYLES = {
    "dee": Style(front=14, back=-15, crouch=7, lean=9, guard_near=(13, -7), guard_far=(9, -2), open_hands=False,
                 jab=(50, 92), straight=(60, 96), kick=(66, 62), sweep=(74, 6)),
    "bram": Style(front=17, back=-19, crouch=10, lean=14, guard_near=(22, -16), guard_far=(16, -9), open_hands=True,
                  jab=(52, 92), straight=(60, 100), kick=(70, 58), sweep=(80, 6)),
}


class PoseBook:
    """Builds every named frame pose for one character."""

    def __init__(self, character: Character) -> None:
        self.c = character
        self.b = character.build
        self.s = STYLES[character.id]
        self.hip = self.b.hip_height

    # --- helpers ------------------------------------------------------------

    def shoulders(self, pelvis: Vec, lean: float) -> Vec:
        return add(pelvis, rot((0.0, self.b.spine), -lean))

    def stance(self, bob: float = 0.0, lean: float | None = None, dx: float = 0.0, **changes) -> Pose:
        s, b = self.s, self.b
        lean = s.lean if lean is None else lean
        pelvis = (-1.0 + dx, self.hip - s.crouch + bob)
        sh = self.shoulders(pelvis, lean)
        pose = Pose(pelvis=pelvis, lean=lean,
                    near_foot=(s.front, b.ankle), far_foot=(s.back, b.ankle),
                    near_hand=add(sh, (s.guard_near[0], s.guard_near[1] + bob * 0.5)),
                    far_hand=add(sh, (s.guard_far[0], s.guard_far[1] + bob * 0.3)),
                    near_open=s.open_hands, far_open=s.open_hands)
        return replace(pose, **changes)

    def lying(self, pelvis_x: float = 0.0, **changes) -> Pose:
        """On the back, head towards -x: author upright, rotate +90 about the pelvis."""
        b = self.b
        pelvis = (pelvis_x, self.b.chest_r + 4)
        down = self.hip - b.ankle
        pose = Pose(pelvis=pelvis, lean=0, head=-10, rotate=90,
                    near_foot=add(pelvis, (6, -down + 2)), far_foot=add(pelvis, (-3, -down + 4)),
                    near_hand=add(pelvis, (12, 8)), far_hand=add(pelvis, (9, 18)),
                    near_knee=1, far_knee=1, expression="hurt")
        return replace(pose, **changes)

    # --- animation frames -------------------------------------------------

    def frames(self) -> dict[str, Pose]:
        f: dict[str, Pose] = {}
        s, b, hip = self.s, self.b, self.hip

        for i, bob in enumerate((0, -1, -2, -1)):
            f[f"idle_{i}"] = self.stance(bob=bob)

        for i in range(6):
            phase = i / 6 * 2 * math.pi
            near_lift = 4 * max(0.0, math.sin(phase))
            far_lift = 4 * max(0.0, -math.sin(phase))
            f[f"walk_{i}"] = self.stance(
                bob=-1.5 * abs(math.sin(phase)), dx=1.5 * math.cos(phase),
                near_foot=(s.front + 5 * math.cos(phase), b.ankle + near_lift),
                far_foot=(s.back - 5 * math.cos(phase), b.ankle + far_lift))

        crouch_pelvis = (-3.0, hip * 0.47)
        csh = self.shoulders(crouch_pelvis, 30)
        f["crouch"] = Pose(pelvis=crouch_pelvis, lean=30, near_foot=(s.front + 3, b.ankle),
                           far_foot=(s.back - 3, b.ankle), near_toe=0, far_toe=8,
                           near_hand=add(csh, (s.guard_near[0], s.guard_near[1] + 2)),
                           far_hand=add(csh, (s.guard_far[0], s.guard_far[1] + 2)),
                           near_open=s.open_hands, far_open=s.open_hands)

        gsh = self.shoulders((-3.0, hip - s.crouch - 1), 3)
        f["guard"] = self.stance(lean=3, dx=-2, near_hand=add(gsh, (11, 12)), far_hand=add(gsh, (8, 7)),
                                 near_open=False, far_open=False, near_elbow=-1, head=6)
        f["guard_crouch"] = replace(f["crouch"], near_hand=add(csh, (11, 12)), far_hand=add(csh, (8, 7)),
                                    near_open=False, far_open=False, head=6)

        f["jump_rise"] = self.stance(lean=6, bob=-2, near_foot=(8, 22), far_foot=(-10, 14))
        f["jump_apex"] = self.stance(lean=10, bob=-6, near_foot=(12, 38), far_foot=(-4, 30))
        f["jump_fall"] = self.stance(lean=4, bob=-2, near_foot=(10, 10), far_foot=(-12, 6))

        # Light punch: chamber, extension (active), retraction.
        f["lp_0"] = self.stance(lean=s.lean + 4, near_hand=add(self.shoulders((-1, hip - s.crouch), s.lean + 4),
                                                               (7, -9)), near_open=False)
        f["lp_1"] = self.stance(lean=s.lean + 8, dx=3, near_foot=(s.front + 3, b.ankle),
                                near_hand=s.jab, near_open=False, near_elbow=-1)
        f["lp_2"] = self.stance(lean=s.lean + 5, dx=1, near_hand=add(s.jab, (-16, 4)), near_open=False)

        # Heavy punch: wind-up, rear straight (active), follow-through, recovery.
        f["hp_0"] = self.stance(lean=s.lean - 4, dx=-2, far_hand=add(self.shoulders((-3, hip - s.crouch), 5),
                                                                       (-2, -8)), far_open=False, near_open=False)
        f["hp_1"] = self.stance(lean=s.lean + 18, dx=5, near_foot=(s.front + 6, b.ankle), far_toe=14,
                                far_hand=s.straight, far_open=False, near_open=False,
                                near_hand=add(self.shoulders((4, hip - s.crouch), s.lean + 18), (6, -14)),
                                expression="shout")
        f["hp_2"] = replace(f["hp_1"], far_hand=add(s.straight, (-6, -5)), lean=s.lean + 14, expression="neutral")
        f["hp_3"] = self.stance(lean=s.lean + 6, dx=2, far_hand=add(s.straight, (-26, 2)), far_open=False)

        # Light kick: knee chamber, front kick (active), retraction.
        kick_pelvis = (3.0, hip - 4)
        ksh = self.shoulders(kick_pelvis, -10)
        guard_hands = dict(near_hand=add(ksh, (12, -6)), far_hand=add(ksh, (6, -2)), near_open=False, far_open=False)
        f["lk_0"] = Pose(pelvis=(1.0, hip - 5), lean=-4, near_foot=(12, 40), far_foot=(s.back + 4, b.ankle),
                         near_toe=-30, **{**guard_hands})
        f["lk_1"] = Pose(pelvis=kick_pelvis, lean=-12, near_foot=(s.kick[0] - b.foot * 0.6, s.kick[1]),
                         far_foot=(s.back + 6, b.ankle), near_toe=8, far_toe=10, near_knee=1, **guard_hands)
        f["lk_2"] = Pose(pelvis=(2.0, hip - 5), lean=-2, near_foot=(18, 30), far_foot=(s.back + 4, b.ankle),
                         near_toe=-20, **guard_hands)

        # Sweep: drop, sweep (active), follow-through, recover.
        low = hip * 0.42
        lsh = self.shoulders((4, low), 34)
        f["hk_0"] = Pose(pelvis=(0.0, hip * 0.5), lean=30, near_foot=(s.front + 8, b.ankle),
                         far_foot=(s.back, b.ankle), far_toe=20, near_hand=add(lsh, (10, -6)),
                         far_hand=(s.back + 10, 6), far_open=True, near_open=s.open_hands)
        f["hk_1"] = Pose(pelvis=(4.0, low), lean=36, near_foot=(s.sweep[0] - b.foot * 0.6, s.sweep[1] + 1),
                         far_foot=(-8, b.ankle), far_knee=1, far_toe=30, near_toe=-6,
                         near_hand=add(lsh, (10, -10)), far_hand=(-4, 5), far_open=True, near_open=s.open_hands)
        f["hk_2"] = replace(f["hk_1"], near_foot=(s.sweep[0] - b.foot * 0.6 - 14, s.sweep[1] + 3), lean=32)
        f["hk_3"] = replace(f["crouch"], lean=28)

        # Throw attempt: reach, grab.
        tsh = self.shoulders((3, hip - s.crouch), s.lean + 10)
        f["throw_0"] = self.stance(lean=s.lean + 10, dx=4, near_foot=(s.front + 5, b.ankle),
                                   near_hand=add(tsh, (30, -8)), far_hand=add(tsh, (24, -16)),
                                   near_open=True, far_open=True)
        f["throw_1"] = self.stance(lean=s.lean + 8, dx=3, near_foot=(s.front + 4, b.ankle),
                                   near_hand=add(tsh, (24, -10)), far_hand=add(tsh, (20, -18)),
                                   near_open=False, far_open=False)
        # Throw hold (victim held at +30 px), then the toss.
        f["throw_hold_0"] = replace(f["throw_1"], partner=(30, 0))
        hsh = self.shoulders((-4, hip - s.crouch + 2), -18)
        f["throw_hold_1"] = Pose(pelvis=(-4, hip - s.crouch + 2), lean=-18, near_foot=(s.front - 2, b.ankle),
                                 far_foot=(s.back - 4, b.ankle), near_toe=20,
                                 near_hand=(30, 70), far_hand=(25, 62), near_open=False, far_open=False, partner=(34, 34),
                                 expression="shout")

        # Hit reactions.
        hit_pelvis = (-3.0, hip - s.crouch)
        hsh0 = self.shoulders(hit_pelvis, -14)
        f["hit_0"] = Pose(pelvis=hit_pelvis, lean=-14, head=-14, near_foot=(s.front, b.ankle),
                          far_foot=(s.back, b.ankle), near_hand=add(hsh0, (6, -32)), far_hand=add(hsh0, (-10, -30)),
                          near_open=True, far_open=True, expression="hurt")
        hsh1 = self.shoulders((-5.0, hip - s.crouch - 2), -22)
        f["hit_1"] = Pose(pelvis=(-5.0, hip - s.crouch - 2), lean=-22, head=-18, near_foot=(s.front - 2, b.ankle),
                          far_foot=(s.back - 2, b.ankle), near_hand=add(hsh1, (2, -34)),
                          far_hand=add(hsh1, (-12, -30)), near_open=True, far_open=True, expression="hurt")
        f["hit_crouch"] = replace(f["crouch"], lean=8, head=-14, expression="hurt", near_open=True, far_open=True)

        # Held by a thrower: recoil upright, then airborne on the back.
        f["held_0"] = replace(f["hit_1"], lean=-10)
        f["held_1"] = self.lying(near_foot=add((0, 30), (14, -40)), far_foot=add((0, 30), (2, -44)),
                                 pelvis=(0.0, 30.0), near_hand=(10, 40), far_hand=(-12, 44))

        # Knockdown: falling, lying (also KO), sitting up, rising.
        fall = self.stance(lean=-30, bob=-14, near_foot=(26, 22), far_foot=(18, 14), expression="hurt",
                           near_open=True, far_open=True)
        f["down_0"] = replace(fall, rotate=36, pelvis=(-6.0, hip * 0.55))
        f["down_1"] = self.lying()
        sit = (-4.0, b.pelvis[1] + 2)
        f["down_2"] = Pose(pelvis=sit, lean=-28, near_foot=(30, b.ankle), far_foot=(24, b.ankle), near_knee=1,
                           far_knee=1, near_hand=(-22, 8), far_hand=(-18, 8), near_open=True, far_open=True)
        f["down_3"] = replace(f["crouch"], lean=30, head=10)

        f["win_0"], f["win_1"] = self.victory()
        f.update(self.champion())
        f.update(self.specials())
        return f

    def champion(self) -> dict[str, Pose]:
        """Arcade ending: the champion raises a length of the ring's chain overhead.

        champion_0..2 play once (turn to the crowd, lift, hold high), then
        champion_3..5 loop (a slow breath, then a shout). The chain itself is
        drawn by the artist between the hands; these poses place the hands.
        """
        b, hip = self.b, self.hip
        f: dict[str, Pose] = {}
        pelvis = (0.0, hip - 2)
        sh = self.shoulders(pelvis, 0)
        feet = {"near_foot": (12, b.ankle), "far_foot": (-14, b.ankle)}
        f["champion_0"] = self.stance(lean=2, bob=2, near_hand=add(sh, (10, -30)), far_hand=add(sh, (-6, -32)),
                                      near_open=False, far_open=False, **feet)
        f["champion_1"] = self.stance(lean=-2, bob=3, near_hand=add(sh, (18, 6)), far_hand=add(sh, (-12, 4)),
                                      near_elbow=1, far_elbow=1, **feet)
        if self.c.id == "dee":
            # Both fists overhead, the chain stretched between them.
            high = {"near_hand": add(sh, (12, 46)), "far_hand": add(sh, (-10, 46)), "near_elbow": 1, "far_elbow": 1}
        else:
            # Arms wide and high, the chain held taut across them.
            high = {"near_hand": add(sh, (30, 40)), "far_hand": add(sh, (-26, 40)), "near_elbow": 1, "far_elbow": 1}
        f["champion_2"] = self.stance(lean=-4, bob=4, expression="shout", **high, **feet)
        f["champion_3"] = self.stance(lean=-4, bob=4, **high, **feet)
        f["champion_4"] = self.stance(lean=-4, bob=3, **high, **feet)
        f["champion_5"] = self.stance(lean=-6, bob=4, head=-8, expression="shout", **high, **feet)
        return f

    def victory(self) -> tuple[Pose, Pose]:
        b, hip = self.b, self.hip
        pelvis = (0.0, hip - 3)
        sh = self.shoulders(pelvis, 0)
        if self.c.id == "dee":
            a = self.stance(lean=0, bob=4, near_hand=add(sh, (10, 40)), far_hand=add(sh, (2, -18)),
                            near_foot=(10, b.ankle), far_foot=(-12, b.ankle), near_elbow=1)
            return a, replace(a, near_hand=add(sh, (12, 44)), expression="shout")
        # Bram: double biceps.
        a = self.stance(lean=-2, bob=2, near_hand=add(sh, (20, 30)), far_hand=add(sh, (-14, 32)),
                        near_foot=(14, b.ankle), far_foot=(-16, b.ankle), near_elbow=-1, far_elbow=1,
                        near_open=False, far_open=False)
        return a, replace(a, expression="shout", near_hand=add(sh, (22, 36)), far_hand=add(sh, (-16, 38)))

    def specials(self) -> dict[str, Pose]:
        s, b, hip = self.s, self.b, self.hip
        f: dict[str, Pose] = {}
        if self.c.id == "dee":
            # Rush Uppercut (236 + P): dip, rush, uppercut (active), recovery.
            dip = (2.0, hip * 0.66)
            f["special_0"] = Pose(pelvis=dip, lean=26, near_foot=(s.front + 4, b.ankle), far_foot=(s.back, b.ankle),
                                  near_hand=add(self.shoulders(dip, 26), (10, -24)),
                                  far_hand=add(self.shoulders(dip, 26), (10, -6)), far_toe=16)
            rush = (8.0, hip * 0.74)
            f["special_1"] = Pose(pelvis=rush, lean=30, near_foot=(28, b.ankle), far_foot=(-22, b.ankle + 3),
                                  far_toe=30, near_hand=add(self.shoulders(rush, 30), (14, -26)),
                                  far_hand=add(self.shoulders(rush, 30), (8, -6)))
            rise = (6.0, hip + 4)
            f["special_2"] = Pose(pelvis=rise, lean=-6, near_foot=(14, b.ankle + 6), far_foot=(-12, b.ankle + 12),
                                  near_toe=-20, far_toe=-30, near_hand=(22, 152), near_elbow=-1,
                                  far_hand=add(self.shoulders(rise, -6), (4, -18)), expression="shout")
            f["special_3"] = Pose(pelvis=(3.0, hip - 4), lean=4, near_foot=(s.front, b.ankle),
                                  far_foot=(s.back, b.ankle), near_hand=(22, 128), near_elbow=-1,
                                  far_hand=add(self.shoulders((3, hip - 4), 4), (8, -4)))
        else:
            # Dock Hoist (63214 + P): reach low, grab; on connect: hoist, slam.
            reach = (6.0, hip * 0.72)
            rsh = self.shoulders(reach, 30)
            f["special_0"] = Pose(pelvis=reach, lean=30, near_foot=(s.front + 8, b.ankle), far_foot=(s.back, b.ankle),
                                  far_toe=12, near_hand=add(rsh, (34, -20)), far_hand=add(rsh, (28, -28)),
                                  near_open=True, far_open=True)
            f["special_1"] = replace(f["special_0"], near_hand=add(rsh, (28, -22)), far_hand=add(rsh, (24, -28)),
                                     near_open=False, far_open=False)
            low = (0.0, hip * 0.7)
            f["hoist_0"] = Pose(pelvis=low, lean=20, near_foot=(s.front, b.ankle), far_foot=(s.back, b.ankle),
                                near_hand=(30, 70), far_hand=(26, 64), partner=(30, 0))
            up = (0.0, hip + 2)
            f["hoist_1"] = Pose(pelvis=up, lean=-6, near_foot=(10, b.ankle), far_foot=(-12, b.ankle),
                                near_hand=(10, 170), far_hand=(2, 168), near_elbow=1, far_elbow=1,
                                partner=(4, 126), expression="shout")
            slam = (6.0, hip * 0.62)
            f["hoist_2"] = Pose(pelvis=slam, lean=44, near_foot=(s.front + 6, b.ankle), far_foot=(s.back, b.ankle),
                                far_toe=20, near_hand=(48, 30), far_hand=(42, 26), partner=(44, 6))
        return f
