#include "Simulation/Round.h"

#include <algorithm>
#include <cstdlib>

namespace iron::sim {
namespace {

constexpr int kBufferTicks = 6;
constexpr int kBreakRange = 120;
constexpr int kBreakPush = 55;
constexpr int kPushHeight = 40;
constexpr int kHoldOffset = 30;   // victim's distance from the thrower while held
constexpr int kTechSeparation = 24;

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

bool chordPressed(Input held, Input pressed, Input a, Input b) {
    return (held & a) && (held & b) && (pressed & (a | b));
}

// Advances the special-move motion recogniser by one tick of input.
void updateMotion(Fighter& f, const MoveSpec& special, Input in) {
    if (!special.enabled || special.motionLength <= 0) return;
    const int dir = numpadDirection(in, f.facing);
    if (dir == special.motion[static_cast<std::size_t>(f.motionStep)]) {
        f.motionTimer = 0;
        if (++f.motionStep == special.motionLength) {
            f.motionStep = 0;
            f.motionReady = kMotionReadyWindow;
        }
    } else if (++f.motionTimer > kMotionStepWindow) {
        f.motionStep = dir == special.motion[0] ? 1 : 0;
        f.motionTimer = 0;
    }
}

void stopActing(Fighter& f) {
    f.move = -1;
    f.age = 0;
    f.connected = false;
    f.buffered = -1;
    f.bufferTicks = 0;
    f.guarding = false;
    f.crouching = false;
    f.stun = 0;
    f.blockstun = false;
}

struct Contact {
    bool hit = false;
    bool blocked = false;
    const MoveSpec* move = nullptr;
    int direction = 1;
};

} // namespace

int numpadDirection(Input in, int facing) {
    const int horizontal = ((in & Right) != 0) - ((in & Left) != 0);
    const int forward = horizontal * facing;
    const int column = forward > 0 ? 2 : (forward < 0 ? 0 : 1);
    const int row = (in & Down) ? 0 : ((in & Up) ? 2 : 1);
    return row * 3 + column + 1;
}

Input inputForDirection(int numpad, int facing) {
    const int row = (numpad - 1) / 3;
    const int horizontal = ((numpad - 1) % 3 - 1) * facing;
    Input in = 0;
    if (horizontal > 0) in |= Right;
    if (horizontal < 0) in |= Left;
    if (row == 0) in |= Down;
    if (row == 2) in |= Up;
    return in;
}

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

void Round::setEffect(Effect e, int x, int y, int ticks, StepEvents& events) {
    effect = e;
    effectX = x;
    effectY = y;
    effectTicks = ticks;
    events.push(e, x, y);
}

void Round::applyTrainingRules() {
    for (auto& f : fighters) {
        if (f.hp <= 0) f.hp = 1; // no K.O. in training
        const bool recovering = f.stun > 0 || f.knockdown > 0 || f.heldTicks > 0;
        f.recoverTicks = recovering ? 0 : f.recoverTicks + 1;
        if (f.recoverTicks >= kTrainingRefillTicks) {
            f.hp = kMaxHealth;
            f.meter = kMeterMax;
        }
    }
}

StepEvents Round::step(const std::array<Input, 2>& inputs) {
    StepEvents events;
    if (outcome != Outcome::Running) return events;
    ++tick;

    std::array<Input, 2> pressed{};
    std::array<bool, 2> techs{};
    readInputs(inputs, pressed, techs);
    if (effectTicks > 0) --effectTicks;
    if (hitstop > 0) {
        --hitstop;
        return events;
    }
    if (!training) --remaining;

    for (std::size_t i = 0; i < 2; ++i) updateFighter(i, inputs[i], pressed[i], events);
    resolveThrows(techs, events);
    resolveContacts(events);
    separate();
    if (training) {
        applyTrainingRules();
        return events;
    }

    const auto& a = fighters[0];
    const auto& b = fighters[1];
    if (a.hp == 0 || b.hp == 0 || remaining <= 0)
        outcome = a.hp == b.hp ? Outcome::Draw : (a.hp > b.hp ? Outcome::Player1 : Outcome::Player2);
    return events;
}

// Edge detection, motion recognition and input buffering. Runs even during
// hitstop, so attacks and a Breakaway can be entered while the game is frozen.
void Round::readInputs(const std::array<Input, 2>& inputs, std::array<Input, 2>& pressed, std::array<bool, 2>& techs) {
    for (std::size_t i = 0; i < 2; ++i) {
        auto& f = fighters[i];
        const FighterSpec& s = spec(f);
        const Input in = inputs[i];
        pressed[i] = static_cast<Input>(in & ~f.previous);
        f.previous = in;

        const MoveSpec& special = s.moves[SpecialSlot];
        updateMotion(f, special, in);
        const bool throwChord = chordPressed(in, pressed[i], LightPunch, LightKick);
        if (f.heldTicks > 0) {
            techs[i] = throwChord && f.techTicks > 0;
            continue;
        }

        int slot = -1;
        if (special.enabled && f.motionReady > 0 && (pressed[i] & special.motionButtons)) {
            slot = SpecialSlot;
            f.motionReady = 0;
        } else if (s.moves[ThrowSlot].enabled && throwChord) {
            slot = ThrowSlot;
        } else {
            slot = attackSlotFrom(pressed[i]);
        }
        if (slot >= 0) {
            f.buffered = slot;
            f.bufferTicks = kBufferTicks;
        }
        if (f.motionReady > 0 && slot != SpecialSlot) --f.motionReady;
        if (chordPressed(in, pressed[i], HeavyPunch, HeavyKick)) f.breakTicks = kBufferTicks;
    }
}

void Round::updateFighter(std::size_t i, Input in, Input pressed, StepEvents& events) {
    auto& f = fighters[i];
    auto& other = fighters[1 - i];
    const FighterSpec& s = spec(f);

    const auto applyGravity = [&] {
        if (f.y > 0 || f.vy > 0) {
            f.y += f.vy;
            f.vy -= s.gravity;
            if (f.y <= 0) {
                f.y = 0;
                f.vy = 0;
            }
        }
        clampToStage(f);
    };

    if (f.throwTicks > 0 || f.heldTicks > 0) return; // resolveThrows drives both fighters
    if (f.knockdown > 0) {
        stopActing(f);
        if (f.y == 0) --f.knockdown; // the clock starts once the fighter lands
        applyGravity();
        return;
    }

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
        if (!other.busy()) {
            other.x += f.facing * kBreakPush * kFixed;
            clampToStage(other);
        }
        hitstop = 6;
        setEffect(Effect::Breakaway, f.x, f.y + 55 * kFixed, 12, events);
    } else if (f.stun == 0 && f.move < 0) {
        if ((pressed & Up) && f.y == 0) f.vy = s.jumpVelocity;
        if (!f.crouching) {
            const int direction = ((in & Right) != 0) - ((in & Left) != 0);
            f.x += direction * (direction == f.facing ? s.walkForward : s.walkBack);
        }
        // Throws and command throws only start on the ground.
        const bool groundOnly = f.buffered >= 0 && s.moves[static_cast<std::size_t>(f.buffered)].kind == MoveKind::Throw;
        if (f.bufferTicks > 0 && f.buffered >= 0 && (!groundOnly || f.y == 0)) {
            f.move = f.buffered;
            f.age = 0;
            f.connected = false;
            f.buffered = -1;
            f.bufferTicks = 0;
            f.guarding = false;
        }
    }
    if (f.move >= 0) {
        const MoveSpec& m = currentMove(f);
        if (m.advance != 0 && f.age < m.startup + m.active) f.x += f.facing * m.advance;
    }
    if (f.bufferTicks > 0 && --f.bufferTicks == 0) f.buffered = -1;
    if (f.breakTicks > 0) --f.breakTicks;
    applyGravity();
}

