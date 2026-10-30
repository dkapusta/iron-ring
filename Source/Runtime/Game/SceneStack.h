#pragma once

#include "Game/Scene.h"

#include <memory>
#include <optional>
#include <vector>

namespace iron {

enum class Transition { Cut, Fade };

// The scene stack: title at the bottom, then character select, stage select,
// the fight, a pause menu on top. Scenes ask for changes while handling input
// or updating; changes are applied between calls, never under the scene that
// asked. A Fade transition dims to black, swaps, and fades back in; input is
// ignored meanwhile.
class SceneStack {
public:
    void push(std::unique_ptr<Scene> scene, Transition transition = Transition::Fade);
    void pop(Transition transition = Transition::Fade);
    void popToRoot(Transition transition = Transition::Fade) { popTo(1, transition); }
    // Pops until `depth` scenes remain (pause menu + fight -> back to the select screen).
    void popTo(std::size_t depth, Transition transition = Transition::Fade);
    void replaceTop(std::unique_ptr<Scene> scene, Transition transition = Transition::Fade);
    void quit() { quit_ = true; }

    [[nodiscard]] bool quitting() const { return quit_ || (scenes_.empty() && !pending_); }
    [[nodiscard]] std::size_t depth() const { return scenes_.size(); }
    // Counts applied changes, so the caller can tell that the top scene changed.
    [[nodiscard]] unsigned generation() const { return generation_; }

    // Applies a pending Cut request now (start-up and shutdown, outside the loop).
    void flush() { settle(); }

    void handleEvent(const SDL_Event& event);
    void handleMenu(const MenuEvent& event);
    void fixedUpdate();
    void render(const RenderContext& context, float width, float height);
    [[nodiscard]] std::string status() const;

private:
    static constexpr int kFadeTicks = 9;
    enum class Kind { Push, PopTo, Replace };
    struct Request {
        Kind kind;
        std::unique_ptr<Scene> scene;
        Transition transition;
        std::size_t depth = 0;
    };
    void request(Kind kind, std::unique_ptr<Scene> scene, Transition transition, std::size_t depth = 0);
    void settle(); // applies a pending Cut request
    void apply();
    [[nodiscard]] bool fading() const { return fadeOut_ || fade_ > 0; }

    std::vector<std::unique_ptr<Scene>> scenes_;
    std::optional<Request> pending_;
    int fade_ = 0;        // 0..kFadeTicks: how dark the screen is
    bool fadeOut_ = false; // darkening towards the swap
    bool quit_ = false;
    unsigned generation_ = 0;
};

} // namespace iron
