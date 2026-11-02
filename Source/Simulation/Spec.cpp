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

// The kit had no throws or specials.
MoveSpec disabled() {
    MoveSpec m;
    m.enabled = false;
    return m;
}

FighterSpec makeBoxer() {
    FighterSpec s;
    s.moves = {
        kitMove(4, 3, 10, 50, 6, 14, 5, false),
        kitMove(8, 3, 20, 62, 12, 23, 10, false),
        kitMove(7, 4, 15, 72, 9, 18, 8, false),
        kitMove(12, 5, 23, 80, 14, 26, 14, true),
        disabled(),
        disabled(),
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
        disabled(),
        disabled(),
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

std::uint64_t specHash(const FighterSpec& spec) {
    std::uint64_t h = 14695981039346656037ULL;
    const auto add = [&h](std::int64_t v) {
        const auto u = static_cast<std::uint64_t>(v);
        for (int j = 0; j < 8; ++j) {
            h ^= (u >> (8 * j)) & 255U;
            h *= 1099511628211ULL;
        }
    };
    add(kRulesVersion);
    const auto box = [&add](const Box& b) {
        add(b.x); add(b.y); add(b.w); add(b.h);
    };
    for (const auto& m : spec.moves) {
        add(m.enabled); add(static_cast<std::int64_t>(m.kind));
        add(m.startup); add(m.active); add(m.recovery);
        box(m.hitbox);
        add(m.damage); add(m.hitstun); add(m.blockstun); add(m.knockback); add(m.blockPush);
        add(m.hitstopOnHit); add(m.hitstopOnBlock);
        add(m.low); add(m.heavy); add(m.knockdown); add(m.cancel); add(m.advance);
        for (const auto& phase : m.hurt)
            for (const auto& b : phase) box(b);
        add(m.throwDuration); add(m.throwDistance); add(m.techable);
        for (const auto d : m.motion) add(d);
        add(m.motionLength); add(m.motionButtons);
    }
    add(spec.walkForward); add(spec.walkBack); add(spec.jumpVelocity); add(spec.gravity);
    box(spec.standHurt);
    box(spec.crouchHurt);
    add(spec.pushWidth); add(spec.knockdownTicks);
    return h;
}

} // namespace iron::sim

