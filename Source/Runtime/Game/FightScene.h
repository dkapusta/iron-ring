#pragma once

#include "Audio/AudioSystem.h"
#include "Game/Effects.h"
#include "Game/FighterView.h"
#include "Game/Hud.h"
#include "Game/InputMapper.h"
#include "Game/Scene.h"
#include "Game/TrainingOverlay.h"
#include "Render/BitmapFont.h"
#include "Simulation/Match.h"

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace iron {

class AssetLibrary;


enum class Control { VsCpu, TwoPlayers, Demo };

struct FightOptions {
    Control control = Control::VsCpu;
    bool training = false;
    std::string stage; // stage id; empty = GameSettings startStage
};

// Versus screen: a best-of-3 sim::Match between the two fighters listed in
// ProjectSettings/GameSettings.json, on one of the stages, with HUD, effects
// and the training overlay. Player vs CPU, two players, or CPU vs CPU.
class FightScene final : public Scene {
public:
    FightScene(AssetLibrary& assets, AudioSystem& audio, FightOptions options);

    void handleEvent(const SDL_Event& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;

private:
    struct StageView {
        std::string name;
        const Texture* background = nullptr;
        std::string music; // theme id in music.json; empty = no music
    };

    void loadStages(AssetLibrary& assets, const std::string& stagesPath, const std::string& startStage);
    void loadMusic(AssetLibrary& assets, const std::string& musicPath);
    void newMatch();
    void playStageMusic();
    [[nodiscard]] const char* controlName() const;
    void playEvents(const sim::MatchStep& step);
    void drawShadow(SDL_Renderer* r, float x, float groundY, float height) const;
    [[nodiscard]] std::array<const FighterView*, 2> views() const { return {views_[0].get(), views_[1].get()}; }

    AudioSystem& audio_;
    InputMapper input_;
    std::array<std::unique_ptr<FighterView>, 2> views_;
    std::unique_ptr<sim::Match> match_; // refers to the views' FighterSpecs
    std::vector<StageView> stages_;
    std::unique_ptr<BitmapFont> small_, large_;
    std::unique_ptr<Hud> hud_;
    std::unique_ptr<Effects> effects_;
    std::unique_ptr<TrainingOverlay> overlay_;
    std::size_t stage_ = 0;
    Control control_ = Control::VsCpu;
    int clock_ = 0; // presentation clock: stops during hitstop
    std::unordered_map<std::string, AudioSystem::Music> themes_; // by theme id
};

} // namespace iron
