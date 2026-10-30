#pragma once

#include "Game/FighterView.h"
#include "Render/BitmapFont.h"
#include "Simulation/Match.h"

#include <nlohmann/json.hpp>

#include <array>
#include <string>

namespace iron {

// Health bars, portraits, names, round markers, Heat meters, timer, the
// round/match banners, and each side's callouts: the combo counter ("3 HITS")
// and COUNTER / BREAKAWAY. Layout comes from Resources/UI/hud_layout.json.
class Hud {
public:
    Hud(nlohmann::json layout, const BitmapFont& small, const BitmapFont& large, std::string meterName);

    void update(const sim::Match& match);
    void onHit(int attacker, int combo, bool counter);
    void onBreakaway(int side);
    void clearCallouts();
    void draw(SDL_Renderer* r, const sim::Match& match, const std::array<const FighterView*, 2>& fighters,
              const std::string& status) const;

private:
    void drawPlayer(SDL_Renderer* r, const sim::Match& match, const FighterView& view, int side) const;
    void drawBanner(SDL_Renderer* r, const sim::Match& match, const std::array<const FighterView*, 2>& fighters) const;
    void drawCallouts(SDL_Renderer* r, int side) const;

    struct Callouts {
        int combo = 0, comboTicks = 0; // the last combo's length, and how long it stays up
        std::string text;              // COUNTER, BREAKAWAY
        int textTicks = 0;
    };
    std::array<Callouts, 2> callouts_{};

    nlohmann::json layout_;
    const BitmapFont& small_;
    const BitmapFont& large_;
    std::string meterName_;
    std::array<float, 2> ghostHealth_{100, 100}; // red "recent damage" bar that drains behind
    int blink_ = 0;
};

} // namespace iron
