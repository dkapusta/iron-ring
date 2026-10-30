#pragma once

#include "Core/Menu.h"
#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

// Options -> Controls: each player's keys, and the buttons of the pad in that
// player's slot (saved per device). Choose an action, then press the new key
// or pad button; Escape cancels. A key taken from another action leaves that
// action unbound, and the screen says so.
class ControlsScene final : public Scene {
public:
    explicit ControlsScene(Services& services);

    void handleEvent(const SDL_Event& event) override;
    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;

private:
    static constexpr int kCaptureTicks = 60 * 5; // give up waiting for a press after 5 s

    [[nodiscard]] int player() const { return menu_.find("player")->choice; }
    void finishCapture(std::string message);
    void close();

    Services& services_;
    Menu menu_;
    int capturing_ = -1; // action index waiting for a press
    int captureLeft_ = 0;
    bool swallow_ = false; // the press that ended a capture must not also act as a menu input
    bool changed_ = false;
    std::string message_;
    int messageTicks_ = 0;
    int clock_ = 0;
};

} // namespace iron
