#pragma once

#include "Game/FightScene.h"
#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

// Stage select: one venue at a time, with its theme and a line about it.
// Remembers the last venue picked (prefs.cfg: last-stage).
class StageSelectScene final : public Scene {
public:
    StageSelectScene(Services& services, FightSetup setup);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;

private:
    Services& services_;
    FightSetup setup_;
    std::size_t stage_ = 0;
    int clock_ = 0;
    int slide_ = 0; // -1/1 for a few ticks after a change: the arrow that was pressed lights up
    int slideTicks_ = 0;
};

} // namespace iron
