#pragma once

#include "Game/Controls.h"

#include <SDL3/SDL.h>

#include <string>

namespace iron {

struct RenderContext {
    SDL_Renderer* renderer = nullptr;
    float highlightBoost = 1.0f; // > 1 only on HDR output
};

// A screen of the game: title, character select, a fight, the pause menu.
// Scenes live on a SceneStack; only the top one gets input and updates.
// fixedUpdate() runs at exactly 60 Hz; render() once per displayed frame.
class Scene {
public:
    virtual ~Scene() = default;

    virtual void handleEvent(const SDL_Event& /*event*/) {} // raw events (debug keys)
    virtual void handleMenu(const MenuEvent& /*event*/) {}
    virtual void fixedUpdate() = 0;
    virtual void render(const RenderContext& context) = 0;
    [[nodiscard]] virtual std::string status() const = 0; // one line for the debug overlay

    // Became the top scene: pushed, or the scene above it was popped.
    virtual void onEnter() {}
    // False for overlays (pause menu): the scene below is drawn first.
    [[nodiscard]] virtual bool opaque() const { return true; }
};

} // namespace iron
