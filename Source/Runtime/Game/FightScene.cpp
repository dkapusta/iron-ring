#include "Game/FightScene.h"

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Game/FightMenuScene.h"
#include "Game/OnlineSession.h"
#include "Render/Ui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
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
    if (setup_.online) control_ = Control::Online;
    newMatch();
    if (setup_.online) startOnline();
    if (setup_.replay) {
        const auto expected = sim::replayContentHash(views_[0]->definition().spec, views_[1]->definition().spec,
                                                     match_->round.stageWidth);
        if (expected != setup_.replay->contentHash)
            SDL_Log("Replay: recorded with different game data (build %s); it may not play back the same",
                    setup_.replay->gameVersion.c_str());
    }
}

void FightScene::onEnter() {
    if (online()) {
        // Back from the post-match menu: the server started the rematch.
        if (auto start = services_.online->takeMatchStart()) {
            const bool newStage = start->stage != services_.data.stages()[stage_].id;
            setup_.online = std::move(*start);
            if (newStage) setStage(services_.data.stageIndex(setup_.online->stage));
            newMatch();
            startOnline();
            if (newStage) playStageMusic();
            return;
        }
    }
    if (entered_) return; // back from the pause menu: the music carries on
    entered_ = true;
    playStageMusic();
}

void FightScene::playStageMusic() { services_.data.playTheme(audio_, services_.data.stages()[stage_].music); }

void FightScene::setStage(std::size_t index) {
    stage_ = index;
    stageView_ = std::make_unique<StageView>(services_.data.useStage(stage_)); // streams in its layers and theme
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
    // Fresh CPUs for both sides (used when a side is CPU-controlled), seeded per match:
    // deterministic, but a rematch isn't a replay.
    ++matches_;
    for (std::size_t side = 0; side < 2; ++side) {
        const int localSide = setup_.online ? setup_.online->side : 0;
        const bool autoplayer =
            static_cast<int>(side) == localSide && services_.settings.autoplay && control_ != Control::Demo;
        cpu_[side].emplace(autoplayer ? sim::kCpuLevels - 1 : setup_.cpuLevel,
                           0x1F2E3D4Cu ^ (matches_ * 2654435761u) ^ static_cast<unsigned>(side * 40503u));
    }
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
    recorded_.clear();
    playback_ = 0;
    if (entered_) playStageMusic(); // a rematch: the theme stopped at the end of the match
}

bool FightScene::human(int side) const {
    if (control_ == Control::Online) return setup_.online && side == setup_.online->side;
    if (control_ == Control::Demo || setup_.replay) return false;
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
    if (event.action == MenuAction::Pause && online()) {
        // No pause online: a second press within two seconds leaves the match.
        if (leaveTicks_ > 0) {
            services_.online->leaveRoom();
            services_.scenes.pop();
        } else {
            leaveTicks_ = 120;
            audio_.play("menu_move");
        }
        return;
    }
    if (event.action == MenuAction::Pause) {
        audio_.play("menu_confirm");
        openMenu(false);
    }
}

void FightScene::handleEvent(const SDL_Event& event) {
    if (online()) return; // no pause and no developer keys: the other player is live
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
    if (online()) return onlineUpdate();
    const auto& round = match_->round;
    const bool demo = control_ == Control::Demo;
    auto& controls = services_.controls;
    const auto cpu = [&](int side) { return cpu_[static_cast<std::size_t>(side)]->think(round, side); };
    const bool dummyCpu = training_ && dummy_ == sim::Dummy::Cpu;
    const sim::Input p2 = control_ == Control::TwoPlayers ? controls.sample(1)
                        : training_ && !dummyCpu         ? sim::dummyInput(round, 1, dummy_)
                                                          : cpu(1);
    const bool autoplay = services_.settings.autoplay && control_ != Control::TwoPlayers;
    std::array<sim::Input, 2> inputs{demo || autoplay ? cpu(0) : controls.sample(0), p2};
    if (setup_.replay) {
        const auto& recording = setup_.replay->inputs;
        inputs = playback_ < recording.size() ? recording[playback_++] : std::array<sim::Input, 2>{};
    } else {
        recorded_.push_back(inputs);
    }

    const auto step = match_->step(inputs);
    playEvents(step);
    playCues();
    if (step.flow == sim::FlowEvent::Finished && setup_.replay) {
        finishPlayback();
        return;
    }
    if (step.flow == sim::FlowEvent::Finished) {
        saveReplay();
        if (setup_.onFinished) {
            setup_.onFinished(match_->winner);
            services_.scenes.pop();
        } else if (setup_.attract) services_.scenes.pop();
        else openMenu(true); // rematch, character select or title
        return;
    }

    present(step.round.count, inputs);
}

