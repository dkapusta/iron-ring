#include "Game/FightScene.h"

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Game/FightMenuScene.h"
#include "Render/Ui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace iron {
namespace {

const char* soundFor(sim::Effect effect) {
    switch (effect) {
    case sim::Effect::LightHit:  return "light_hit";
    case sim::Effect::HeavyHit:  return "heavy_hit";
    case sim::Effect::Guard:     return "guard";
    case sim::Effect::Breakaway: return "breakaway";
    case sim::Effect::ThrowTech: return "tech";
    case sim::Effect::Slam:      return "slam";
    }
    return "";
}

} // namespace

FightScene::FightScene(Services& services, FightSetup setup)
    : services_(services), audio_(services.audio), setup_(std::move(setup)), control_(setup_.control) {
    auto& data = services_.data;
    const auto& settings = data.settings();
    const auto pair = data.defaultPair();
    for (std::size_t i = 0; i < views_.size(); ++i) {
        if (setup_.fighters[i].empty()) setup_.fighters[i] = pair[i];
        const auto* entry = data.fighter(setup_.fighters[i]);
        if (!entry || !entry->playable())
            throw std::runtime_error("fighter '" + setup_.fighters[i] + "' is not playable");
        views_[i] = std::make_unique<FighterView>(services_.assets, entry->definition);
    }
    setStage(data.stageIndex(setup_.stage));
    effects_ = std::make_unique<Effects>(services_.assets, settings.at("effects").get<std::string>());
    hud_ = std::make_unique<Hud>(services_.assets.json("Resources/UI/hud_layout.json"), data.small(), data.large(),
                                 settings.value("meterName", "METER"));
    overlay_ = std::make_unique<TrainingOverlay>(data.small());
    inputDisplay_ = std::make_unique<InputDisplay>(data.small());
    training_ = setup_.training;
    dummy_ = setup_.dummy;
    if (training_) overlay_->toggle();
    newMatch();
}

void FightScene::onEnter() {
    if (entered_) return; // back from the pause menu: the music carries on
    entered_ = true;
    playStageMusic();
}

void FightScene::playStageMusic() { services_.data.playTheme(audio_, services_.data.stages()[stage_].music); }

void FightScene::setStage(std::size_t index) {
    stage_ = index;
    stageView_ = std::make_unique<StageView>(services_.data.stages()[stage_]);
}

void FightScene::followCamera(bool snap) {
    const auto& f = match_->round.fighters;
    const float a = static_cast<float>(f[0].x) / sim::kFixed, b = static_cast<float>(f[1].x) / sim::kFixed;
    const auto width = static_cast<float>(match_->round.stageWidth);
    if (snap) camera_.snap(a, b, width, services_.width);
    else camera_.follow(a, b, width, services_.width);
}

void FightScene::shake(float amplitude, int ticks) {
    // Reduced screen effects: a one-pixel nudge at most.
    if (services_.settings.reducedEffects) amplitude = std::min(amplitude, 1.0f);
    if (amplitude * static_cast<float>(ticks) < shakeAmplitude_ * static_cast<float>(shakeTicks_)) return;
    shakeAmplitude_ = amplitude;
    shakeTicks_ = shakeTotal_ = ticks;
}

void FightScene::flash(int ticks) {
    if (!services_.settings.reducedEffects) flashTicks_ = std::max(flashTicks_, ticks);
}

float FightScene::shakeOffset() const {
    if (shakeTicks_ <= 0) return 0;
    // Alternating sides every two ticks, dying away.
    const float strength = shakeAmplitude_ * static_cast<float>(shakeTicks_) / static_cast<float>(shakeTotal_);
    return std::round(((shakeTicks_ / 2) % 2 == 0 ? 1.0f : -1.0f) * strength);
}

void FightScene::newMatch() {
    match_ = std::make_unique<sim::Match>(views_[0]->definition().spec, views_[1]->definition().spec);
    match_->round.training = training_;
    match_->round.stageWidth = services_.data.stages()[stage_].definition.width;
    match_->round.reset(); // start positions depend on the width
    followCamera(true);
    stageView_->calm();
    hud_->clearCallouts();
    shakeTicks_ = flashTicks_ = 0;
    effects_->clear();
    inputDisplay_->clear();
    cues_.clear();
    if (entered_) playStageMusic(); // a rematch: the theme stopped at the end of the match
}

bool FightScene::human(int side) const {
    if (control_ == Control::Demo) return false;
    return side == 0 || control_ == Control::TwoPlayers;
}

void FightScene::resetPositions() {
    match_->round.reset();
    effects_->clear();
    followCamera(true);
}

void FightScene::openMenu(bool postMatch) {
    services_.scenes.push(std::make_unique<FightMenuScene>(services_, *this, postMatch), Transition::Cut);
}

