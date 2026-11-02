#include "Game/FighterView.h"

#include <cstdlib>

namespace iron {
namespace {

content::FighterDefinition load(AssetLibrary& assets, std::string_view path) {
    return content::parseFighter(assets.json(path), path);
}

constexpr int kWinPoseDelay = 45; // ticks into "round over" before the winner celebrates

} // namespace

FighterView::FighterView(AssetLibrary& assets, std::string_view definitionPath)
    : def_(load(assets, definitionPath)), sheet_(assets, def_.atlas), portrait_(&assets.texture(def_.portrait)) {
    if (!def_.selectPortrait.empty()) selectPortrait_ = &assets.texture(def_.selectPortrait);
    if (!def_.ending.empty()) ending_ = &assets.texture(def_.ending);
}

void FighterView::play(std::string_view animation, int tick) {
    animation_ = std::string(animation);
    animationTick_ = tick;
}

void FighterView::playFromClock(std::string_view animation, int clock) {
    if (clockAnimation_ != animation) {
        clockAnimation_ = std::string(animation);
        clockStart_ = clock;
    }
    play(animation, clock - clockStart_);
}

void FighterView::update(const sim::Match& match, int side, int clock) {
    const auto& r = match.round;
    const auto& f = r.fighters[static_cast<std::size_t>(side)];
    const auto& other = r.fighters[static_cast<std::size_t>(1 - side)];
    const int dx = f.x - lastX_;
    lastX_ = f.x;

    const bool roundOver = match.phase == sim::Phase::RoundOver || match.phase == sim::Phase::MatchOver;
    const bool won = match.lastRound == (side == 0 ? sim::Outcome::Player1 : sim::Outcome::Player2);

    if (f.hp == 0) return playFromClock("ko", clock);
    // The winner celebrates once landed and done with the last move.
    if (roundOver && won && match.phaseTicks > kWinPoseDelay && f.y == 0 && f.move < 0)
        return playFromClock("win", clock);
    if (f.heldTicks > 0) return playFromClock("held", clock);
    if (f.throwTicks > 0) {
        const auto& m = r.spec(f).moves[static_cast<std::size_t>(f.throwSlot)];
        clockAnimation_.clear();
        return play(def_.moves[static_cast<std::size_t>(f.throwSlot)].holdAnimation, m.throwDuration - f.throwTicks);
    }
    if (f.knockdown > 0) return playFromClock("knockdown", clock);
    if (f.move >= 0) {
        clockAnimation_.clear();
        return play(def_.moves[static_cast<std::size_t>(f.move)].animation, f.age);
    }
    if (f.stun > 0) {
        if (f.blockstun) return playFromClock(f.crouching ? "guard_crouch" : "guard", clock);
        return playFromClock(f.crouching ? "hit_crouch" : "hit", clock);
    }
    if (f.y > 0) return playFromClock(f.vy > 0 ? "jump_rise" : "jump_fall", clock);
    // Guarding shows only when there is something to block; otherwise walk back.
    const bool threatened = other.move >= 0 || other.throwTicks > 0;
    if (f.crouching) return playFromClock(f.guarding && threatened ? "guard_crouch" : "crouch", clock);
    if (f.guarding && threatened) return playFromClock("guard", clock);
    if (dx != 0 && match.phase == sim::Phase::Fight) {
        walkingBack_ = (dx > 0) != (f.facing > 0);
        return playFromClock("walk", clock);
    }
    playFromClock("idle", clock);
}

const content::AnimationFrame& FighterView::currentFrame() const {
    const auto& animation = def_.animation(animation_);
    if (animation_ == "walk" && walkingBack_) {
        // Walking backwards plays the cycle in reverse.
        const int total = animation.totalTicks();
        return animation.at(total - 1 - animationTick_ % total);
    }
    return animation.at(animationTick_);
}

FighterPose FighterView::pose(const sim::Fighter& f) const {
    return {currentFrame().frame, static_cast<float>(f.x) / sim::kFixed,
            static_cast<float>(sim::kFloorY) - static_cast<float>(f.y) / sim::kFixed, f.facing < 0};
}

bool FighterView::heldPose(const sim::Fighter& thrower, const sim::Fighter& victim, FighterPose& out) const {
    if (thrower.throwTicks <= 0 || victim.heldTicks <= 0) return false;
    const auto& frame = currentFrame();
    const auto* atlasFrame = sheet_.frame(frame.frame);
    if (frame.partnerFrame.empty() || !atlasFrame || !atlasFrame->partner) return false;
    const auto [dx, dy] = *atlasFrame->partner;
    out.frame = frame.partnerFrame;
    out.x = static_cast<float>(thrower.x) / sim::kFixed + static_cast<float>(thrower.facing * dx);
    out.y = static_cast<float>(sim::kFloorY) - static_cast<float>(thrower.y) / sim::kFixed - static_cast<float>(dy);
    out.flip = victim.facing < 0;
    return true;
}

} // namespace iron
