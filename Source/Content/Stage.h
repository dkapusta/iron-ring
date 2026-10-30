#pragma once

// Stage catalogue entries (Assets/Data/stages.json; docs/art/asset-spec.md §3).
//
// A stage is either one flat 640x360 image, or a stack of layers over a world
// wider than the screen. Layers scroll with the camera by their parallax
// factor (0 = fixed to the screen, 1 = moves with the floor, > 1 = foreground
// that passes faster), can drift on their own (steam, water), and can be
// animated (a crowd that bobs, then cheers when the fight heats up).

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace iron::content {

struct StageLayer {
    std::string image;        // virtual path
    float parallax = 1.0f;    // horizontal scroll per pixel of camera movement
    int x = 0, y = 0;         // top-left at camera 0, in logical pixels
    float drift = 0.0f;       // px per second of automatic scroll (the layer repeats horizontally)
    int frames = 1;           // animation frames stacked vertically in the image
    int frameTicks = 10;      // ticks per frame at rest
    bool crowd = false;       // frame rate follows the crowd's excitement; second half of frames = cheering
    bool front = false;       // drawn over the fighters
};

struct StageDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string image;        // 640x360: the flat stage, or a preview of a layered one
    std::string music;        // theme id; empty = no music
    int width = 640;          // world width in logical pixels (>= 640)
    std::vector<StageLayer> layers; // back to front; empty = draw `image`

    [[nodiscard]] bool layered() const { return !layers.empty(); }
    // The width a layer must cover so it never runs out while the camera
    // scrolls from 0 to width - 640 (drifting layers repeat instead).
    [[nodiscard]] int requiredWidth(const StageLayer& layer) const;
};

[[nodiscard]] StageDefinition parseStage(const nlohmann::json& json, std::string_view path);

} // namespace iron::content
