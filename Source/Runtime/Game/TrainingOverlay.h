#pragma once

#include "Game/FighterView.h"
#include "Render/BitmapFont.h"
#include "Simulation/Round.h"

#include <array>
#include <string>

namespace iron {

// F3: hurt/hit/grab/push boxes straight from the simulation, the move each
// fighter is performing with its frame data, and frame advantage after
// every hit or block.
class TrainingOverlay {
public:
    explicit TrainingOverlay(const BitmapFont& font) : font_(font) {}

    void toggle() { visible_ = !visible_; }
    [[nodiscard]] bool visible() const { return visible_; }

    // Call once per simulated tick (not during hitstop), after Round::step.
    void observe(const sim::Round& round, bool contactThisTick);
    // cameraX: world pixels at the screen's left edge.
    void draw(SDL_Renderer* r, const sim::Round& round, const std::array<const FighterView*, 2>& views,
              float cameraX) const;

private:
    const BitmapFont& font_;
    bool visible_ = false;
    bool measuring_ = false;
    int tick_ = 0;
    std::array<int, 2> freeAt_{-1, -1};
    std::string advantage_ = "-";
};

} // namespace iron
