#pragma once

// Fighter content: the *.fighter.json format (docs/adr/0010-fighter-content-format.md,
// docs/art/asset-spec.md). Parsed into the simulation's FighterSpec plus the
// presentation data the runtime needs (names, animations, atlas paths).

#include "Simulation/Spec.h"

#include <nlohmann/json.hpp>

#include <array>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace iron::content {

// Thrown for malformed content; the message names the file and the field.
struct ContentError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct AnimationFrame {
    std::string frame;        // atlas frame name
    int ticks = 1;
    std::string partnerFrame; // for throw holds: the frame the held opponent shows
};

struct Animation {
    bool loop = false;
    std::vector<AnimationFrame> frames;

    [[nodiscard]] int totalTicks() const;
    // Frame shown `tick` ticks after the animation started (loops or holds the last frame).
    [[nodiscard]] const AnimationFrame& at(int tick) const;
};

struct MoveInfo {
    std::string key;              // "lightPunch", "throw", "special", ...
    std::string name;             // display name ("Stiff Jab")
    std::string input;            // special input notation ("236P"), empty otherwise
    std::string animation;        // played while the move runs
    std::string holdAnimation;    // throws: played while holding the opponent
};

struct FighterDefinition {
    std::string id;
    std::string name;
    std::string displayName;
    std::string atlas;     // virtual path
    std::string portrait;  // virtual path
    sim::FighterSpec spec;
    std::array<MoveInfo, sim::kSlotCount> moves;
    std::map<std::string, Animation, std::less<>> animations;

    [[nodiscard]] const Animation& animation(std::string_view name) const;
    [[nodiscard]] bool hasAnimation(std::string_view name) const;
};

// JSON keys of the move slots, in Slot order.
inline constexpr std::array<std::string_view, sim::kSlotCount> kMoveKeys{
    "lightPunch", "heavyPunch", "lightKick", "heavyKick", "throw", "special"};

// Animations every fighter must provide (the runtime picks them by state).
inline constexpr std::array<std::string_view, 13> kRequiredAnimations{
    "idle", "walk", "crouch", "guard", "guard_crouch", "jump_rise", "jump_fall",
    "hit", "hit_crouch", "held", "knockdown", "ko", "win"};

// "236P" -> motion {2,3,6} + punch buttons. Accepts P, K, LP, HP, LK, HK.
void parseMotion(std::string_view notation, sim::MoveSpec& move);

// `path` is the fighter file's virtual path; atlas/portrait are resolved next to it.
[[nodiscard]] FighterDefinition parseFighter(const nlohmann::json& json, std::string_view path);

} // namespace iron::content
