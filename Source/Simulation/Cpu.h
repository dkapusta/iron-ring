#pragma once

// The CPU opponent: an input source like a player, with five levels.
//
// Deterministic: its only randomness is its own xorshift generator, seeded
// at construction, and it reads nothing but the round. A recorded match
// therefore replays exactly. It is not part of the round's state or
// checksum; like a player, it only produces inputs.
//
// What the levels change (CpuProfile): how late it reacts to an attack, how
// often it blocks, punishes, anti-airs, throws, techs, breaks away, and
// whether it hit-confirms its cancels into the special.

#include "Simulation/Round.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace iron::sim {

inline constexpr int kCpuLevels = 5;

struct CpuProfile {
    int reaction;      // ticks before it can react to an attack it sees start
    int think;         // ticks between neutral decisions
    int block;         // % chance to guard an attack it has reacted to
    int punish;        // % chance to punish a whiffed or blocked move
    int combo;         // % chance to cancel a connecting light attack into the special
    bool hitConfirm;   // cancel only when the light attack hit (not when blocked)
    int antiAir;       // % chance to meet a jump-in
    int throwing;      // % chance to throw a blocking opponent at close range
    int tech;          // % chance to break a normal throw
    int breakaway;     // % chance per blocked hit to spend 50 Heat on a Breakaway
    int aggression;    // % chance to step in and attack on a neutral decision
};

[[nodiscard]] const CpuProfile& cpuProfile(int level); // 0..kCpuLevels-1
[[nodiscard]] std::string_view cpuLevelName(int level); // ROOKIE .. CHAMPION

class CpuPlayer {
public:
    CpuPlayer(int level, std::uint32_t seed);

    // The CPU's input for this tick as `side`. Call once per tick.
    Input think(const Round& round, int side);
    [[nodiscard]] int level() const { return level_; }

private:
    [[nodiscard]] bool chance(int percent);
    [[nodiscard]] int random(int below);
    void planSpecial(const Round& r, const Fighter& self, Input button);
    void planPress(Input input, int ticks);
    [[nodiscard]] int reachOf(const Round& r, const Fighter& self, int slot) const; // px in front
    [[nodiscard]] int bestStrikeFor(const Round& r, const Fighter& self, int distance, bool fastest) const;

    int level_;
    const CpuProfile* profile_;
    std::uint32_t rng_;
    std::array<Input, 12> plan_{}; // queued inputs: motions, then a button
    int planLength_ = 0, planAt_ = 0;
    int waitTicks_ = 0;            // until the next neutral decision
    Input stance_ = 0;             // what it holds between decisions (walk, crouch, guard)
    int seenAttackTicks_ = 0;      // how long the opponent's current attack has been visible
    int lastOpponentMove_ = -1, lastOpponentAge_ = 0;
    bool reactedThisAttack_ = false, guardThisAttack_ = false;
    bool techDecided_ = false, techPlanned_ = false;
    int lastBlockstun_ = 0;
};

} // namespace iron::sim
