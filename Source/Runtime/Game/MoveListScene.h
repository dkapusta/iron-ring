#pragma once

#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

class FightScene;

// The pause menu's move list: both fighters' moves, with the inputs written
// the way each side faces right now and in the keys that side's player uses
// (the CPU's side shows button names). Any button returns to the pause menu.
class MoveListScene final : public Scene {
public:
    MoveListScene(Services& services, const FightScene& fight);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override {}
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override { return "move list"; }
    [[nodiscard]] bool opaque() const override { return false; }

private:
    [[nodiscard]] std::string button(int side, int action) const; // a key label, or "LP" for the CPU
    void drawSide(SDL_Renderer* r, int side, float x) const;

    Services& services_;
    const FightScene& fight_;
};

} // namespace iron
