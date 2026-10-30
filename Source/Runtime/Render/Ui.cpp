#include "Render/Ui.h"

#include <cctype>

namespace iron::ui {

void fill(SDL_Renderer* r, float x, float y, float w, float h, SDL_Color c) {
    if (w <= 0 || h <= 0) return;
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    const SDL_FRect rect{x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void frame(SDL_Renderer* r, const SDL_FRect& box) {
    fill(r, box.x - 2, box.y - 2, box.w + 4, box.h + 4, kInk);
    fill(r, box.x - 1, box.y - 1, box.w + 2, box.h + 2, kBrass);
    fill(r, box.x - 1, box.y - 1, box.w + 2, 1, kBrassLight);
    fill(r, box.x, box.y, box.w, box.h, kInk);
}

void outline(SDL_Renderer* r, const SDL_FRect& box, SDL_Color c, float t) {
    fill(r, box.x - t, box.y - t, box.w + 2 * t, t, c);
    fill(r, box.x - t, box.y + box.h, box.w + 2 * t, t, c);
    fill(r, box.x - t, box.y, t, box.h, c);
    fill(r, box.x + box.w, box.y, t, box.h, c);
}

void dim(SDL_Renderer* r, float width, float height, Uint8 alpha) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    fill(r, 0, 0, width, height, {kInk.r, kInk.g, kInk.b, alpha});
}

std::vector<std::string> wrap(const BitmapFont& font, std::string_view text, int maxWidth) {
    std::vector<std::string> lines;
    std::string line;
    while (!text.empty()) {
        const auto space = text.find(' ');
        const auto word = text.substr(0, space);
        text = space == std::string_view::npos ? std::string_view{} : text.substr(space + 1);
        if (word.empty()) continue;
        const std::string candidate = line.empty() ? std::string(word) : line + " " + std::string(word);
        if (!line.empty() && font.measure(candidate) > maxWidth) {
            lines.push_back(line);
            line = std::string(word);
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

std::string upper(std::string_view text) {
    std::string out(text);
    for (char& c : out) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (c == ';') c = ','; // not in the bitmap fonts
        if (c == '&') c = '+';
    }
    return out;
}

} // namespace iron::ui
