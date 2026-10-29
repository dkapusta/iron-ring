#include "Render/SpriteSheet.h"

#include <cmath>

namespace iron {

SpriteSheet::SpriteSheet(AssetLibrary& assets, std::string_view atlasJsonPath)
    : atlas_(content::parseAtlas(assets.json(atlasJsonPath), atlasJsonPath)), texture_(&assets.texture(atlas_.image)) {}

void SpriteSheet::draw(SDL_Renderer* renderer, std::string_view name, float x, float y, bool flip) const {
    const auto* f = atlas_.find(name);
    if (!f) return;
    // Snap to whole logical pixels so pixel art never shimmers between frames.
    const float px = std::floor(x), py = std::floor(y);
    const float left = flip ? px - static_cast<float>(f->w - f->pivotX) : px - static_cast<float>(f->pivotX);
    const SDL_FRect src{static_cast<float>(f->x), static_cast<float>(f->y), static_cast<float>(f->w),
                        static_cast<float>(f->h)};
    const SDL_FRect dst{left, py - static_cast<float>(f->pivotY), src.w, src.h};
    SDL_RenderTextureRotated(renderer, texture(), &src, &dst, 0.0, nullptr, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

} // namespace iron
