#include "Simulation/Spec.h"

namespace iron::sim {
namespace {

// The kit expressed attacks as a reach (forward extent) plus a high or low
// band of 24 px; hits of 12+ damage were "heavy" (bigger hitstop and spark).
MoveSpec kitMove(int startup, int active, int recovery, int reach, int damage, int hitstun, int knockback, bool low) {
    MoveSpec m;
    m.startup = startup;
    m.active = active;
    m.recovery = recovery;
    m.hitbox = {18, low ? 4 : 56, reach - 18, 24};
    m.damage = damage;
    m.hitstun = hitstun;
    m.knockback = knockback;
    m.low = low;
    m.heavy = damage >= 12;
    m.hitstopOnHit = m.heavy ? 8 : 5;
    return m;
}

FighterSpec makeBoxer() {
    FighterSpec s;
    s.moves = {
        kitMove(4, 3, 10, 50, 6, 14, 5, false),
        kitMove(8, 3, 20, 62, 12, 23, 10, false),
        kitMove(7, 4, 15, 72, 9, 18, 8, false),
        kitMove(12, 5, 23, 80, 14, 26, 14, true),
    };
    s.walkForward = s.walkBack = 420;
    return s;
}

FighterSpec makeGrappler() {
    // The kit derived these from the boxer: +4 reach, heavier and slower heavies.
    FighterSpec s;
    s.moves = {
        kitMove(4, 3, 10, 54, 6, 14, 5, false),
        kitMove(10, 3, 23, 66, 16, 23, 10, false),
        kitMove(7, 4, 15, 76, 9, 18, 8, false),
        kitMove(14, 5, 26, 84, 18, 26, 14, true),
    };
    s.walkForward = s.walkBack = 340;
    return s;
}

} // namespace

const FighterSpec& kitBoxer() {
    static const FighterSpec spec = makeBoxer();
    return spec;
}

const FighterSpec& kitGrappler() {
    static const FighterSpec spec = makeGrappler();
    return spec;
}

} // namespace iron::sim
