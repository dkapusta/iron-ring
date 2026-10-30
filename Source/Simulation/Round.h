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

inline constexpr int kTechWindow = 10;     // ticks after a throw connects in which LP + LK escapes
inline constexpr int kMotionStepWindow = 12; // max ticks between two directions of a motion
inline constexpr int kMotionReadyWindow = 8; // ticks to press the button after the motion
inline constexpr int kTrainingRefillTicks = 60; // training: idle ticks before health and Heat refill

enum class Outcome : std::int32_t { Running = 0, Player1 = 1, Player2 = 2, Draw = 3 };
enum class Effect : std::int32_t { LightHit = 0, HeavyHit = 1, Guard = 2, Breakaway = 3, ThrowTech = 4, Slam = 5 };

// Numpad notation relative to facing: 1 down-back ... 5 neutral ... 6 forward, 9 up-forward.
[[nodiscard]] int numpadDirection(Input input, int facing);
// Inverse, for the CPU and tests: the input bits for a numpad direction.
[[nodiscard]] Input inputForDirection(int numpad, int facing);

struct Fighter {
    int x = 0, y = 0, vy = 0, facing = 1, hp = kMaxHealth, meter = 0;
    int move = -1, age = 0, stun = 0, buffered = -1, bufferTicks = 0, breakTicks = 0;
    int side = 0; // 0 = player 1, 1 = player 2; indexes the round's specs
    Input previous = 0;
    bool connected = false, crouching = false, guarding = false, blockstun = false;

    int knockdown = 0;   // ticks left on the floor; invulnerable
    int throwTicks = 0;  // > 0 while performing a throw
    int throwSlot = -1;  // which move is being performed as a throw
    int heldTicks = 0;   // > 0 while being thrown
    int techTicks = 0;   // remaining tech window while held
    int motionStep = 0, motionTimer = 0, motionReady = 0;
    int recoverTicks = 0; // training: ticks since the fighter was last hit or held

    // Free to act, block and be thrown.
    [[nodiscard]] bool busy() const { return knockdown > 0 || throwTicks > 0 || heldTicks > 0; }
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
    // Training rules: the clock stops, nobody can be K.O.'d, the round never
    // ends, and a fighter's health and Heat refill once they have recovered
    // for kTrainingRefillTicks. A rule, not state: reset() keeps it.
    bool training = false;

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
    // Strikes connect only with fighters that are neither down nor in a throw.
    [[nodiscard]] static bool hittable(const Fighter& f) { return !f.busy(); }

private:
    void readInputs(const std::array<Input, 2>& inputs, std::array<Input, 2>& pressed, std::array<bool, 2>& techs);
    void updateFighter(std::size_t i, Input in, Input pressed, StepEvents& events);
    void resolveThrows(const std::array<bool, 2>& techs, StepEvents& events);
    void resolveContacts(StepEvents& events);
    void separate();
    void setEffect(Effect e, int x, int y, int ticks, StepEvents& events);
    void applyTrainingRules();
};

// Deterministic CPU opponent for fighters[side]; a pure function of the round state.
[[nodiscard]] Input cpuInput(const Round& round, int side = 1);

} // namespace iron::sim
