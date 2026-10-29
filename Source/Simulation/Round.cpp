#include "Simulation/Round.h"

#include <algorithm>
#include <cstdlib>

namespace iron::sim {
namespace {

constexpr int kBufferTicks = 6;
constexpr int kBreakRange = 120;
constexpr int kBreakPush = 55;
constexpr int kPushHeight = 40;

int attackSlotFrom(Input pressed) {
    if (pressed & HeavyKick) return HeavyKickSlot;
    if (pressed & HeavyPunch) return HeavyPunchSlot;
    if (pressed & LightKick) return LightKickSlot;
    if (pressed & LightPunch) return LightPunchSlot;
    return -1;
}

void clampToStage(Fighter& f) {
    f.x = std::clamp(f.x, kStageMinX * kFixed, kStageMaxX * kFixed);
}

int addMeter(int meter, int amount) { return std::min(kMeterMax, meter + amount); }

struct Contact {
    bool hit = false;
    bool blocked = false;
    const MoveSpec* move = nullptr;
    int direction = 1;
};

} // namespace

WorldBox place(const Box& box, const Fighter& f) {
    const int x0 = f.facing > 0 ? f.x + box.x * kFixed : f.x - (box.x + box.w) * kFixed;
    const int y0 = f.y + box.y * kFixed;
    return {x0, y0, x0 + box.w * kFixed, y0 + box.h * kFixed};
}

Round::Round() : Round(kitBoxer(), kitGrappler()) {}

Round::Round(const FighterSpec& p1, const FighterSpec& p2) : specs{&p1, &p2} { reset(); }

void Round::reset() {
    fighters = {Fighter{}, Fighter{}};
    fighters[0].x = 230 * kFixed;
    fighters[1].x = 410 * kFixed;
    fighters[1].facing = -1;
    fighters[1].side = 1;
    tick = 0;
    remaining = kRoundSeconds * kTickRate;
    hitstop = 0;
    outcome = Outcome::Running;
    effectTicks = 0;
    effect = Effect::LightHit;
    effectX = 0;
    effectY = 0;
}

StepEvents Round::step(const std::array<Input, 2>& inputs) {
    StepEvents events;
    if (outcome != Outcome::Running) return events;
    ++tick;

    // Edge detection and input buffering run even during hitstop, so a
    // Breakaway can be entered while the game is frozen.
    std::array<Input, 2> pressed{};
    for (std::size_t i = 0; i < 2; ++i) {
        auto& f = fighters[i];
        pressed[i] = static_cast<Input>(inputs[i] & ~f.previous);
        f.previous = inputs[i];
        if (const int slot = attackSlotFrom(pressed[i]); slot >= 0) {
            f.buffered = slot;
            f.bufferTicks = kBufferTicks;
        }
        if ((inputs[i] & HeavyPunch) && (inputs[i] & HeavyKick) && (pressed[i] & (HeavyPunch | HeavyKick)))
            f.breakTicks = kBufferTicks;
    }
    if (effectTicks > 0) --effectTicks;
    if (hitstop > 0) {
        --hitstop;
        return events;
    }
    --remaining;

    for (std::size_t i = 0; i < 2; ++i) {
        auto& f = fighters[i];
        auto& other = fighters[1 - i];
        const FighterSpec& s = spec(f);
        const Input in = inputs[i];

        if (f.move < 0) f.facing = other.x >= f.x ? 1 : -1;
        if (f.stun > 0) --f.stun;
        if (f.stun == 0) f.blockstun = false;
        if (f.move >= 0 && ++f.age >= currentMove(f).duration()) {
            f.move = -1;
            f.age = 0;
            f.connected = false;
        }

        f.crouching = (in & Down) && f.y == 0;
        const bool holdingBack = f.facing == 1 ? (in & Left) : (in & Right);
        f.guarding = ((in & Guard) || holdingBack) && f.move < 0 && (f.stun == 0 || f.blockstun) && f.y == 0;

        const bool canBreak = f.guarding && f.meter >= kBreakCost && f.breakTicks > 0 &&
                              std::abs(other.x - f.x) < kBreakRange * kFixed;
        if (canBreak) {
            // Breakaway: spend Heat, cancel blockstun, shove the opponent away.
            f.meter -= kBreakCost;
            f.stun = 0;
            f.blockstun = false;
            f.buffered = -1;
            f.bufferTicks = 0;
            f.breakTicks = 0;
            other.x += f.facing * kBreakPush * kFixed;
            clampToStage(other);
            hitstop = 6;
            effectTicks = 12;
            effect = Effect::Breakaway;
            effectX = f.x;
            effectY = f.y + 55 * kFixed;
            events.push(effect, effectX, effectY);
        } else if (f.stun == 0 && f.move < 0) {
            if ((pressed[i] & Up) && f.y == 0) f.vy = s.jumpVelocity;
            if (!f.crouching) {
                const int direction = ((in & Right) != 0) - ((in & Left) != 0);
                f.x += direction * (direction == f.facing ? s.walkForward : s.walkBack);
            }
            if (f.bufferTicks > 0 && f.buffered >= 0) {
                f.move = f.buffered;
                f.age = 0;
                f.connected = false;
                f.buffered = -1;
                f.bufferTicks = 0;
                f.guarding = false;
            }
        }
        if (f.bufferTicks > 0 && --f.bufferTicks == 0) f.buffered = -1;
        if (f.breakTicks > 0) --f.breakTicks;
        if (f.y > 0 || f.vy > 0) {
            f.y += f.vy;
            f.vy -= s.gravity;
            if (f.y <= 0) {
                f.y = 0;
                f.vy = 0;
            }
        }
        clampToStage(f);
    }

    // Gather both contacts before resolving either: equal-frame strikes trade.
    std::array<Contact, 2> contacts{};
    for (std::size_t i = 0; i < 2; ++i) {
        auto& a = fighters[i];
        const auto& d = fighters[1 - i];
        if (a.move < 0 || a.connected) continue;
        const MoveSpec& m = currentMove(a);
        if (a.age < m.startup || a.age >= m.startup + m.active) continue;
        if (!place(m.hitbox, a).overlaps(place(hurtbox(d), d))) continue;

        a.connected = true;
        const bool blocked = d.guarding && (!m.low || d.crouching);
        contacts[i] = {true, blocked, &m, a.facing};
    }

    for (std::size_t i = 0; i < 2; ++i) {
        const auto& c = contacts[i];
        if (!c.hit) continue;
        auto& a = fighters[i];
        auto& d = fighters[1 - i];
        const MoveSpec& m = *c.move;
        if (c.blocked) {
            d.meter = addMeter(d.meter, 8);
            a.meter = addMeter(a.meter, 3);
            d.stun = m.blockstun;
            d.blockstun = true;
            hitstop = std::max(hitstop, m.hitstopOnBlock);
            effect = Effect::Guard;
        } else {
            d.hp = std::max(0, d.hp - m.damage);
            d.meter = addMeter(d.meter, 6);
            a.meter = addMeter(a.meter, 12);
            d.stun = m.hitstun;
            d.blockstun = false;
            d.move = -1;
            d.age = 0;
            d.connected = false;
            hitstop = std::max(hitstop, m.hitstopOnHit);
            effect = m.heavy ? Effect::HeavyHit : Effect::LightHit;
        }
        d.x += c.direction * (c.blocked ? m.blockPush : m.knockback) * kFixed;
        a.x -= c.direction * 2 * kFixed;
        effectTicks = 10;
        effectX = (a.x + d.x) / 2;
        effectY = a.y + 62 * kFixed;
        events.push(effect, effectX, effectY);
    }

    // Push boxes are independent of drawing and only overlap at nearby heights.
    auto& a = fighters[0];
    auto& b = fighters[1];
    const int pushWidth = (spec(a).pushWidth + spec(b).pushWidth) / 2 * kFixed;
    if (std::abs(a.y - b.y) < kPushHeight * kFixed && std::abs(a.x - b.x) < pushWidth) {
        const int dir = b.x >= a.x ? 1 : -1;
        const int push = pushWidth - std::abs(a.x - b.x);
        a.x -= dir * (push / 2);
        b.x += dir * (push - push / 2);
    }
    clampToStage(a);
    clampToStage(b);

    if (a.hp == 0 || b.hp == 0 || remaining <= 0)
        outcome = a.hp == b.hp ? Outcome::Draw : (a.hp > b.hp ? Outcome::Player1 : Outcome::Player2);
    return events;
}

std::uint64_t Round::checksum() const {
    // FNV-1a over explicitly listed fields in a fixed byte order; never hash
    // raw object memory (padding, ABI). Field order matches the kit prototype.
    std::uint64_t h = 14695981039346656037ULL;
    const auto add = [&h](std::int32_t v) {
        const auto u = static_cast<std::uint32_t>(v);
        for (int j = 0; j < 4; ++j) {
            h ^= (u >> (8 * j)) & 255U;
            h *= 1099511628211ULL;
        }
    };
    const auto addEnum = [&add](auto e) { add(static_cast<std::int32_t>(e)); };

    add(tick);
    add(remaining);
    add(hitstop);
    addEnum(outcome);
    add(effectTicks);
    addEnum(effect);
    add(effectX);
    add(effectY);
    for (const auto& f : fighters) {
        add(f.x); add(f.y); add(f.vy); add(f.facing); add(f.hp); add(f.meter);
        add(f.move); add(f.age); add(f.stun); add(f.buffered); add(f.bufferTicks); add(f.breakTicks);
        add(f.side); add(f.previous);
        add(f.connected); add(f.crouching); add(f.guarding); add(f.blockstun);
    }
    return h;
}

Input cpuInput(const Round& r) {
    const auto& self = r.fighters[1];
    const auto& other = r.fighters[0];
    const int distance = std::abs(self.x - other.x) / kFixed;
    Input result = 0;

    if (distance > 62) result |= other.x < self.x ? Left : Right;
    if (other.move >= 0 && distance < 90 && r.tick % 4 != 0) {
        result |= Guard;
        if (r.currentMove(other).low) result |= Down;
        if (self.meter >= kBreakCost && r.tick % 31 == 0) result |= HeavyPunch | HeavyKick;
    } else if (distance < 80 && r.tick % 36 == 0) {
        constexpr Input sequence[]{LightPunch, LightKick, HeavyPunch, HeavyKick};
        result |= sequence[(r.tick / 36) % 4];
    }
    if (r.tick % 211 == 0 && distance > 100) result |= Up;
    return result;
}

} // namespace iron::sim
