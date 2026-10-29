#pragma once

// Whole-project content validation. AssetPacker runs it on exactly the files
// it is about to pack, so broken data fails the build instead of the game.

#include "Content/Atlas.h"
#include "Content/Fighter.h"

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace iron::content {

using FileReader = std::function<std::optional<std::vector<std::byte>>(std::string_view virtualPath)>;

struct ValidationReport {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    [[nodiscard]] bool ok() const { return errors.empty(); }
};

// Frame names every fighter atlas must contain because other fighters' throw
// animations show them on the held opponent.
inline constexpr std::array<std::string_view, 2> kSharedFrames{"held_0", "held_1"};

// Effect frames the runtime spawns from simulation events.
inline constexpr std::array<std::string_view, 6> kEffects{"light_hit", "heavy_hit", "guard", "breakaway", "tech",
                                                          "slam"};
// Sound ids the game plays (Game/FightScene.cpp); each must be in GameSettings.json -> sounds.
inline constexpr std::array<std::string_view, 10> kSounds{"light_hit", "heavy_hit", "guard", "breakaway", "tech",
                                                          "slam",      "round_call", "round_bell", "ko", "victory"};

[[nodiscard]] std::vector<std::string> validateFighter(const FighterDefinition& fighter, const AtlasDefinition& atlas,
                                                       std::pair<int, int> imageSize);

[[nodiscard]] ValidationReport validateProject(const FileReader& read);

} // namespace iron::content
