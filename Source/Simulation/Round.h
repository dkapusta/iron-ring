#pragma once

// Deterministic fixed-step simulation of one round (docs/adr/0006-deterministic-simulation.md).
//
// Rules: integer state only (positions in 1/kFixed logical pixels), exactly
// one 60 Hz tick per step(), no SDL, clock, randomness or I/O. Gameplay
// numbers come from immutable FighterSpecs owned by the caller.

#include "Simulation/Spec.h"

#include <array>
#include <cstdint>

namespace iron::sim {

enum Button : std::uint16_t {
    Left = 1, Right = 2, Up = 4, Down = 8,
    LightPunch = 16, HeavyPunch = 32, LightKick = 64, HeavyKick = 128,
    Guard = 256,
};
using Input = std::uint16_t;
inline constexpr Input kInputMask = 511;

enum class Outcome : std::int32_t { Running = 0, Player1 = 1, Player2 = 2, Draw = 3 };
enum class Effect : std::int32_t { LightHit = 0, HeavyHit = 1, Guard = 2, Breakaway = 3 };

struct Fighter {
    int x = 0, y = 0, vy = 0, facing = 1, hp = 100, meter = 0;
    int move = -1, age = 0, stun = 0, buffered = -1, bufferTicks = 0, breakTicks = 0;
    int side = 0; // 0 = player 1, 1 = player 2; indexes the round's specs
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

// Box placed in the world, in fixed-point units.
struct WorldBox {
    int x0, y0, x1, y1;
    [[nodiscard]] constexpr bool overlaps(const WorldBox& o) const {
        return x0 < o.x1 && x1 > o.x0 && y0 < o.y1 && y1 > o.y0;
    }
};
[[nodiscard]] WorldBox place(const Box& box, const Fighter& f);

// One round. Copyable: a copy is a snapshot (specs are shared, not copied).
struct Round {
    std::array<Fighter, 2> fighters{};
    std::array<const FighterSpec*, 2> specs{};
    int tick = 0, remaining = kRoundSeconds * kTickRate, hitstop = 0;
    Outcome outcome = Outcome::Running;
    int effectTicks = 0, effectX = 0, effectY = 0;
    Effect effect = Effect::LightHit;

    Round(); // kit boxer vs kit grappler
    Round(const FighterSpec& p1, const FighterSpec& p2);
    void reset();
    StepEvents step(const std::array<Input, 2>& inputs);
    [[nodiscard]] std::uint64_t checksum() const;

    [[nodiscard]] const FighterSpec& spec(const Fighter& f) const { return *specs[static_cast<std::size_t>(f.side)]; }
    [[nodiscard]] const MoveSpec& currentMove(const Fighter& f) const {
        return spec(f).moves[static_cast<std::size_t>(f.move)];
    }
    [[nodiscard]] const Box& hurtbox(const Fighter& f) const {
        return f.crouching ? spec(f).crouchHurt : spec(f).standHurt;
    }
};

[[nodiscard]] Input cpuInput(const Round& round); // drives fighters[1]

} // namespace iron::sim
