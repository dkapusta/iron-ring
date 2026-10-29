#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace iron::content {

// Resolves a path relative to another asset's folder:
//   siblingPath("Fighters/Dee/dee.fighter.json", "dee_atlas.json") == "Fighters/Dee/dee_atlas.json"
[[nodiscard]] std::string siblingPath(std::string_view assetPath, std::string_view relative);

// Width and height from a PNG header, without decoding the image.
[[nodiscard]] std::optional<std::pair<int, int>> pngSize(const std::vector<std::byte>& bytes);

} // namespace iron::content