void FightScene::handleMenu(const MenuEvent& event) {
    if (setup_.attract) {
        // The demo: Start or Confirm goes back to the title.
        if (event.action == MenuAction::Confirm || event.action == MenuAction::Pause) services_.scenes.pop();
        return;
    }
    if (event.action == MenuAction::Pause) {
        audio_.play("menu_confirm");
        openMenu(false);
    }
}

void FightScene::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST && !setup_.attract && !services_.settings.batchMode) {
        openMenu(false); // pause when the player switches away
        return;
    }
    if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat) return;
    switch (event.key.scancode) {
    case SDL_SCANCODE_TAB:
        control_ = control_ == Control::VsCpu ? Control::TwoPlayers
                 : control_ == Control::TwoPlayers ? Control::Demo : Control::VsCpu;
        break;
    case SDL_SCANCODE_F2:
        setStage((stage_ + 1) % services_.data.stages().size());
        newMatch(); // a stage of another width needs new start positions
        playStageMusic();
        break;
    case SDL_SCANCODE_F3: overlay_->toggle(); break;
    case SDL_SCANCODE_F5: newMatch(); break;
    case SDL_SCANCODE_F4: // training: back to the starting positions, full health
        if (training_) resetPositions();
        break;
    case SDL_SCANCODE_F6:
        if (training_) dummy_ = sim::nextDummy(dummy_);
        break;
    default: break;
    }
}

void FightScene::playEvents(const sim::MatchStep& step) {
    stageView_->onEvents(step.round);
    for (int i = 0; i < step.round.count; ++i) {
        const auto& e = step.round.items[static_cast<std::size_t>(i)];
        effects_->spawn(e, clock_);
        audio_.play(soundFor(e.effect));
        const bool hit = e.effect == sim::Effect::LightHit || e.effect == sim::Effect::HeavyHit;
        if (hit) {
            // Layers over the hit: a sting for a counter, a blip that climbs with the combo.
            if (e.counter) audio_.play("counter");
            if (e.combo >= 2) audio_.play("combo", std::min(1.0f + 0.12f * static_cast<float>(e.combo - 2), 1.7f));
            hud_->onHit(e.attacker, e.combo, e.counter);
            if (e.counter) stageView_->excite(0.25f);
            if (e.combo >= 3) stageView_->excite(0.1f * static_cast<float>(e.combo));
        }
        switch (e.effect) {
        case sim::Effect::HeavyHit: shake(e.counter ? 3.0f : 2.0f, e.counter ? 9 : 6); break;
        case sim::Effect::LightHit: if (e.counter) shake(2.0f, 6); break;
        case sim::Effect::Slam: shake(4.0f, 10); break;
        case sim::Effect::Breakaway:
            shake(2.0f, 6);
            hud_->onBreakaway(e.attacker);
            break;
        default: break;
        }
        if (e.counter) flash(3);
    }
    switch (step.flow) {
    case sim::FlowEvent::RoundCall:
        // The announcer calls the round (it replaced a fanfare: the line needs the whole second before FIGHT!).
        cue(kVoice, sim::isFinalRound(*match_) ? "final_round" : "round_" + std::to_string(match_->roundNumber), 0);
        break;
    case sim::FlowEvent::Fight:
        stageView_->excite(0.3f);
        audio_.play("round_bell");
        cue(kVoice, "fight", 0);
        break;
    case sim::FlowEvent::RoundEnd: {
        const auto& f = match_->round.fighters;
        const bool ko = f[0].hp == 0 || f[1].hp == 0;
        audio_.play(ko ? "ko" : "round_bell");
        stageView_->excite(ko ? 1.0f : 0.4f);
        if (ko) {
            shake(5.0f, 18);
            flash(4);
        }
        // Let the impact land before the call.
        cue(kVoice, !ko ? "time_over" : f[0].hp == 0 && f[1].hp == 0 ? "double_ko" : "ko", ko ? 24 : 6);
        break;
    }
    case sim::FlowEvent::MatchEnd:
        audio_.stopMusic(800);
        if (match_->winner == sim::Outcome::Draw) {
            audio_.play("round_bell");
            cue(kVoice, "draw", 0);
        } else {
            const auto& winner = views_[match_->winner == sim::Outcome::Player1 ? 0 : 1]->definition();
            cue(kVoice, winner.id + "_wins", 0);
            cue(kSound, "victory", 96); // the jingle after the call
        }
        break;
    default: break;
    }
}

void FightScene::cue(bool voice, std::string id, int delayTicks) {
    cues_.push_back({voice, std::move(id), audioTick_ + delayTicks});
}

void FightScene::playCues() {
    ++audioTick_;
    for (auto it = cues_.begin(); it != cues_.end();) {
        if (it->tick > audioTick_) {
            ++it;
            continue;
        }
        if (it->voice) audio_.playVoice(it->id);
        else audio_.play(it->id);
        it = cues_.erase(it);
    }
}

