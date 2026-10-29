#pragma once

// Sprite atlas JSON (docs/art/asset-spec.md, "Sprite atlas"):
//   { "image": "x.png", "frames": [ {"name", "source_rect": [x,y,w,h], "pivot_pixels": [px,py], "partner": [dx,dy]?} ] }

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace iron::content {

struct AtlasFrame {
    int x = 0, y = 0, w = 0, h = 0;   // source rectangle in the image
    int pivotX = 0, pivotY = 0;       // figure origin inside the rectangle (y = foot baseline)
    std::optional<std::pair<int, int>> partner; // where a held opponent's origin goes (dx forward, dy up)
};

struct AtlasDefinition {
    std::string image; // virtual path
    std::map<std::string, AtlasFrame, std::less<>> frames;

    [[nodiscard]] const AtlasFrame* find(std::string_view name) const;
};

// `path` is the atlas JSON's virtual path; the image is resolved next to it.
[[nodiscard]] AtlasDefinition parseAtlas(const nlohmann::json& json, std::string_view path);

} // namespace iron::content