void FightScene::present(int events, const std::array<sim::Input, 2>& inputs) {
    const bool fighting = match_->phase == sim::Phase::Fight;
    if (fighting && training_) inputDisplay_->record(match_->round, inputs);
    if (!fighting || match_->round.hitstop == 0) ++clock_;
    if (fighting && (events > 0 || match_->round.hitstop == 0)) overlay_->observe(match_->round, events > 0);
    for (int side = 0; side < 2; ++side)
        views_[static_cast<std::size_t>(side)]->update(*match_, side, clock_);
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
    if (bannerTicks_ > 0) {
        const SDL_FRect box{170, 160, 300, 40};
        ui::fill(r, box.x, box.y, box.w, box.h, ui::kNight);
        ui::frame(r, box);
        services_.data.small().draw(r, banner_, 320, 168, ui::kCoralLight, Align::Centre, true);
        services_.data.small().draw(r, "BACK TO THE ROOM...", 320, 184, ui::kFog, Align::Centre, true);
    } else if (leaveTicks_ > 0) {
        services_.data.small().draw(r, "PRESS START AGAIN TO LEAVE THE MATCH", 320, 330, ui::kChalk, Align::Centre,
                                    true);
    } else if (online() && rollback_ && rollback_->predicted() > sim::kMaxRollback - 2 && blink) {
        services_.data.small().draw(r, "WAITING FOR THE OPPONENT", 320, 330, ui::kFog, Align::Centre, true);
    }
}

void FightScene::saveReplay() {
    // Every finished match except training and the title's attract demo.
    if (training_ || setup_.attract || services_.userFolder.empty() || recorded_.empty()) return;
    sim::Replay replay;
    replay.gameVersion = IR_VERSION_STRING;
    replay.fighters = setup_.fighters;
    replay.stage = services_.data.stages()[stage_].id;
    replay.stageWidth = match_->round.stageWidth;
    replay.contentHash = sim::replayContentHash(views_[0]->definition().spec, views_[1]->definition().spec,
                                                replay.stageWidth);
    replay.recordedAt = static_cast<std::int64_t>(std::time(nullptr));
    replay.inputs = recorded_;
    replay.finalChecksum = match_->checksum();

    namespace fs = std::filesystem;
    const auto folder = services_.userFolder / "Replays";
    std::error_code ec;
    fs::create_directories(folder, ec);
    char stamp[32];
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::strftime(stamp, sizeof stamp, "%Y%m%d-%H%M%S", &local);
    const auto path = folder / (std::string(stamp) + "_" + replay.fighters[0] + "-vs-" + replay.fighters[1] + ".irr");
    const auto bytes = sim::encodeReplay(replay);
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        SDL_Log("Replay: cannot write %s", path.string().c_str());
        return;
    }
    SDL_Log("Replay saved: %s (%d ticks)", path.string().c_str(), static_cast<int>(replay.inputs.size()));

    // Keep the newest kKeptReplays (the names sort by time).
    constexpr std::size_t kKeptReplays = 50;
    std::vector<fs::path> files;
    for (const auto& e : fs::directory_iterator(folder, ec))
        if (e.path().extension() == ".irr") files.push_back(e.path());
    std::sort(files.begin(), files.end());
    for (std::size_t i = 0; i + kKeptReplays < files.size(); ++i) fs::remove(files[i], ec);
}

void FightScene::finishPlayback() {
    const bool same = match_->checksum() == setup_.replay->finalChecksum && playback_ == setup_.replay->inputs.size();
    const int played = static_cast<int>(playback_), total = static_cast<int>(setup_.replay->inputs.size());
    if (same) SDL_Log("Replay verified: %d ticks, identical result", played);
    else SDL_Log("Replay MISMATCH: the match did not play back the same (%d of %d ticks)", played, total);
    // A batch run of -replay is a check: it ends with the playback.
    if (services_.settings.batchMode && !services_.settings.replay.empty()) return services_.scenes.quit();
    services_.scenes.pop();
}