void Round::resolveThrows(const std::array<bool, 2>& techs, StepEvents& events) {
    for (std::size_t i = 0; i < 2; ++i) {
        auto& a = fighters[i];
        auto& d = fighters[1 - i];
        if (a.throwTicks <= 0) continue;
        const MoveSpec& m = spec(a).moves[static_cast<std::size_t>(a.throwSlot)];

        if (techs[1 - i]) {
            a.throwTicks = 0;
            a.throwSlot = -1;
            d.heldTicks = 0;
            d.techTicks = 0;
            a.x -= a.facing * kTechSeparation * kFixed;
            d.x += a.facing * kTechSeparation * kFixed;
            clampToStage(a);
            clampToStage(d);
            setEffect(Effect::ThrowTech, (a.x + d.x) / 2, 70 * kFixed, 10, events);
            continue;
        }

        if (d.techTicks > 0) --d.techTicks;
        d.x = a.x + a.facing * kHoldOffset * kFixed;
        d.facing = -a.facing;
        clampToStage(d);
        d.heldTicks = --a.throwTicks;
        if (a.throwTicks > 0) continue;

        // Throw lands: damage, knockdown, victim thrown forward.
        d.hp = std::max(0, d.hp - m.damage);
        d.techTicks = 0;
        d.knockdown = spec(d).knockdownTicks;
        d.x = a.x + a.facing * m.throwDistance * kFixed;
        clampToStage(d);
        a.meter = addMeter(a.meter, 12);
        d.meter = addMeter(d.meter, 6);
        a.throwSlot = -1;
        hitstop = std::max(hitstop, m.hitstopOnHit);
        setEffect(Effect::Slam, d.x, d.y + 12 * kFixed, 12, events);
    }
}