void FightScene::fixedUpdate() {
    const auto& round = match_->round;
    const bool demo = control_ == Control::Demo;
    auto& controls = services_.controls;
    const sim::Input p2 = control_ == Control::TwoPlayers ? controls.sample(1)
                        : training_                       ? sim::dummyInput(round, 1, dummy_)
                                                          : sim::cpuInput(round, 1);
    const std::array<sim::Input, 2> inputs{demo ? sim::cpuInput(round, 0) : controls.sample(0), p2};

    const auto step = match_->step(inputs);
    playEvents(step);
    playCues();
    if (step.flow == sim::FlowEvent::Finished) {
        if (setup_.attract) services_.scenes.pop();
        else openMenu(true); // rematch, character select or title
        return;
    }

    const bool fighting = match_->phase == sim::Phase::Fight;
    if (fighting && training_) inputDisplay_->record(match_->round, inputs);
    if (!fighting || match_->round.hitstop == 0) ++clock_;
    if (fighting && (step.round.count > 0 || match_->round.hitstop == 0))
        overlay_->observe(match_->round, step.round.count > 0);
    for (int side = 0; side < 2; ++side) views_[static_cast<std::size_t>(side)]->update(*match_, side, clock_);
    followCamera(false);
    stageView_->update();
    if (shakeTicks_ > 0) --shakeTicks_; // counts on through hitstop: the shake sells the freeze
    if (flashTicks_ > 0) --flashTicks_;
    effects_->update(clock_);
    hud_->update(*match_);
}

void FightScene::drawShadow(SDL_Renderer* r, float x, float groundY, float height) const {
    // A soft dithered oval that shrinks as the fighter rises.
    const float scale = std::clamp(1.0f - height / 160.0f, 0.4f, 1.0f);
    const int half = static_cast<int>(22 * scale);
    SDL_SetRenderDrawColor(r, 7, 9, 15, 140);
    for (int row = -2; row <= 2; ++row) {
        const int w = half - std::abs(row) * 5;
        for (int dx = -w; dx <= w; ++dx) {
            if ((dx + row) % 2 != 0 && std::abs(dx) > w - 6) continue;
            SDL_RenderPoint(r, std::floor(x) + static_cast<float>(dx), groundY + static_cast<float>(row));
        }
    }
}

void FightScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    const float cx = camera_.offset() + shakeOffset();
    stageView_->drawBack(r, cx);

    const auto& round = match_->round;
    std::array<FighterPose, 2> poses;
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& f = round.fighters[i];
        const auto& other = round.fighters[1 - i];
        if (!views_[1 - i]->heldPose(other, f, poses[i])) poses[i] = views_[i]->pose(f);
        const float ground = static_cast<float>(sim::kFloorY);
        poses[i].x -= cx;
        drawShadow(r, poses[i].x, ground, ground - poses[i].y);
    }

    // The fighter acting (attacking or throwing) is drawn in front.
    const auto acting = [&](const sim::Fighter& f) { return f.move >= 0 || f.throwTicks > 0; };
    const bool p2Front = acting(round.fighters[1]) && !acting(round.fighters[0]);
    for (int n = 0; n < 2; ++n) {
        const std::size_t i = p2Front ? static_cast<std::size_t>(n) : static_cast<std::size_t>(1 - n);
        views_[i]->sheet().draw(r, poses[i].frame, poses[i].x, poses[i].y, poses[i].flip);
    }

    stageView_->drawFront(r, cx);
    effects_->draw(context, clock_, cx);
    if (flashTicks_ > 0) {
        // A brief white wash over the world (not the HUD); above SDR white on HDR displays.
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
        SDL_SetRenderColorScale(r, context.highlightBoost);
        ui::fill(r, 0, 0, services_.width, services_.height, {255, 244, 220, static_cast<Uint8>(22 * flashTicks_)});
        SDL_SetRenderColorScale(r, 1.0f);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    }
    overlay_->draw(r, round, views(), cx);
    if (training_) inputDisplay_->draw(r);
    const bool blink = (clock_ / 30) % 2 == 0;
    const std::string line = setup_.attract ? (blink ? "PRESS START" : "")
                                            : services_.data.stages()[stage_].name + " - " + controlName();
    hud_->draw(r, *match_, views(), ui::upper(line));
}

std::string FightScene::controlName() const {
    if (training_ && control_ == Control::VsCpu) return "TRAINING - DUMMY " + std::string(sim::dummyName(dummy_));
    switch (control_) {
    case Control::VsCpu: return "VS CPU";
    case Control::TwoPlayers: return training_ ? "TRAINING - 2P" : "2P";
    case Control::Demo: return "CPU VS CPU";
    }
    return "";
}

std::string FightScene::status() const {
    char line[160];
    std::snprintf(line, sizeof line, "%s | %s | round %d | Tab mode  F2 stage  F3 boxes  F5 restart%s",
                  services_.data.stages()[stage_].name.c_str(), controlName().c_str(), match_->roundNumber,
                  training_ ? "  F4 reset  F6 dummy" : "");
    return line;
}

} // namespace iron
