#pragma once

#include "Game/InputMapper.h"
#include "Game/Scene.h"
#include "Game/SpriteAtlas.h"
#include "Simulation/Round.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace iron {

class AssetLibrary;
class AudioSystem;

// Placeholder versus screen: the ported combat core rendered with the concept
// kit's pose atlases, stages and effects. Replaced piece by piece in M1/M2.
class FightScene final : public Scene {
public:
    FightScene(AssetLibrary& assets, AudioSystem& audio);

    void handleEvent(const SDL_Event& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;

private:
    struct FighterView {
        std::unique_ptr<SpriteAtlas> atlas;
        std::string displayName;
        float heightPixels = 140;
    };
    struct StageView {
        std::string name;
        const Texture* background = nullptr;
    };

    void loadStages(AssetLibrary& assets, const std::string& stagesPath, const std::string& startStage);
    void restartRound();
    void drawFighter(SDL_Renderer* r, const sim::Fighter& f, const FighterView& view) const;
    void drawEffect(const RenderContext& context) const;
    void drawHud(SDL_Renderer* r) const;

    AudioSystem& audio_;
    InputMapper input_;
    sim::Round match_;
    std::array<FighterView, 2> fighters_;
    std::vector<StageView> stages_;
    std::unique_ptr<SpriteAtlas> effects_;
    nlohmann::json hud_;
    std::string meterName_;
    std::size_t stage_ = 0;
    bool cpu_ = true;
    int roundOverTicks_ = 0;
};

} // namespace iron