void Round::resolveContacts(StepEvents& events) {
    std::array<Contact, 2> contacts{};
    std::array<bool, 2> grabs{};

    // Gather everything before resolving anything: equal-frame strikes trade.
    for (std::size_t i = 0; i < 2; ++i) {
        auto& a = fighters[i];
        const auto& d = fighters[1 - i];
        if (a.busy() || a.move < 0 || a.connected) continue;
        const MoveSpec& m = currentMove(a);
        if (a.age < m.startup || a.age >= m.startup + m.active) continue;

        if (m.kind == MoveKind::Throw) {
            const bool throwable = d.y == 0 && !d.busy() && d.stun == 0;
            if (!throwable || !place(m.hitbox, a).overlaps(place(hurtbox(d), d))) continue;
            a.connected = true;
            grabs[i] = true;
            continue;
        }
        if (!hittable(d) || !place(m.hitbox, a).overlaps(place(hurtbox(d), d))) continue;
        a.connected = true;
        const bool blocked = d.guarding && (!m.low || d.crouching);
        contacts[i] = {true, blocked, &m, a.facing};
    }

    if (grabs[0] && grabs[1]) {
        // Simultaneous throws cancel out like a tech.
        for (auto& f : fighters) stopActing(f);
        fighters[0].x -= fighters[0].facing * kTechSeparation * kFixed;
        fighters[1].x -= fighters[1].facing * kTechSeparation * kFixed;
        setEffect(Effect::ThrowTech, (fighters[0].x + fighters[1].x) / 2, 70 * kFixed, 10, events);
        return;
    }
    for (std::size_t i = 0; i < 2; ++i) {
        if (!grabs[i]) continue;
        auto& a = fighters[i];
        auto& d = fighters[1 - i];
        const MoveSpec& m = currentMove(a);
        a.throwSlot = a.move;
        a.throwTicks = m.throwDuration;
        a.move = -1;
        a.age = 0;
        stopActing(d);
        d.heldTicks = m.throwDuration;
        d.techTicks = m.techable ? kTechWindow : 0;
        contacts[1 - i] = {}; // a fighter being grabbed doesn't finish its own strike
    }

    for (std::size_t i = 0; i < 2; ++i) {
        const auto& c = contacts[i];
        if (!c.hit) continue;
        auto& a = fighters[i];
        auto& d = fighters[1 - i];
        if (!hittable(d)) continue; // grabbed on this tick
        const MoveSpec& m = *c.move;
        if (c.blocked) {
            d.meter = addMeter(d.meter, 8);
            a.meter = addMeter(a.meter, 3);
            d.stun = m.blockstun;
            d.blockstun = true;
            hitstop = std::max(hitstop, m.hitstopOnBlock);
        } else {
            const bool airborne = d.y > 0;
            d.hp = std::max(0, d.hp - m.damage);
            d.meter = addMeter(d.meter, 6);
            a.meter = addMeter(a.meter, 12);
            d.move = -1;
            d.age = 0;
            d.connected = false;
            d.blockstun = false;
            if (m.knockdown || airborne || d.hp == 0) {
                stopActing(d);
                d.knockdown = spec(d).knockdownTicks;
            } else {
                d.stun = m.hitstun;
            }
            hitstop = std::max(hitstop, m.hitstopOnHit);
        }
        d.x += c.direction * (c.blocked ? m.blockPush : m.knockback) * kFixed;
        a.x -= c.direction * 2 * kFixed;
        const Effect e = c.blocked ? Effect::Guard : (m.heavy ? Effect::HeavyHit : Effect::LightHit);
        const WorldBox box = place(m.hitbox, a);
        setEffect(e, (a.x + d.x) / 2, (box.y0 + box.y1) / 2, 10, events);
    }
}

