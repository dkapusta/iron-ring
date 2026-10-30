#pragma once

#include "Core/Menu.h"
#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

class FightScene;

// The menu over a fight: the pause menu (Start/Escape), or after a match the
// rematch menu. Drawn over the frozen fight.
class FightMenuScene final : public Scene {
public:
    FightMenuScene(Services& services, FightScene& fight, bool postMatch);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override { ++blink_; }
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    [[nodiscard]] bool opaque() const override { return false; }

private:
    void choose(const std::string& id);

    Services& services_;
    FightScene& fight_;
    bool postMatch_;
    Menu menu_;
    int blink_ = 0;
};

} // namespace iron
