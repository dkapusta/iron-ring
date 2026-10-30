#pragma once

#include "Render/BitmapFont.h"

#include <SDL3/SDL.h>

#include <string>
#include <string_view>
#include <vector>

// Shared drawing for the HUD and the menus: palette colours
// (Tools/Art/ironart/palette.py), the brass frame, word wrapping.
namespace iron::ui {

inline constexpr SDL_Color kInk{7, 9, 15, 255};
inline constexpr SDL_Color kNight{20, 26, 48, 230};
inline constexpr SDL_Color kBrass{217, 130, 43, 255};
inline constexpr SDL_Color kBrassLight{242, 176, 78, 255};
inline constexpr SDL_Color kChalk{241, 234, 216, 255};
inline constexpr SDL_Color kCoral{191, 58, 46, 255};
inline constexpr SDL_Color kCoralLight{239, 106, 69, 255};
inline constexpr SDL_Color kTeal{63, 196, 184, 255};
inline constexpr SDL_Color kTealDark{20, 85, 96, 255};
inline constexpr SDL_Color kTealLight{166, 240, 224, 255};
inline constexpr SDL_Color kSteel{61, 74, 102, 255};
inline constexpr SDL_Color kFog{138, 147, 168, 255};
inline constexpr SDL_Color kWhite{255, 255, 255, 255};
// The two sides: player 1 teal, player 2 coral (asset-spec §1).
inline constexpr SDL_Color kSide[2]{kTeal, kCoralLight};

void fill(SDL_Renderer* r, float x, float y, float w, float h, SDL_Color c);
// A brass frame with an ink outline around `box`: the UI's one recurring shape.
void frame(SDL_Renderer* r, const SDL_FRect& box);
// A frame in a flat colour (selection cursors), `thickness` px outside `box`.
void outline(SDL_Renderer* r, const SDL_FRect& box, SDL_Color c, float thickness = 2);
// Darkens the whole logical screen (behind overlays).
void dim(SDL_Renderer* r, float width, float height, Uint8 alpha);

// Splits text into lines of at most `maxWidth` pixels, breaking at spaces.
[[nodiscard]] std::vector<std::string> wrap(const BitmapFont& font, std::string_view text, int maxWidth);
// Uppercase copy for the bitmap fonts (uppercase only; ';' and '&' become ',' and '+').
[[nodiscard]] std::string upper(std::string_view text);

} // namespace iron::ui