// Push boxes keep fighters from overlapping; they are independent of drawing.
void Round::separate() {
    auto& a = fighters[0];
    auto& b = fighters[1];
    if (a.throwTicks > 0 || b.throwTicks > 0) return;
    const int pushWidth = (spec(a).pushWidth + spec(b).pushWidth) / 2 * kFixed;
    if (std::abs(a.y - b.y) < kPushHeight * kFixed && std::abs(a.x - b.x) < pushWidth) {
        const int dir = b.x >= a.x ? 1 : -1;
        const int push = pushWidth - std::abs(a.x - b.x);
        a.x -= dir * (push / 2);
        b.x += dir * (push - push / 2);
    }
    clampToStage(a);
    clampToStage(b);
}

std::uint64_t Round::checksum() const {
    // FNV-1a over explicitly listed fields in a fixed byte order; never hash
    // raw object memory (padding, ABI). Every state field must appear here.
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
        add(f.knockdown); add(f.throwTicks); add(f.throwSlot); add(f.heldTicks); add(f.techTicks);
        add(f.motionStep); add(f.motionTimer); add(f.motionReady);
        if (training) add(f.recoverTicks); // only under training rules, so match checksums stay put
    }
    if (training) add(1);
    return h;
}

Input cpuInput(const Round& r, int side) {
    const auto& self = r.fighters[static_cast<std::size_t>(side)];
    const auto& other = r.fighters[static_cast<std::size_t>(1 - side)];
    // Offset the schedules per side so two CPUs don't mirror each other.
    const int tick = r.tick + (1 - side) * 37;
    const FighterSpec& s = r.spec(self);
    const int distance = std::abs(self.x - other.x) / kFixed;

    // Held: try to tech roughly half of the throws.
    if (self.heldTicks > 0) return (tick / 7) % 2 == 0 ? Input(LightPunch | LightKick) : Input(0);
    if (self.busy()) return 0;

    // Specials are typed over several ticks; the schedule is a pure function
    // of the tick counter, so the CPU stays deterministic.
    const MoveSpec& special = s.moves[SpecialSlot];
    if (special.enabled && special.motionLength > 0 && distance > 36 && distance < 90 && other.knockdown == 0) {
        const int phase = tick % 113;
        const int steps = special.motionLength * 2;
        if (phase <= steps) {
            const auto dir = special.motion[static_cast<std::size_t>(std::min(phase / 2, special.motionLength - 1))];
            Input in = inputForDirection(dir, self.facing);
            if (phase == steps) in |= LightPunch;
            return in;
        }
    }
    // Every fourth second the CPU presses in to throwing range.
    const bool pressing = (tick / kTickRate) % 4 == 3;
    const MoveSpec& grab = s.moves[ThrowSlot];
    const bool throwable = !other.busy() && other.stun == 0 && other.y == 0 && self.y == 0;
    if (grab.enabled && throwable && tick % 7 == 0 &&
        place(grab.hitbox, self).overlaps(place(r.hurtbox(other), other)))
        return LightPunch | LightKick;

    Input result = 0;
    if (distance > (pressing ? 30 : 62)) result |= other.x < self.x ? Left : Right;
    if (other.move >= 0 && distance < 90 && tick % 4 != 0) {
        result |= Guard;
        if (r.currentMove(other).low) result |= Down;
        if (self.meter >= kBreakCost && tick % 31 == 0) result |= HeavyPunch | HeavyKick;
    } else if (distance < 80 && tick % 36 == 0) {
        constexpr Input sequence[]{LightPunch, LightKick, HeavyPunch, HeavyKick};
        result |= sequence[(tick / 36) % 4];
    }
    if (tick % 211 == 0 && distance > 100) result |= Up;
    return result;
}

} // namespace iron::sim
