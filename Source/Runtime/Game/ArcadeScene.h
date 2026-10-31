#pragma once

#include "Content/Arcade.h"
#include "Game/FighterView.h"
#include "Game/Scene.h"
#include "Game/Services.h"
#include "Simulation/Match.h"

#include <array>
#include <memory>
#include <optional>

namespace iron {

// One arcade run: the ladder from Data/arcade.json, bout by bout. It opens on
// the player's story; between fights it shows the versus card (the boss bout:
// his greeting), the continue countdown, game over, or the fighter's ending;
// each fight is pushed on top and reports its winner back.
class ArcadeScene final : public Scene {
public:
    ArcadeScene(Services& services, std::string player);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

private:
    static constexpr int kStoryTicks = 60 * 8;
    static constexpr int kVersusTicks = 150;
    static constexpr int kContinueSeconds = 9;
    static constexpr int kGameOverTicks = 180;
    static constexpr int kEndingTicks = 60 * 30;

    [[nodiscard]] const content::ArcadeBout& bout() const;
    [[nodiscard]] const std::string& opponent() const;
    void loadViews();
    void begin();
    void startFight();
    [[nodiscard]] std::size_t titleBouts() const; // the bouts up to and including the title
    void drawStory(SDL_Renderer* r) const;
    void drawVersus(SDL_Renderer* r) const;
    void drawContinue(SDL_Renderer* r) const;
    void drawEnding(SDL_Renderer* r) const;
    void drawFighter(SDL_Renderer* r, const FighterView& view, float x, bool flip, bool won) const;
    void drawChampion(SDL_Renderer* r, const FighterView& view, float x) const;

    Services& services_;
    const content::ArcadeLadder& ladder_;
    std::string player_;
    std::vector<std::string> opponents_; // per bout, fixed for the run
    content::ArcadeRun run_;
    std::array<std::unique_ptr<FighterView>, 2> views_; // player, this bout's opponent
    std::optional<sim::Outcome> result_;                 // set by the fight, read on return
    int ticks_ = 0;                                      // in the current step
};

} // namespace iron
