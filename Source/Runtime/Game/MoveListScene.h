#pragma once

#include "Content/Fighter.h"
#include "Game/Scene.h"
#include "Game/Services.h"

#include <string>
#include <vector>

namespace iron {

class FightScene;

// One fighter's column in the move list.
struct MoveListPanel {
    const content::FighterDefinition* fighter = nullptr;
    int facing = 1;     // +1 right, -1 left: motions are drawn as arrows for this facing
    int player = -1;    // whose keys label the buttons (0, 1), or -1 for button names (the CPU)
    std::string label;  // "PLAYER 1", "CPU", ...
};

// A fighter's moves, with the inputs written for the way they face and in the
// keys of the player who uses them. From the pause menu: both fighters as they
// face right now. From the character select (heavy punch): the fighter under
// the cursor. Any button returns.
class MoveListScene final : public Scene {
public:
    MoveListScene(Services& services, std::vector<MoveListPanel> panels, std::string footer);
    // The pause menu's list: both sides of the fight.
    MoveListScene(Services& services, const FightScene& fight);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override {}
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    [[nodiscard]] bool opaque() const override { return false; }

private:
    [[nodiscard]] std::string button(const MoveListPanel& panel, int action) const;
    void drawPanel(SDL_Renderer* r, const MoveListPanel& panel, int colour, float x) const;

    Services& services_;
    std::vector<MoveListPanel> panels_;
    std::string footer_;
};

} // namespace iron
