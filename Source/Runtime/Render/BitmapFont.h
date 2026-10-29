#pragma once

#include "Assets/AssetLibrary.h"

#include <string>
#include <string_view>

namespace iron {

enum class Align { Left, Centre, Right };

// Monospaced bitmap font from a generated atlas (Resources/UI/font_*.json).
// White glyphs are tinted per draw; pre-coloured glyphs (banner font) are
// drawn as they are with tint = white.
class BitmapFont {
public:
    BitmapFont(AssetLibrary& assets, std::string_view jsonPath);

    [[nodiscard]] int measure(std::string_view text) const;
    [[nodiscard]] int lineHeight() const { return cellH_; }

    // shadow: draws an ink copy 1 px down-right first (for tinted small text).
    void draw(SDL_Renderer* r, std::string_view text, float x, float y, SDL_Color tint = {255, 255, 255, 255},
              Align align = Align::Left, bool shadow = false) const;

private:
    void drawRun(SDL_Renderer* r, std::string_view text, float x, float y) const;

    const Texture* texture_ = nullptr;
    std::string charset_;
    int cellW_ = 6, cellH_ = 8, glyphW_ = 5, glyphH_ = 7, advance_ = 6, columns_ = 16;
};

} // namespace iron
