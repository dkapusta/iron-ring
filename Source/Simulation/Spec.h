#pragma once

// Immutable gameplay data consumed by the simulation (docs/adr/0010-fighter-content-format.md).
// Units: lengths in logical pixels, speeds in 1/kFixed pixels per tick, times in ticks.
// Boxes are relative to the fighter's origin (centre of the feet), x pointing
// forward (the direction the fighter faces), y pointing up.

#include <array>
#include <cstdint>

namespace iron::sim {

inline constexpr int kFixed = 256;        // sub-pixel units per logical pixel
inline constexpr int kStageWidth = 640;   // logical pixels
inline constexpr int kFloorY = 316;       // foot baseline, logical pixels from the top of the screen
inline constexpr int kStageMinX = 30;
inline constexpr int kStageMaxX = 610;
inline constexpr int kTickRate = 60;
inline constexpr int kRoundSeconds = 99;
inline constexpr int kBreakCost = 50;     // Breakaway cost, in Heat meter units
inline constexpr int kMeterMax = 100;

struct Box {
    int x = 0, y = 0, w = 0, h = 0;
    [[nodiscard]] constexpr bool empty() const { return w <= 0 || h <= 0; }
};

// Attack slots. Index into FighterSpec::moves and Fighter::move.
enum Slot : int { LightPunchSlot = 0, HeavyPunchSlot, LightKickSlot, HeavyKickSlot, kAttackSlots };

struct MoveSpec {
    int startup = 1, active = 1, recovery = 1;
    Box hitbox;
    int damage = 0;
    int hitstun = 0, blockstun = 7;
    int knockback = 0, blockPush = 3;     // pixels the defender slides
    int hitstopOnHit = 5, hitstopOnBlock = 3;
    bool low = false;                     // must be blocked crouching
    bool heavy = false;                   // presentation: heavy hit effect

    [[nodiscard]] constexpr int duration() const { return startup + active + recovery; }
};

struct FighterSpec {
    std::array<MoveSpec, kAttackSlots> moves{};
    int walkForward = 420, walkBack = 420;  // 1/kFixed px per tick
    int jumpVelocity = 1480, gravity = 70;  // 1/kFixed px per tick (squared)
    Box standHurt{-18, 0, 36, 100};
    Box crouchHurt{-18, 0, 36, 64};
    int pushWidth = 36;
};

// The concept kit prototype's two archetypes, kept bit-exact for regression
// tests (the golden checksum) and as a fallback when no content is loaded.
[[nodiscard]] const FighterSpec& kitBoxer();
[[nodiscard]] const FighterSpec& kitGrappler();

} // namespace iron::sim
