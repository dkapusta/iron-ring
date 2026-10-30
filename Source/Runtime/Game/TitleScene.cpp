#include "Game/TitleScene.h"

#include "Game/CharacterSelectScene.h"
#include "Game/FightScene.h"
#include "Game/MenuView.h"
#include "Game/OptionsScene.h"
#include "Render/Ui.h"

#include <cmath>

namespace iron {

TitleScene::TitleScene(Services& services) : services_(services) {
    menu_.add(MenuItem::action("arcade", "ARCADE", services_.data.arcade() != nullptr));
    menu_.add(MenuItem::action("versus", "VERSUS CPU"));
    menu_.add(MenuItem::action("2p", "2 PLAYERS"));
    menu_.add(MenuItem::action("training", "TRAINING"));
    menu_.add(MenuItem::action("options", "OPTIONS"));
    menu_.add(MenuItem::action("quit", "QUIT"));
    venue_ = services_.data.stageIndex("");
}

void TitleScene::onEnter() {
    idle_ = 0;
    services_.data.playTheme(services_.audio, services_.data.menuTheme());
    if (fromAttract_) {
        // Back from the demo: the press-start screen, on the next venue.
        fromAttract_ = false;
        showMenu_ = false;
        venue_ = (venue_ + 1) % services_.data.stages().size();
    }
}

void TitleScene::startAttract() {
    FightSetup setup;
    setup.control = Control::Demo;
    setup.attract = true;
    setup.stage = services_.data.stages()[venue_].id;
    // Alternate the sides on each demo.
    auto pair = services_.data.defaultPair();
    if (venue_ % 2 == 1) std::swap(pair[0], pair[1]);
    setup.fighters = pair;
    fromAttract_ = true;
    services_.scenes.push(std::make_unique<FightScene>(services_, std::move(setup)));
}

void TitleScene::choose(const std::string& id) {
    auto& scenes = services_.scenes;
    if (id == "arcade") scenes.push(std::make_unique<CharacterSelectScene>(services_, SelectMode::Arcade));
    else if (id == "versus") scenes.push(std::make_unique<CharacterSelectScene>(services_, SelectMode::VsCpu));
    else if (id == "2p") scenes.push(std::make_unique<CharacterSelectScene>(services_, SelectMode::TwoPlayers));
    else if (id == "training") scenes.push(std::make_unique<CharacterSelectScene>(services_, SelectMode::Training));
    else if (id == "options") scenes.push(std::make_unique<OptionsScene>(services_));
    else if (id == "quit") scenes.quit();
}

void TitleScene::handleMenu(const MenuEvent& event) {
    idle_ = 0;
    if (!showMenu_) {
        if (event.action == MenuAction::Confirm || event.action == MenuAction::Pause) {
            services_.audio.play("menu_confirm");
            showMenu_ = true;
        }
        return;
    }
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: choose(menu_.focused().id); break;
    case MenuResult::Back: showMenu_ = false; break;
    default: break;
    }
}

void TitleScene::fixedUpdate() {
    ++clock_;
    if (++idle_ >= kAttractTicks) {
        idle_ = 0;
        startAttract();
    }
}

void TitleScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    const float w = services_.width, h = services_.height;
    const SDL_FRect all{0, 0, w, h};
    SDL_RenderTexture(r, data.stages()[venue_].image->handle.get(), nullptr, &all);
    ui::dim(r, w, h, 170);

    const auto& logo = data.logo();
    const SDL_FRect logoRect{std::floor((w - logo.width) / 2), 34, logo.width, logo.height};
    SDL_RenderTexture(r, logo.handle.get(), nullptr, &logoRect);
    data.small().draw(r, "GRAYHAVEN 1992 - INVITATION ONLY", w / 2, logoRect.y + logo.height + 6, ui::kBrassLight,
                      Align::Centre, true);

    if (showMenu_) {
        MenuStyle style;
        style.y = 200;
        style.width = 160;
        drawMenu(r, menu_, data.small(), style, clock_);
    } else if ((clock_ / 30) % 2 == 0) {
        data.large().draw(r, "PRESS START", w / 2, 236, ui::kWhite, Align::Centre);
    }
    data.small().draw(r, "DKAPUSTA 2026", 8, h - 14, ui::kFog, Align::Left, true);
    data.small().draw(r, std::string("V") + IR_VERSION_STRING, w - 8, h - 14, ui::kFog, Align::Right, true);
}

std::string TitleScene::status() const {
    return showMenu_ ? "title menu: " + menu_.focused().id : "title: press start (demo in " +
                                                                 std::to_string((kAttractTicks - idle_) / 60) + " s)";
}

} // namespace iron
