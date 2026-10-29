#include "Simulation/Combat.h"

#include <algorithm>
#include <cstdlib>

namespace iron::sim {
namespace {

constexpr int kBufferTicks = 6;
constexpr int kJumpVelocity = 1480;
constexpr int kGravity = 70;
constexpr int kBreakRange = 120;
constexpr int kBreakPush = 55;
constexpr int kBodyHalfWidth = 18;
constexpr int kPushBoxWidth = 36;

int attackSlotFrom(Input pressed) {
    if (pressed & HeavyKick) return 3;
    if (pressed & HeavyPunch) return 1;
    if (pressed & LightKick) return 2;
    if (pressed & LightPunch) return 0;
    return -1;
}

void clampToStage(Fighter& f) {
    f.x = std::clamp(f.x, kStageMinX * kFixed, kStageMaxX * kFixed);
}

int addMeter(int meter, int amount) { return std::min(kMeterMax, meter + amount); }

struct Contact {
    bool hit = false;
    bool blocked = false;
    Move move{};
    int direction = 1;
};

} // namespace

Match::Match() { reset(); }

void Match::reset() {
    fighters = {Fighter{}, Fighter{}};
    fighters[0].x = 230 * kFixed;
    fighters[1].x = 410 * kFixed;
    fighters[1].facing = -1;
    fighters[1].archetype = Archetype::Grappler;
    tick = 0;
    remaining = kRoundSeconds * kTickRate;
    hitstop = 0;
    outcome = Outcome::Running;
    effectTicks = 0;
    effect = Effect::LightHit;
    effectX = 0;
    effectY = 0;
}

Move moveFor(const Fighter& fighter, int slot) {
    Move m = kBaseMoves.at(static_cast<std::size_t>(slot));
    if (fighter.archetype == Archetype::Grappler) {
        m.reach += 4;
        if (slot == 1 || slot == 3) {
            m.startup += 2;
            m.recovery += 3;
            m.damage += 4;
        }
    }
    return m;
}

StepEvents Match::step(const std::array<Input, 2>& inputs) {
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
        const Input in = inputs[i];

        if (f.move < 0) f.facing = other.x >= f.x ? 1 : -1;
        if (f.stun > 0) --f.stun;
        if (f.stun == 0) f.blockstun = false;
        if (f.move >= 0 && ++f.age >= moveFor(f, f.move).duration()) {
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
            // Breakaway: spend meter, cancel blockstun, push the opponent away.
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
            if ((pressed[i] & Up) && f.y == 0) f.vy = kJumpVelocity;
            if (!f.crouching) {
                const int direction = ((in & Right) != 0) - ((in & Left) != 0);
                f.x += direction * (f.archetype == Archetype::Grappler ? 340 : 420);
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
            f.vy -= kGravity;
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
        const Move m = moveFor(a, a.move);
        if (a.age < m.startup || a.age >= m.startup + m.active) continue;

        const int edge0 = a.x + a.facing * kBodyHalfWidth * kFixed;
        const int edge1 = a.x + a.facing * m.reach * kFixed;
        const int ax0 = std::min(edge0, edge1);
        const int ax1 = std::max(edge0, edge1);
        const int ay = a.y + (m.low ? 16 : 68) * kFixed;
        const int bodyHeight = (d.crouching ? 64 : 100) * kFixed;
        const bool overlap = ax0 < d.x + kBodyHalfWidth * kFixed && ax1 > d.x - kBodyHalfWidth * kFixed &&
                             ay - 12 * kFixed < d.y + bodyHeight && ay + 12 * kFixed > d.y;
        if (!overlap) continue;

        a.connected = true;
        const bool blocked = d.guarding && (!m.low || d.crouching);
        contacts[i] = {true, blocked, m, a.facing};
    }

    for (std::size_t i = 0; i < 2; ++i) {
        const auto& c = contacts[i];
        if (!c.hit) continue;
        auto& a = fighters[i];
        auto& d = fighters[1 - i];
        if (c.blocked) {
            d.meter = addMeter(d.meter, 8);
            a.meter = addMeter(a.meter, 3);
            d.stun = 7;
            d.blockstun = true;
            hitstop = std::max(hitstop, 3);
            effect = Effect::Guard;
        } else {
            const bool heavy = c.move.damage >= 12;
            d.hp = std::max(0, d.hp - c.move.damage);
            d.meter = addMeter(d.meter, 6);
            a.meter = addMeter(a.meter, 12);
            d.stun = c.move.stun;
            d.blockstun = false;
            d.move = -1;
            d.age = 0;
            d.connected = false;
            hitstop = std::max(hitstop, heavy ? 8 : 5);
            effect = heavy ? Effect::HeavyHit : Effect::LightHit;
        }
        d.x += c.direction * (c.blocked ? 3 : c.move.knockback) * kFixed;
        a.x -= c.direction * 2 * kFixed;
        effectTicks = 10;
        effectX = (a.x + d.x) / 2;
        effectY = a.y + 62 * kFixed;
        events.push(effect, effectX, effectY);
    }

    // Push boxes are independent of drawing and only overlap at nearby heights.
    auto& a = fighters[0];
    auto& b = fighters[1];
    if (std::abs(a.y - b.y) < 40 * kFixed && std::abs(a.x - b.x) < kPushBoxWidth * kFixed) {
        const int dir = b.x >= a.x ? 1 : -1;
        const int push = kPushBoxWidth * kFixed - std::abs(a.x - b.x);
        a.x -= dir * (push / 2);
        b.x += dir * (push - push / 2);
    }
    clampToStage(a);
    clampToStage(b);

    if (a.hp == 0 || b.hp == 0 || remaining <= 0)
        outcome = a.hp == b.hp ? Outcome::Draw : (a.hp > b.hp ? Outcome::Player1 : Outcome::Player2);
    return events;
}

std::uint64_t Match::checksum() const {
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
        addEnum(f.archetype); add(f.previous);
        add(f.connected); add(f.crouching); add(f.guarding); add(f.blockstun);
    }
    return h;
}

Input cpuInput(const Match& m) {
    const auto& self = m.fighters[1];
    const auto& other = m.fighters[0];
    const int distance = std::abs(self.x - other.x) / kFixed;
    Input result = 0;

    if (distance > 62) result |= other.x < self.x ? Left : Right;
    if (other.move >= 0 && distance < 90 && m.tick % 4 != 0) {
        result |= Guard;
        if (other.move == 3) result |= Down;
        if (self.meter >= kBreakCost && m.tick % 31 == 0) result |= HeavyPunch | HeavyKick;
    } else if (distance < 80 && m.tick % 36 == 0) {
        constexpr Input sequence[]{LightPunch, LightKick, HeavyPunch, HeavyKick};
        result |= sequence[(m.tick / 36) % 4];
    }
    if (m.tick % 211 == 0 && distance > 100) result |= Up;
    return result;
}

} // namespace iron::sim
