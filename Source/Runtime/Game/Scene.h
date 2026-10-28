#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace iron {

struct RenderContext {
    SDL_Renderer* renderer = nullptr;
    float highlightBoost = 1.0f; // > 1 only on HDR output
};

// A screen of the game (fight, later: title, character select, training).
// fixedUpdate() runs at exactly 60 Hz; render() once per displayed frame.
class Scene {
public:
    virtual ~Scene() = default;

    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void fixedUpdate() = 0;
    virtual void render(const RenderContext& context) = 0;
    [[nodiscard]] virtual std::string status() const = 0; // one line for the debug overlay
};

} // namespace iron
