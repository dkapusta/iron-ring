#pragma once

#include "Assets/AssetLibrary.h"

#include <string_view>
#include <vector>

namespace iron {

struct SpriteFrame {
    SDL_FRect source{};
    float pivotX = 0; // relative to source rect; y is the foot baseline
    float pivotY = 0;
};

// A texture plus frame rectangles. Two JSON shapes are accepted (both from
// the concept kit): explicit "frames" with source_rect/pivot_pixels, or a
// uniform "columns"/"rows" grid with centred pivots.
class SpriteAtlas {
public:
    SpriteAtlas(AssetLibrary& assets, std::string_view jsonPath);

    [[nodiscard]] const Texture& texture() const { return *texture_; }
    [[nodiscard]] const SpriteFrame& frame(int index) const;
    [[nodiscard]] int frameCount() const { return static_cast<int>(frames_.size()); }
    // Height of one nominal cell; used to keep scale constant across poses.
    [[nodiscard]] float cellHeight() const { return cellHeight_; }

private:
    const Texture* texture_ = nullptr;
    std::vector<SpriteFrame> frames_;
    float cellHeight_ = 1.0f;
};

} // namespace iron
