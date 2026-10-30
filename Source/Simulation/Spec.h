#pragma once

// Immutable gameplay data consumed by the simulation (docs/adr/0010-fighter-content-format.md).
// Units: lengths in logical pixels, speeds in 1/kFixed pixels per tick, times in ticks.
// Boxes are relative to the fighter's origin (centre of the feet), x pointing
// forward (the direction the fighter faces), y pointing up.

#include <array>
#include <cstdint>

namespace iron::sim {

inline constexpr int kFixed = 256;        // sub-pixel units per logical pixel
inline constexpr int kStageWidth = 640;   // logical pixels: the screen, and the default stage width
inline constexpr int kFloorY = 316;       // foot baseline, logical pixels from the top of the screen
inline constexpr int kStageMargin = 30;   // fighters stay this far from a stage's ends
inline constexpr int kStageMinX = kStageMargin;
inline constexpr int kStageMaxX = kStageWidth - kStageMargin;
inline constexpr int kMaxSeparation = kStageWidth - 2 * kStageMargin; // both fighters always fit on screen
inline constexpr int kStartOffset = 90;   // round start: this far either side of the stage centre
inline constexpr int kTickRate = 60;
inline constexpr int kRoundSeconds = 99;
inline constexpr int kBreakCost = 50;     // Breakaway cost, in Heat meter units
inline constexpr int kMeterMax = 100;
inline constexpr int kMaxHealth = 100;

struct Box {
    int x = 0, y = 0, w = 0, h = 0;
    [[nodiscard]] constexpr bool empty() const { return w <= 0 || h <= 0; }
};

// Move slots. Index into FighterSpec::moves and Fighter::move.
enum Slot : int {
    LightPunchSlot = 0, HeavyPunchSlot, LightKickSlot, HeavyKickSlot,
    ThrowSlot,   // LP + LK together, close to a grounded, free opponent
    SpecialSlot, // motion input + button (see MoveSpec::motion)
    kSlotCount,
};

enum class MoveKind : std::int32_t { Strike = 0, Throw = 1 };

// Phases of a move, by tick: startup, active (the hitbox is out), recovery.
enum class MovePhase : std::int32_t { Startup = 0, Active = 1, Recovery = 2 };
inline constexpr int kPhaseCount = 3;
inline constexpr int kMaxHurtBoxes = 2;
// Hurtboxes for one phase: up to kMaxHurtBoxes; an all-empty set means
// "the fighter's default stand/crouch hurtbox".
using HurtSet = std::array<Box, kMaxHurtBoxes>;

inline constexpr int kMaxMotionLength = 6;

struct MoveSpec {
    bool enabled = true;
    MoveKind kind = MoveKind::Strike;
    int startup = 1, active = 1, recovery = 1;
    Box hitbox;                           // strike box, or grab box for throws
    int damage = 0;
    int hitstun = 0, blockstun = 7;
    int knockback = 0, blockPush = 3;     // pixels the defender slides
    int hitstopOnHit = 5, hitstopOnBlock = 3;
    bool low = false;                     // must be blocked crouching
    bool heavy = false;                   // presentation: heavy hit effect
    bool knockdown = false;               // a clean hit knocks the opponent down
    bool cancel = false;                  // on hit or block, the special can cancel its recovery
    int advance = 0;                      // 1/kFixed px per tick forward during startup + active
    // Hurtboxes per phase, for moves that change the silhouette: a sweep
    // ducks low, an extended limb can be hit (whiff punishes).
    std::array<HurtSet, kPhaseCount> hurt{};

    // Throws (kind == Throw): the victim is held for throwDuration ticks, then
    // takes damage, is knocked down and lands throwDistance px in front.
    int throwDuration = 40, throwDistance = 50;
    bool techable = true;                 // LP + LK within the tech window escapes

    // Special input: numpad directions relative to facing (6 = forward,
    // 2 = down, 3 = down-forward, ...), then any of motionButtons.
    std::array<std::uint8_t, kMaxMotionLength> motion{};
    int motionLength = 0;
    std::uint16_t motionButtons = 0;

    [[nodiscard]] constexpr int duration() const { return startup + active + recovery; }
    [[nodiscard]] constexpr MovePhase phaseAt(int age) const {
        return age < startup ? MovePhase::Startup : age < startup + active ? MovePhase::Active : MovePhase::Recovery;
    }
};

struct FighterSpec {
    std::array<MoveSpec, kSlotCount> moves{};
    int walkForward = 420, walkBack = 420;  // 1/kFixed px per tick
    int jumpVelocity = 1480, gravity = 70;  // 1/kFixed px per tick (squared)
    Box standHurt{-18, 0, 36, 100};
    Box crouchHurt{-18, 0, 36, 64};
    int pushWidth = 36;
    int knockdownTicks = 54;                // on the floor (invulnerable) incl. getting up
};

// The concept kit prototype's two archetypes, kept bit-exact for regression
// tests (the golden checksum) and as a fallback when no content is loaded.
[[nodiscard]] const FighterSpec& kitBoxer();
[[nodiscard]] const FighterSpec& kitGrappler();

} // namespace iron::sim
