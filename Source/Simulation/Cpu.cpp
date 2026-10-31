#include "Simulation/Cpu.h"

#include <algorithm>
#include <cstdlib>

namespace iron::sim {
namespace {

//                             react think block punish combo confirm aa  throw tech break aggr
constexpr CpuProfile kProfiles[kCpuLevels]{
    /* ROOKIE    */ {24, 44, 30, 10, 0, false, 5, 8, 10, 0, 20},
    /* BRAWLER   */ {17, 30, 50, 30, 15, false, 20, 20, 30, 10, 35},
    /* CONTENDER */ {12, 22, 68, 55, 40, false, 40, 35, 50, 25, 45},
    /* VETERAN   */ {9, 16, 80, 75, 65, true, 60, 50, 70, 40, 55},
    /* CHAMPION  */ {7, 11, 90, 90, 85, true, 80, 60, 85, 55, 65},
};
constexpr std::string_view kNames[kCpuLevels]{"ROOKIE", "BRAWLER", "CONTENDER", "VETERAN", "CHAMPION"};
constexpr Input kSlotButton[]{LightPunch, HeavyPunch, LightKick, HeavyKick};

} // namespace

const CpuProfile& cpuProfile(int level) { return kProfiles[std::clamp(level, 0, kCpuLevels - 1)]; }
std::string_view cpuLevelName(int level) { return kNames[std::clamp(level, 0, kCpuLevels - 1)]; }

CpuPlayer::CpuPlayer(int level, std::uint32_t seed)
    : level_(std::clamp(level, 0, kCpuLevels - 1)), profile_(&cpuProfile(level)), rng_(seed ? seed : 0x9e3779b9u) {}

int CpuPlayer::random(int below) {
    // xorshift32: small, fast, and the same on every platform.
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;
    return below <= 0 ? 0 : static_cast<int>(rng_ % static_cast<std::uint32_t>(below));
}

bool CpuPlayer::chance(int percent) { return random(100) < percent; }

void CpuPlayer::planPress(Input input, int ticks) {
    planLength_ = planAt_ = 0;
    for (int i = 0; i < ticks && planLength_ < static_cast<int>(plan_.size()); ++i) plan_[static_cast<std::size_t>(planLength_++)] = input;
}

void CpuPlayer::planSpecial(const Round& r, const Fighter& self, Input button) {
    const MoveSpec& special = r.spec(self).moves[SpecialSlot];
    // The callers ask for a strength (light or heavy); the special decides the limb (236P, 236K).
    if (!(special.motionButtons & button)) {
        const Input kick = button == HeavyPunch ? HeavyKick : LightKick;
        const Input punch = button == HeavyKick ? HeavyPunch : LightPunch;
        if (special.motionButtons & kick) button = kick;
        else if (special.motionButtons & punch) button = punch;
        else {
            const int allowed = special.motionButtons;
            button = static_cast<Input>(allowed & -allowed); // the lowest allowed button
        }
    }
    planLength_ = planAt_ = 0;
    for (int i = 0; i < special.motionLength; ++i)
        plan_[static_cast<std::size_t>(planLength_++)] = inputForDirection(special.motion[static_cast<std::size_t>(i)], self.facing);
    const Input last = planLength_ > 0 ? plan_[static_cast<std::size_t>(planLength_ - 1)] : Input(0);
    plan_[static_cast<std::size_t>(planLength_++)] = static_cast<Input>(last | button);
}

int CpuPlayer::reachOf(const Round& r, const Fighter& self, int slot) const {
    const Box& h = r.spec(self).moves[static_cast<std::size_t>(slot)].hitbox;
    return h.x + h.w;
}

int CpuPlayer::bestStrikeFor(const Round& r, const Fighter& self, int distance, bool fastest) const {
    const auto& s = r.spec(self);
    const auto& other = r.fighters[static_cast<std::size_t>(1 - self.side)];
    const int half = r.spec(other).standHurt.w / 2;
    int best = -1;
    for (int slot = 0; slot < 4; ++slot) {
        const MoveSpec& m = s.moves[static_cast<std::size_t>(slot)];
        if (!m.enabled || reachOf(r, self, slot) + half < distance) continue;
        if (best < 0) {
            best = slot;
            continue;
        }
        const MoveSpec& b = s.moves[static_cast<std::size_t>(best)];
        if (fastest ? m.startup < b.startup : m.damage > b.damage) best = slot;
    }
    return best;
}

Input CpuPlayer::think(const Round& r, int side) {
    const auto& self = r.fighters[static_cast<std::size_t>(side)];
    const auto& other = r.fighters[static_cast<std::size_t>(1 - side)];
    const auto& s = r.spec(self);
    const CpuProfile& p = *profile_;
    const int distance = std::abs(self.x - other.x) / kFixed;
    const Input toward = other.x < self.x ? Left : Right;
    const Input away = toward == Left ? Right : Left;

    // Track the opponent's attack: a new one starts the reaction clock.
    if (other.move >= 0) {
        if (other.move != lastOpponentMove_ || other.age < lastOpponentAge_) {
            seenAttackTicks_ = 0;
            reactedThisAttack_ = false;
            guardThisAttack_ = chance(p.block);
        }
        ++seenAttackTicks_;
    } else {
        seenAttackTicks_ = 0;
    }
    lastOpponentMove_ = other.move;
    lastOpponentAge_ = other.age;

    // Held: decide once whether to tech, then mash the chord.
    if (self.heldTicks > 0) {
        if (!techDecided_) {
            techDecided_ = true;
            techPlanned_ = chance(p.tech);
        }
        planLength_ = 0;
        return techPlanned_ && r.tick % 2 == 0 ? Input(LightPunch | LightKick) : Input(0);
    }
    techDecided_ = false;

    // Hit: whatever it was typing is lost.
    if (self.stun > 0 && !self.blockstun) planLength_ = 0;
    if (planAt_ < planLength_) return plan_[static_cast<std::size_t>(planAt_++)];
    if (self.busy()) return 0;

    // Under pressure: a Breakaway on a fresh blockstun, now and then.
    const bool freshBlock = self.blockstun && self.stun > lastBlockstun_;
    lastBlockstun_ = self.blockstun ? self.stun : 0;
    if (freshBlock && self.meter >= kBreakCost && chance(p.breakaway)) return Guard | HeavyPunch | HeavyKick;

    // A light attack connected: cancel into the special (after confirming the hit at higher levels).
    if (self.move >= 0 && self.move != SpecialSlot && self.connected && !reactedThisAttack_) {
        const MoveSpec& m = r.currentMove(self);
        const MoveSpec& special = s.moves[SpecialSlot];
        const bool hit = other.stun > 0 && !other.blockstun;
        if (m.cancel && special.enabled && special.kind == MoveKind::Strike && (hit || !p.hitConfirm)) {
            reactedThisAttack_ = true; // one roll per connection
            if (chance(p.combo)) {
                planSpecial(r, self, LightPunch);
                return plan_[static_cast<std::size_t>(planAt_++)];
            }
        }
    }
    if (self.move >= 0 || (self.stun > 0 && !self.blockstun)) return 0;

    // Defence: guard an attack it has had time to see coming.
    if (other.move >= 0 && distance < reachOf(r, other, std::max(0, other.move)) + 40) {
        const MoveSpec& m = r.currentMove(other);
        const bool recovering = other.age >= m.startup + m.active;
        if (!recovering && guardThisAttack_ && seenAttackTicks_ >= p.reaction)
            return static_cast<Input>(Guard | away | (m.low ? Down : 0));
        // Punish: the attack is over and it still has recovery left.
        if (recovering && !reactedThisAttack_) {
            reactedThisAttack_ = true;
            const int left = m.duration() - other.age;
            const int slot = bestStrikeFor(r, self, distance, false);
            if (slot >= 0 && s.moves[static_cast<std::size_t>(slot)].startup < left && chance(p.punish))
                return kSlotButton[slot];
        }
    }
    if (self.blockstun) return static_cast<Input>(Guard | away);

    // Staggered by a Breakaway, or reeling: free damage.
    if (other.stun > 0 && !other.blockstun && other.move < 0 && !reactedThisAttack_) {
        reactedThisAttack_ = true;
        const int slot = bestStrikeFor(r, self, distance, true);
        if (slot >= 0 && chance(p.punish)) return kSlotButton[slot];
    }
    if (other.stun == 0 && other.move < 0) reactedThisAttack_ = false;

    // Anti-air: meet a jump-in with the special (a rising strike) or a heavy punch.
    if (other.y > 0 && distance < 120) {
        if (!reactedThisAttack_) {
            reactedThisAttack_ = true;
            guardThisAttack_ = chance(p.antiAir);
        }
        if (guardThisAttack_ && other.vy < 0 && other.y < 80 * kFixed) {
            guardThisAttack_ = false;
            const MoveSpec& special = s.moves[SpecialSlot];
            if (special.enabled && special.kind == MoveKind::Strike && special.motionLength > 0) {
                planSpecial(r, self, HeavyPunch);
                return plan_[static_cast<std::size_t>(planAt_++)];
            }
            return HeavyPunch;
        }
        return static_cast<Input>(Guard | away);
    }

    const MoveSpec& grab = s.moves[ThrowSlot];
    const MoveSpec& special = s.moves[SpecialSlot];
    // A grappler wants to stand in throw range; a striker at the tip of a light kick.
    const bool grappler = special.enabled && special.kind == MoveKind::Throw;
    const int preferred = grappler ? reachOf(r, self, ThrowSlot) + 10 : reachOf(r, self, LightKickSlot) + 6;

    // Its attack was just blocked: press in (a throw or another string is next).
    if (other.blockstun && other.stun > 0 && self.move < 0 && waitTicks_ > 4 && chance(p.aggression)) {
        waitTicks_ = 4;
        stance_ = toward;
    }

    if (--waitTicks_ > 0) {
        // Between decisions: keep walking, but stop at the chosen range.
        const bool walking = (stance_ & (Left | Right)) != 0;
        if (walking && (stance_ & toward) && distance < preferred - (grappler ? 14 : 0)) stance_ = 0;
        return stance_;
    }
    waitTicks_ = p.think + random(p.think / 2 + 1);
    stance_ = 0;

    // Close: throw a blocking or idle opponent, or jab.
    const bool throwable = other.y == 0 && !other.busy() && other.stun == 0;
    if (throwable && grab.enabled && distance < reachOf(r, self, ThrowSlot) + 18 && chance(p.throwing)) {
        // A grappler prefers the command throw: it can't be teched.
        if (grappler && chance(60)) {
            planSpecial(r, self, LightPunch);
            return plan_[static_cast<std::size_t>(planAt_++)];
        }
        return LightPunch | LightKick;
    }
    // A special that out-ranges every normal (a lunge, a long heel kick) is a poke too.
    if (special.enabled && special.kind == MoveKind::Strike && special.motionLength > 0) {
        const int lunge = special.advance * (special.startup + special.active) / kFixed;
        const int reach = reachOf(r, self, SpecialSlot) + lunge;
        int normals = 0;
        for (int slot = 0; slot < 4; ++slot) normals = std::max(normals, reachOf(r, self, slot));
        const int half = r.spec(other).standHurt.w / 2;
        if (reach > normals && distance <= reach + half && distance > normals && chance(p.aggression / 3)) {
            planSpecial(r, self, HeavyPunch);
            return plan_[static_cast<std::size_t>(planAt_++)];
        }
    }
    const int poke = bestStrikeFor(r, self, distance, true);
    // A grappler mostly walks in under the opponent's pokes instead of trading them.
    const bool wantsPoke = !grappler || distance <= preferred + 8 || chance(25);
    if (poke >= 0 && wantsPoke && chance(p.aggression)) {
        // In range: a poke, sometimes the hardest-hitting move that reaches.
        const int slot = chance(35) ? bestStrikeFor(r, self, distance, false) : poke;
        return kSlotButton[slot];
    }
    if (distance > preferred + 12 || (distance > preferred - 10 && chance(p.aggression / 2))) {
        if (distance > 150 && chance(p.aggression / 5)) return static_cast<Input>(Up | toward); // jump in
        stance_ = toward;
    } else if (distance < preferred - 8 && chance(15)) {
        stance_ = away; // a step out, to draw a whiff
    } else if (chance(p.block / 2)) {
        stance_ = static_cast<Input>(away | (chance(40) ? Down : 0)); // wait on guard
    }
    return stance_;
}

} // namespace iron::sim
