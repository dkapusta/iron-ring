#pragma once

#include "Game/FighterView.h"
#include "Render/BitmapFont.h"
#include "Simulation/Match.h"

#include <nlohmann/json.hpp>

#include <array>
#include <string>

namespace iron {

// Health bars, portraits, names, round markers, Heat meters, timer and the
// round/match banners. Layout comes from Resources/UI/hud_layout.json.
class Hud {
public:
    Hud(nlohmann::json layout, const BitmapFont& small, const BitmapFont& large, std::string meterName);

    void update(const sim::Match& match);
    void draw(SDL_Renderer* r, const sim::Match& match, const std::array<const FighterView*, 2>& fighters,
              const std::string& status) const;

private:
    void drawPlayer(SDL_Renderer* r, const sim::Match& match, const FighterView& view, int side) const;
    void drawBanner(SDL_Renderer* r, const sim::Match& match, const std::array<const FighterView*, 2>& fighters) const;

    nlohmann::json layout_;
    const BitmapFont& small_;
    const BitmapFont& large_;
    std::string meterName_;
    std::array<float, 2> ghostHealth_{100, 100}; // red "recent damage" bar that drains behind
    int blink_ = 0;
};

} // namespace iron
