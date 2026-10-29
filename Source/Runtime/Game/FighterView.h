#pragma once

#include "Content/Fighter.h"
#include "Render/SpriteSheet.h"
#include "Simulation/Match.h"

#include <string>
#include <string_view>

namespace iron {

// Where and how a fighter is drawn this frame.
struct FighterPose {
    std::string frame;
    float x = 0, y = 0;   // origin (feet) in logical screen pixels
    bool flip = false;
};

// Presentation of one fighter: its content, sprite sheet and the animation
// chosen from simulation state. Animations that mirror a move (attacks,
// throws) are driven by the simulation's own counters, so art and hitboxes
// can never drift apart; ambient ones run on the scene clock.
class FighterView {
public:
    FighterView(AssetLibrary& assets, std::string_view definitionPath);

    [[nodiscard]] const content::FighterDefinition& definition() const { return def_; }
    [[nodiscard]] const SpriteSheet& sheet() const { return sheet_; }
    [[nodiscard]] const Texture& portrait() const { return *portrait_; }

    // Once per fixed tick, after the simulation stepped. `clock` stops during hitstop.
    void update(const sim::Match& match, int side, int clock);

    // Current animation frame (with partner info for throw holds).
    [[nodiscard]] const content::AnimationFrame& currentFrame() const;
    [[nodiscard]] FighterPose pose(const sim::Fighter& f) const;
    // Pose for the opponent this fighter is holding in a throw, if any.
    [[nodiscard]] bool heldPose(const sim::Fighter& thrower, const sim::Fighter& victim, FighterPose& out) const;

private:
    void play(std::string_view animation, int tick);
    void playFromClock(std::string_view animation, int clock);

    content::FighterDefinition def_;
    SpriteSheet sheet_;
    const Texture* portrait_ = nullptr;

    std::string animation_ = "idle";
    int animationTick_ = 0;
    std::string clockAnimation_;
    int clockStart_ = 0;
    int lastX_ = 0;
    bool walkingBack_ = false;
};

} // namespace iron
