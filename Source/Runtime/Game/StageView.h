#pragma once

#include "Game/GameData.h"
#include "Simulation/Round.h"

#include <SDL3/SDL.h>

#include <vector>

namespace iron {

// Draws a stage for a camera position: a flat image, or its layers with
// parallax, drift and animation. Tracks the crowd's excitement, which hits,
// throws and K.O.s raise and time calms down; crowd layers bob faster with
// it and switch to their cheering frames when it runs high.
class StageView {
public:
    explicit StageView(const StageInfo& stage);

    void excite(float amount);
    void onEvents(const sim::StepEvents& events);
    void update(); // once per fixed tick
    void calm() { excitement_ = 0; }

    void drawBack(SDL_Renderer* r, float cameraX) const;  // everything behind the fighters
    void drawFront(SDL_Renderer* r, float cameraX) const; // foreground layers
    [[nodiscard]] float excitement() const { return excitement_; }

private:
    void drawLayer(SDL_Renderer* r, std::size_t index, float cameraX) const;

    const StageInfo& stage_;
    std::vector<float> phase_; // animation position per layer, in frames
    int ticks_ = 0;
    float excitement_ = 0;
};

} // namespace iron
