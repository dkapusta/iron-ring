#include "Game/SpriteAtlas.h"

#include <algorithm>
#include <stdexcept>

namespace iron {

SpriteAtlas::SpriteAtlas(AssetLibrary& assets, std::string_view jsonPath) {
    const auto json = assets.json(jsonPath);
    texture_ = &assets.texture(siblingPath(jsonPath, json.at("image").get<std::string>()));

    const int rows = json.value("nominal_rows", json.value("rows", 1));
    cellHeight_ = texture_->height / static_cast<float>(std::max(rows, 1));

    if (json.contains("frames")) {
        for (const auto& f : json.at("frames")) {
            const auto& r = f.at("source_rect");
            const auto& p = f.at("pivot_pixels");
            frames_.push_back({{r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>()},
                               p[0].get<float>(), p[1].get<float>()});
        }
    } else {
        const int columns = std::max(json.value("columns", 1), 1);
        const float w = texture_->width / static_cast<float>(columns);
        for (int row = 0; row < rows; ++row)
            for (int col = 0; col < columns; ++col)
                frames_.push_back({{static_cast<float>(col) * w, static_cast<float>(row) * cellHeight_, w, cellHeight_},
                                   w / 2, cellHeight_ / 2});
    }
    if (frames_.empty()) throw std::runtime_error("Sprite atlas has no frames: '" + std::string(jsonPath) + "'");
}

const SpriteFrame& SpriteAtlas::frame(int index) const {
    return frames_[static_cast<std::size_t>(std::clamp(index, 0, frameCount() - 1))];
}

} // namespace iron
