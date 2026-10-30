#pragma once

#include "Game/FighterView.h"
#include "Game/Scene.h"
#include "Game/Services.h"

#include <array>
#include <memory>
#include <unordered_map>

namespace iron {

enum class SelectMode { VsCpu, TwoPlayers, Training };

// Character select: the roster grid with every planned fighter (the ones not
// built yet are locked), a full-size preview of each side's choice, then the
// stage select. Versus CPU and training: player 1 picks both sides in turn.
// Two players: each picks their own side at the same time.
class CharacterSelectScene final : public Scene {
public:
    CharacterSelectScene(Services& services, SelectMode mode);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

private:
    static constexpr int kColumns = 3;
    static constexpr int kCell = 56;
    static constexpr int kGap = 8;
    static constexpr int kReadyTicks = 45; // both picked: a beat before the stage select

    [[nodiscard]] int sideFor(const MenuEvent& event) const; // which side an event steers
    void pick(int side);
    void unpick(int side);
    [[nodiscard]] SDL_FRect cellRect(int index) const;
    void drawGrid(SDL_Renderer* r) const;
    void drawSide(SDL_Renderer* r, int side) const;
    [[nodiscard]] const FighterView* view(int index) const;

    Services& services_;
    SelectMode mode_;
    std::unordered_map<std::string, std::unique_ptr<FighterView>> views_; // by fighter id, playable only
    std::array<int, 2> cursor_{};
    std::array<bool, 2> picked_{};
    std::array<int, 2> pickedAt_{}; // clock tick of the pick (the win pose plays from there)
    int ready_ = 0;
    int clock_ = 0;
};

} // namespace iron
