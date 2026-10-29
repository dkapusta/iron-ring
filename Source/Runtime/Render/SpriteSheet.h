#pragma once

#include "Assets/AssetLibrary.h"
#include "Content/Atlas.h"

#include <string_view>

namespace iron {

// A texture plus named frames with pivots (content::AtlasDefinition).
class SpriteSheet {
public:
    SpriteSheet(AssetLibrary& assets, std::string_view atlasJsonPath);

    [[nodiscard]] const content::AtlasDefinition& atlas() const { return atlas_; }
    [[nodiscard]] const content::AtlasFrame* frame(std::string_view name) const { return atlas_.find(name); }
    [[nodiscard]] SDL_Texture* texture() const { return texture_->handle.get(); }

    // Draws a frame with its pivot at (x, y) in logical pixels; flip mirrors
    // around the pivot (the art faces right).
    void draw(SDL_Renderer* renderer, std::string_view name, float x, float y, bool flip) const;

private:
    content::AtlasDefinition atlas_;
    const Texture* texture_ = nullptr;
};

} // namespace iron
