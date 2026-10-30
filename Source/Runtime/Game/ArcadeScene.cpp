#include "Game/ArcadeScene.h"

#include "Game/FightScene.h"
#include "Render/Ui.h"

#include <cmath>

namespace iron {

using Step = content::ArcadeRun::Step;

ArcadeScene::ArcadeScene(Services& services, std::string player)
    : services_(services), ladder_(*services.data.arcade()), player_(std::move(player)) {
    loadViews();
}

const content::ArcadeBout& ArcadeScene::bout() const { return ladder_.bouts[run_.bout]; }

std::string ArcadeScene::opponent() const {
    return content::arcadeOpponent(bout(), player_, services_.data.playableIds());
}

void ArcadeScene::loadViews() {
    const auto& data = services_.data;
    views_[0] = std::make_unique<FighterView>(services_.assets, data.fighter(player_)->definition);
    views_[1] = std::make_unique<FighterView>(services_.assets, data.fighter(opponent())->definition);
}

void ArcadeScene::onEnter() {
    ticks_ = 0;
    if (!result_) {
        services_.data.playTheme(services_.audio, services_.data.menuTheme());
        return;
    }
    const bool won = *result_ == sim::Outcome::Player1;
    result_.reset();
    run_.finished(won, ladder_.bouts.size());
    if (run_.step == Step::Versus) loadViews();
    if (run_.step == Step::Ending) services_.audio.play("victory");
    if (run_.step != Step::Ending) services_.data.playTheme(services_.audio, services_.data.menuTheme());
}

void ArcadeScene::startFight() {
    FightSetup setup;
    setup.control = Control::VsCpu;
    setup.fighters = {player_, opponent()};
    setup.stage = bout().stage;
    setup.cpuLevel = content::arcadeLevel(bout(), services_.settings.cpuDifficulty);
    setup.onFinished = [this](sim::Outcome winner) { result_ = winner; };
    run_.startFight();
    services_.scenes.push(std::make_unique<FightScene>(services_, std::move(setup)));
}

void ArcadeScene::handleMenu(const MenuEvent& event) {
    const bool confirm = event.action == MenuAction::Confirm || event.action == MenuAction::Pause;
    const bool back = event.action == MenuAction::Back;
    switch (run_.step) {
    case Step::Versus:
        if (confirm && ticks_ > 30) startFight(); // skip the card
        break;
    case Step::Continue:
        if (confirm) {
            services_.audio.play("menu_confirm");
            run_.continueRun();
            ticks_ = 0;
        } else if (back) {
            ticks_ += 60; // count down faster
        }
        break;
    case Step::GameOver:
        if (confirm && ticks_ > 60) services_.scenes.popToRoot();
        break;
    case Step::Ending:
        if (confirm && ticks_ > 120) services_.scenes.popToRoot();
        break;
    case Step::Fight: break;
    }
}

void ArcadeScene::fixedUpdate() {
    ++ticks_;
    switch (run_.step) {
    case Step::Versus:
        if (ticks_ == 1) services_.audio.play("round_bell");
        if (ticks_ >= kVersusTicks) startFight();
        break;
    case Step::Continue:
        if (services_.settings.autoplay) { // soak tests keep going
            run_.continueRun();
            ticks_ = 0;
            break;
        }
        if (ticks_ % 60 == 0) services_.audio.play("menu_move");
        if (ticks_ >= (kContinueSeconds + 1) * 60) {
            run_.giveUp();
            ticks_ = 0;
            services_.audio.stopMusic(600);
        }
        break;
    case Step::GameOver:
        if (ticks_ >= kGameOverTicks) services_.scenes.popToRoot();
        break;
    case Step::Ending:
        if (ticks_ == 200) services_.data.playTheme(services_.audio, services_.data.menuTheme());
        if (ticks_ >= kEndingTicks) services_.scenes.popToRoot(); // like a cabinet, back to the title
        break;
    case Step::Fight: break;
    }
}

void ArcadeScene::drawFighter(SDL_Renderer* r, const FighterView& view, float x, bool flip, bool won) const {
    const auto& def = view.definition();
    const auto& animation = def.animation(won && def.hasAnimation("win") ? "win" : "idle");
    const int tick = won ? std::min(ticks_, animation.totalTicks() - 1) : ticks_;
    view.sheet().draw(r, animation.at(tick).frame, x, 280, flip);
}

void ArcadeScene::drawVersus(SDL_Renderer* r) const {
    const auto& data = services_.data;
    const float w = services_.width;
    const auto& stage = data.stages()[data.stageIndex(bout().stage)];
    // The venue behind, darkened; the two fighters slide in from the sides.
    const SDL_FRect all{0, 0, w, services_.height};
    SDL_RenderTexture(r, stage.image->handle.get(), nullptr, &all);
    ui::dim(r, w, services_.height, 170);
    const float slide = std::max(0.0f, 1.0f - static_cast<float>(ticks_) / 20.0f) * 200.0f;
    drawFighter(r, *views_[0], 150 - slide, false, false);
    drawFighter(r, *views_[1], w - 150 + slide, true, false);

    const std::string count = bout().final ? "FINAL BOUT" : "BOUT " + std::to_string(run_.bout + 1) + " OF " +
                                                                std::to_string(ladder_.bouts.size());
    data.large().draw(r, count, w / 2, 26, ui::kWhite, Align::Centre);
    data.small().draw(r, ui::upper(stage.name), w / 2, 56, ui::kBrassLight, Align::Centre, true);
    if ((ticks_ / 6) % 2 == 0 || ticks_ > 40) data.large().draw(r, "VS", w / 2, 150, ui::kBrassLight, Align::Centre);
    data.large().draw(r, views_[0]->definition().displayName, 16, 296, ui::kWhite, Align::Left);
    data.large().draw(r, views_[1]->definition().displayName, w - 16, 296, ui::kWhite, Align::Right);
    const int level = content::arcadeLevel(bout(), services_.settings.cpuDifficulty);
    data.small().draw(r, "CPU " + std::string(sim::cpuLevelName(level)), w - 16, 322, ui::kCoralLight, Align::Right, true);
    data.small().draw(r, "PLAYER 1", 16, 322, ui::kTeal, Align::Left, true);
    if (run_.continuesUsed > 0)
        data.small().draw(r, "CONTINUES " + std::to_string(run_.continuesUsed), w / 2, 340, ui::kFog, Align::Centre, true);
}

void ArcadeScene::drawContinue(SDL_Renderer* r) const {
    const auto& data = services_.data;
    const float w = services_.width;
    const SDL_FRect all{0, 0, w, services_.height};
    SDL_RenderTexture(r, data.backdrop().handle.get(), nullptr, &all);
    if (run_.step == Step::GameOver) {
        data.large().draw(r, "GAME OVER", w / 2, 160, ui::kWhite, Align::Centre);
        return;
    }
    drawFighter(r, *views_[1], w / 2 + 120, true, true); // the one who beat you, celebrating
    const int left = std::max(0, kContinueSeconds - ticks_ / 60);
    data.large().draw(r, "CONTINUE?", w / 2 - 80, 110, ui::kWhite, Align::Centre);
    data.large().draw(r, std::to_string(left), w / 2 - 80, 150, left <= 3 ? ui::kCoralLight : ui::kBrassLight,
                      Align::Centre);
    if ((ticks_ / 20) % 2 == 0)
        data.small().draw(r, "PRESS START", w / 2 - 80, 190, ui::kChalk, Align::Centre, true);
}

void ArcadeScene::drawEnding(SDL_Renderer* r) const {
    const auto& data = services_.data;
    const float w = services_.width;
    const auto& stage = data.stages()[data.stageIndex(bout().stage)];
    const SDL_FRect all{0, 0, w, services_.height};
    SDL_RenderTexture(r, stage.image->handle.get(), nullptr, &all);
    ui::dim(r, w, services_.height, 150);
    drawFighter(r, *views_[0], w / 2, false, true);
    const auto& name = views_[0]->definition().displayName;
    data.large().draw(r, name, w / 2, 40, ui::kWhite, Align::Centre);
    data.large().draw(r, "TAKES THE CROWN", w / 2, 70, ui::kBrassLight, Align::Centre);
    if (ticks_ > 90) {
        data.small().draw(r, "THE IRON RING HAS A NEW CHAMPION", w / 2, 300, ui::kChalk, Align::Centre, true);
        const std::string continues = run_.continuesUsed == 0 ? "NO CONTINUES" :
                                      std::to_string(run_.continuesUsed) + " CONTINUE" + (run_.continuesUsed > 1 ? "S" : "");
        data.small().draw(r, continues, w / 2, 314, ui::kFog, Align::Centre, true);
    }
    if (ticks_ > 120 && (ticks_ / 30) % 2 == 0)
        data.small().draw(r, "PRESS START", w / 2, 340, ui::kFog, Align::Centre, true);
}

void ArcadeScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    switch (run_.step) {
    case Step::Versus:
    case Step::Fight: drawVersus(r); break;
    case Step::Continue:
    case Step::GameOver: drawContinue(r); break;
    case Step::Ending: drawEnding(r); break;
    }
}

std::string ArcadeScene::status() const {
    const char* steps[]{"versus", "fight", "continue", "game over", "ending"};
    return "arcade: bout " + std::to_string(run_.bout + 1) + "/" + std::to_string(ladder_.bouts.size()) + " vs " +
           opponent() + " (" + steps[static_cast<int>(run_.step)] + ")";
}

} // namespace iron
