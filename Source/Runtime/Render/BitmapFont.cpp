#include "Render/BitmapFont.h"

#include "Content/Paths.h"

#include <cctype>
#include <cmath>

namespace iron {

BitmapFont::BitmapFont(AssetLibrary& assets, std::string_view jsonPath) {
    const auto json = assets.json(jsonPath);
    texture_ = &assets.texture(content::siblingPath(jsonPath, json.at("image").get<std::string>()));
    charset_ = json.at("charset").get<std::string>();
    cellW_ = json.at("cell")[0].get<int>();
    cellH_ = json.at("cell")[1].get<int>();
    glyphW_ = json.at("glyph")[0].get<int>();
    glyphH_ = json.at("glyph")[1].get<int>();
    advance_ = json.value("advance", cellW_);
    columns_ = json.value("columns", 16);
}

int BitmapFont::measure(std::string_view text) const {
    return text.empty() ? 0 : static_cast<int>(text.size()) * advance_ - (advance_ - glyphW_);
}

void BitmapFont::drawRun(SDL_Renderer* r, std::string_view text, float x, float y) const {
    for (const char raw : text) {
        const char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
        const auto index = charset_.find(ch);
        if (ch != ' ' && index != std::string::npos) {
            const SDL_FRect src{static_cast<float>(static_cast<int>(index) % columns_ * cellW_),
                                static_cast<float>(static_cast<int>(index) / columns_ * cellH_),
                                static_cast<float>(glyphW_), static_cast<float>(glyphH_)};
            const SDL_FRect dst{x, y, src.w, src.h};
            SDL_RenderTexture(r, texture_->handle.get(), &src, &dst);
        }
        x += static_cast<float>(advance_);
    }
}

void BitmapFont::draw(SDL_Renderer* r, std::string_view text, float x, float y, SDL_Color tint, Align align,
                      bool shadow) const {
    const float width = static_cast<float>(measure(text));
    if (align == Align::Centre) x -= std::floor(width / 2);
    if (align == Align::Right) x -= width;
    x = std::floor(x);
    y = std::floor(y);
    SDL_Texture* tex = texture_->handle.get();
    if (shadow) {
        SDL_SetTextureColorMod(tex, 7, 9, 15);
        drawRun(r, text, x + 1, y + 1);
    }
    SDL_SetTextureColorMod(tex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(tex, tint.a);
    drawRun(r, text, x, y);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
}

} // namespace iron
