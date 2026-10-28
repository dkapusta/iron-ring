#pragma once

// Deterministic fixed-step combat simulation (docs/adr/0006-deterministic-simulation.md).
//
// Ported from the Brass Current concept kit prototype. Rules:
//   * integer state only; positions are 1/256 logical pixels;
//   * exactly one 60 Hz tick per step() call;
//   * no SDL, no clock, no randomness, no I/O.

#include <array>
#include <cstdint>

namespace iron::sim {

inline constexpr int kFixed = 256;        // sub-pixel units per logical pixel
inline constexpr int kStageWidth = 640;   // logical pixels
inline constexpr int kFloorY = 316;       // foot baseline, logical pixels from top
inline constexpr int kStageMinX = 30;
inline constexpr int kStageMaxX = 610;
inline constexpr int kTickRate = 60;
inline constexpr int kRoundSeconds = 99;
inline constexpr int kBreakCost = 50;     // Current Break meter cost
inline constexpr int kMeterMax = 100;

enum Button : std::uint16_t {
    Left = 1, Right = 2, Up = 4, Down = 8,
    LightPunch = 16, HeavyPunch = 32, LightKick = 64, HeavyKick = 128,
    Guard = 256,
};
using Input = std::uint16_t;
inline constexpr Input kInputMask = 511;

enum class Archetype : std::int32_t { Boxer = 0, Grappler = 1 };
enum class Outcome : std::int32_t { Running = 0, Player1 = 1, Player2 = 2, Draw = 3 };
enum class Effect : std::int32_t { LightHit = 0, HeavyHit = 1, Guard = 2, CurrentBreak = 3 };

struct Move {
    int startup, active, recovery, reach, damage, stun, knockback;
    bool low;
    [[nodiscard]] constexpr int duration() const { return startup + active + recovery; }
};

// Indexed by attack slot: 0 light punch, 1 heavy punch, 2 light kick, 3 sweep.
inline constexpr std::array<Move, 4> kBaseMoves{{
    {4, 3, 10, 50, 6, 14, 5, false},
    {8, 3, 20, 62, 12, 23, 10, false},
    {7, 4, 15, 72, 9, 18, 8, false},
    {12, 5, 23, 80, 14, 26, 14, true},
}};

struct Fighter {
    int x = 0, y = 0, vy = 0, facing = 1, hp = 100, meter = 0;
    int move = -1, age = 0, stun = 0, buffered = -1, bufferTicks = 0, breakTicks = 0;
    Archetype archetype = Archetype::Boxer;
    Input previous = 0;
    bool connected = false, crouching = false, guarding = false, blockstun = false;
};

// Presentation events emitted by one step. Transient: never part of the
// snapshot or checksum, so rollback can re-simulate without duplicating them.
struct StepEvents {
    struct Event {
        Effect effect;
        int x, y; // fixed-point world position
    };
    std::array<Event, 4> items{};
    int count = 0;

    void push(Effect effect, int x, int y) {
        if (count < static_cast<int>(items.size())) items[static_cast<std::size_t>(count++)] = {effect, x, y};
    }
};

struct Match {
    std::array<Fighter, 2> fighters{};
    int tick = 0, remaining = kRoundSeconds * kTickRate, hitstop = 0;
    Outcome outcome = Outcome::Running;
    int effectTicks = 0, effectX = 0, effectY = 0;
    Effect effect = Effect::LightHit;

    Match();
    void reset();
    StepEvents step(const std::array<Input, 2>& inputs);
    [[nodiscard]] std::uint64_t checksum() const;
};

[[nodiscard]] Move moveFor(const Fighter& fighter, int slot);
[[nodiscard]] Input cpuInput(const Match& match); // drives fighters[1]

} // namespace iron::sim