std::string FightScene::controlName() const {
    if (setup_.replay) return "REPLAY";
    if (online() && rollback_)
        return "ONLINE - PING " + std::to_string(services_.online->pingMs()) + " - DELAY " +
               std::to_string(rollback_->delay());
    if (training_ && control_ == Control::VsCpu) return "TRAINING - DUMMY " + std::string(sim::dummyName(dummy_));
    switch (control_) {
    case Control::VsCpu: return "VS CPU - " + std::string(sim::cpuLevelName(setup_.cpuLevel));
    case Control::TwoPlayers: return training_ ? "TRAINING - 2P" : "2P";
    case Control::Demo: return "CPU VS CPU";
    case Control::Online: return "ONLINE";
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

// ---- online (docs/design/online.md §4.3) ------------------------------------------------------

int FightScene::inputDelay() const {
    // "online-delay" in prefs.cfg: auto (from the ping) or 0..4 frames.
    const auto pref = services_.prefs.get("online-delay").value_or("auto");
    if (pref.size() == 1 && pref[0] >= '0' && pref[0] <= '4') return pref[0] - '0';
    const int oneWay = services_.online->pingMs() / 2;
    return std::clamp((oneWay + 8) / 17 + 1, 1, 3); // a frame of slack over the one-way trip
}

void FightScene::startOnline() {
    const auto& start = *setup_.online;
    rollback_ = std::make_unique<sim::Rollback>(*match_, start.side, inputDelay());
    acked_ = 0;
    sentChecksums_ = 0;
    resultSent_ = finishedOnline_ = false;
    played_.clear();
    banner_.clear();
    bannerTicks_ = leaveTicks_ = 0;
    services_.online->clearMatchEnd();
    SDL_Log("Online fight: match %u, side %d, input delay %d", start.match, start.side + 1, rollback_->delay());
}

void FightScene::pumpOnline() {
    if (!rollback_) return;
    auto& online = *services_.online;
    for (const auto& in : online.takeInputs()) {
        for (std::size_t k = 0; k < in.inputs.size(); ++k)
            rollback_->addRemoteInput(in.from + static_cast<int>(k), in.inputs[k]);
        acked_ = std::max(acked_, in.ack);
    }
    // Everything the opponent hasn't acknowledged, every frame: a lost packet costs nothing.
    const int from = std::max(acked_, rollback_->localTicks() - static_cast<int>(net::kMaxInputsPerPacket));
    online.send(net::Inputs{from, rollback_->localInputsFrom(from), rollback_->remoteTicks(), setup_.online->match});
    const auto& sums = rollback_->checksums();
    for (; sentChecksums_ < sums.size(); ++sentChecksums_)
        online.send(net::Checksum{sums[sentChecksums_].first, sums[sentChecksums_].second, setup_.online->match});
    const auto& m = rollback_->state();
    if (m.finished && !resultSent_ && rollback_->confirmedTicks() == rollback_->tick()) {
        online.send(net::Result{m.winner, setup_.online->match}); // decides only on servers that don't referee
        resultSent_ = true;
    }
}

void FightScene::playReconciled(const sim::Rollback::TickStep& step) {
    // Events are keyed by what they are; a re-run of a tick shows only what it didn't show before.
    const auto key = [](const sim::StepEvents::Event& e) {
        return (static_cast<std::uint64_t>(e.effect) << 48) ^ (static_cast<std::uint64_t>(e.attacker + 1) << 40) ^
               (static_cast<std::uint64_t>(e.combo) << 8) ^ (e.counter ? 1u : 0u);
    };
    auto& seen = played_[step.tick];
    sim::MatchStep fresh;
    for (int i = 0; i < step.step.round.count; ++i) {
        const auto& e = step.step.round.items[static_cast<std::size_t>(i)];
        if (std::find(seen.begin(), seen.end(), key(e)) != seen.end()) continue;
        seen.push_back(key(e));
        fresh.round.push(e.effect, e.x, e.y, e.attacker, e.combo, e.counter);
    }
    const auto flowKey = 0xF100000000000000ULL | static_cast<std::uint64_t>(step.step.flow);
    if (step.step.flow != sim::FlowEvent::None && std::find(seen.begin(), seen.end(), flowKey) == seen.end()) {
        seen.push_back(flowKey);
        fresh.flow = step.step.flow;
    }
    playEvents(fresh);
    // Ticks this old can't be re-run any more.
    played_.erase(played_.begin(), played_.lower_bound(step.tick - sim::kMaxRollback - 2));
}

void FightScene::endOnline(std::string why) {
    if (!banner_.empty()) return;
    banner_ = std::move(why);
    bannerTicks_ = 180;
    audio_.stopMusic(600);
    SDL_Log("Online fight stopped: %s", banner_.c_str());
}

void FightScene::onlineUpdate() {
    auto& online = *services_.online;
    ++frames_;
    if (leaveTicks_ > 0) --leaveTicks_;
    if (bannerTicks_ > 0) {
        // The match is over without a result: hold the message, then back to the room.
        if (--bannerTicks_ == 0) services_.scenes.pop(); // the room carries on, or closes itself
        playCues();
        return;
    }
    if (!online.online()) return endOnline("CONNECTION LOST");
    if (const auto& end = online.matchEnd(); end && end->reason != net::EndReason::Finished) {
        const bool mine = end->culprit == static_cast<std::uint8_t>(setup_.online->side);
        endOnline(end->reason == net::EndReason::Desync ? (end->culprit == 0xFF ? "MATCH OUT OF SYNC"
                                                           : mine               ? "YOUR GAME WENT OUT OF SYNC"
                                                                                : "THE OPPONENT WENT OUT OF SYNC")
                                                        : "THE OPPONENT LEFT");
        return;
    }
    pumpOnline();
    if (rollback_->state().finished) return settleOnline(); // no new ticks: wait for the result to be confirmed
    if (!rollback_->canAdvance()) {
        playCues();
        return; // too far ahead of the opponent: wait (the picture holds)
    }
    // Time sync: the side that runs ahead of the other gives up a frame now and then.
    const int rttFrames = online.pingMs() / 17;
    const int ahead = rollback_->tick() - (rollback_->remoteTicks() + rttFrames / 2);
    if (ahead > 2 && frames_ % 6 == 0) {
        playCues();
        return;
    }
    const int side = setup_.online->side;
    const bool autoplay = services_.settings.autoplay;
    const sim::Input local = autoplay ? cpu_[static_cast<std::size_t>(side)]->think(rollback_->state().round, side)
                                      : services_.controls.sample(0);
    rollback_->addLocalInput(local);
    const auto advance = rollback_->advance();
    *match_ = rollback_->state(); // the views read the match; it is a small copy
    int events = 0;
    for (const auto& step : advance.steps) {
        playReconciled(step);
        if (!step.resimulated) events += step.step.round.count;
    }
    playCues();
    pumpOnline();
    std::array<sim::Input, 2> shown{};
    shown[static_cast<std::size_t>(side)] = local;
    present(events, shown);
}

void FightScene::settleOnline() {
    // The match ended here (perhaps on a guess). The opponent's last inputs either
    // confirm it, or a correction re-runs the end and the fight goes on.
    const auto settle = rollback_->settle();
    for (const auto& step : settle.steps)
        playReconciled(step);
    *match_ = rollback_->state();
    playCues();
    const int side = setup_.online->side;
    if (match_->finished && !finishedOnline_ && rollback_->confirmedTicks() == rollback_->tick()) {
        finishedOnline_ = true;
        // The replay holds the inputs both sides confirmed.
        recorded_.clear();
        for (int t = 0; t < rollback_->confirmedTicks(); ++t) {
            const auto remote = rollback_->remoteInput(t).value_or(0);
            std::array<sim::Input, 2> pair{};
            pair[static_cast<std::size_t>(side)] = rollback_->localInput(t);
            pair[static_cast<std::size_t>(1 - side)] = remote;
            recorded_.push_back(pair);
        }
        saveReplay();
        openMenu(true); // REMATCH, CHANGE FIGHTERS, LEAVE ROOM
        return;
    }
    present(0, {});
}

} // namespace iron
