#pragma once

#include "Audio/AudioSystem.h"
#include "Game/Effects.h"
#include "Game/FighterView.h"
#include "Game/Hud.h"
#include "Game/InputDisplay.h"
#include "Game/Scene.h"
#include "Game/Services.h"
#include "Game/StageView.h"
#include "Game/TrainingOverlay.h"
#include "Core/Camera.h"
#include "Net/Protocol.h"
#include "Render/BitmapFont.h"
#include "Simulation/Cpu.h"
#include "Simulation/Match.h"
#include "Simulation/Replay.h"
#include "Simulation/Rollback.h"
#include "Simulation/Training.h"

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <memory>
#include <string>
#include <vector>

namespace iron {

enum class Control { VsCpu, TwoPlayers, Demo, Online };

// What the front end chose: who fights where, and how.
struct FightSetup {
    Control control = Control::VsCpu;
    bool training = false;          // training rules, the overlay, a dummy opponent
    sim::Dummy dummy = sim::Dummy::Stand;
    std::array<std::string, 2> fighters; // roster ids; empty = the default pair
    std::string stage;              // stage id; empty = GameSettings startStage
    bool attract = false;           // the title screen's demo: any button returns
    bool fromSelect = false;        // the character select is beneath (pause offers it)
    int cpuLevel = 2;               // 0 ROOKIE .. 4 CHAMPION, for every CPU-controlled side
    // Arcade: called once with the match's winner when it is decided; the
    // fight then leaves the stack by itself instead of offering a rematch.
    std::function<void(sim::Outcome)> onFinished;
    // Playback: the inputs come from this recording (fighters and stage must match it).
    std::shared_ptr<const sim::Replay> replay;
    // Online (Control::Online): the server's start of this match; the other
    // side's inputs come from the session.
    std::optional<net::MatchStart> online;
};

// Versus screen: a best-of-3 sim::Match between two fighters on one of the
// stages, with HUD, effects and the training overlay. Player vs CPU, two
// players, or CPU vs CPU. Start/Escape pauses; after the match, a menu offers
// a rematch.
class FightScene final : public Scene {
public:
    FightScene(Services& services, FightSetup setup);

    void handleEvent(const SDL_Event& event) override;
    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

    // For the pause and post-match menus.
    void newMatch();
    void resetPositions();
    [[nodiscard]] const FightSetup& setup() const { return setup_; }
    [[nodiscard]] sim::Dummy dummy() const { return dummy_; }
    void setDummy(sim::Dummy dummy) { dummy_ = dummy; }
    // For the move list.
    [[nodiscard]] const FighterView& view(int side) const { return *views_[static_cast<std::size_t>(side)]; }
    [[nodiscard]] int facing(int side) const { return match_->round.fighters[static_cast<std::size_t>(side)].facing; }
    [[nodiscard]] bool human(int side) const; // a player (not the CPU or the dummy) controls this side
    // Online: move inputs and checksums to and from the server without
    // advancing (the post-match menu keeps the last confirmations flowing).
    void pumpOnline();
    [[nodiscard]] bool online() const { return control_ == Control::Online; }

private:
    void openMenu(bool postMatch);
    void setStage(std::size_t index);
    void followCamera(bool snap);
    // Hit feel: a short horizontal shake of the world, and a white flash.
    void shake(float amplitude, int ticks);
    void flash(int ticks);
    [[nodiscard]] float shakeOffset() const;
    void playStageMusic();
    static constexpr bool kVoice = true, kSound = false;
    // Delayed audio, so the announcer doesn't talk over an impact or a jingle.
    void cue(bool voice, std::string id, int delayTicks);
    void playCues();
    [[nodiscard]] std::string controlName() const;
    void playEvents(const sim::MatchStep& step);
    void saveReplay();
    void finishPlayback();
    // Online (M4.4).
    void startOnline();  // a rollback session for the match setup_.online describes
    void onlineUpdate(); // one frame of an online fight
    void settleOnline(); // after the end, until the result is confirmed
    void playReconciled(const sim::Rollback::TickStep& step); // events of a re-simulated tick play once
    void endOnline(std::string why);                          // the match can't go on: say why, then back to the room
    void present(int events, const std::array<sim::Input, 2>& inputs); // per-tick presentation after a step
    [[nodiscard]] int inputDelay() const;
    void drawShadow(SDL_Renderer* r, float x, float groundY, float height) const;
    [[nodiscard]] std::array<const FighterView*, 2> views() const { return {views_[0].get(), views_[1].get()}; }

    Services& services_;
    AudioSystem& audio_;
    FightSetup setup_;
    std::array<std::unique_ptr<FighterView>, 2> views_;
    std::unique_ptr<sim::Match> match_; // refers to the views' FighterSpecs
    std::unique_ptr<Hud> hud_;
    std::unique_ptr<Effects> effects_;
    std::unique_ptr<TrainingOverlay> overlay_;
    std::unique_ptr<InputDisplay> inputDisplay_;
    std::unique_ptr<StageView> stageView_;
    Camera camera_;
    std::array<std::optional<sim::CpuPlayer>, 2> cpu_; // per side, for CPU-controlled sides
    unsigned matches_ = 0;                             // seeds the CPUs: each rematch plays differently
    std::vector<std::array<sim::Input, 2>> recorded_;  // this match's inputs, for its replay file
    std::size_t playback_ = 0;                         // next input of setup_.replay
    std::unique_ptr<sim::Rollback> rollback_;          // online: this match's session
    int acked_ = 0;                                    // online: our inputs the opponent has
    std::size_t sentChecksums_ = 0;
    bool resultSent_ = false, finishedOnline_ = false;
    std::map<int, std::vector<std::uint64_t>> played_; // online: events already shown, by tick
    std::string banner_;                               // online: why the match stopped, or the leave prompt
    int bannerTicks_ = 0, leaveTicks_ = 0, frames_ = 0;
    float shakeAmplitude_ = 0;
    int shakeTicks_ = 0, shakeTotal_ = 0, flashTicks_ = 0;
    bool training_ = false;
    sim::Dummy dummy_ = sim::Dummy::Stand;
    std::size_t stage_ = 0;
    Control control_ = Control::VsCpu;
    int clock_ = 0; // presentation clock: stops during hitstop
    bool entered_ = false;
    struct Cue {
        bool voice;
        std::string id;
        int tick;
    };
    std::vector<Cue> cues_;
    int audioTick_ = 0; // fixed updates, independent of hitstop
};

} // namespace iron
