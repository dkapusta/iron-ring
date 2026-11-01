#include "Game/FightMenuScene.h"

#include "Game/FightScene.h"
#include "Game/MenuView.h"
#include "Game/MoveListScene.h"
#include "Game/OnlineSession.h"
#include "Game/OptionsScene.h"
#include "Render/Ui.h"

#include <cmath>

namespace iron {
namespace {

std::vector<std::string> dummyChoices() {
    std::vector<std::string> names;
    for (const auto mode : sim::kDummyModes) names.emplace_back(sim::dummyName(mode));
    return names;
}

} // namespace

FightMenuScene::FightMenuScene(Services& services, FightScene& fight, bool postMatch)
    : services_(services), fight_(fight), postMatch_(postMatch) {
    const auto& setup = fight_.setup();
    if (fight_.online()) {
        // Online there is no pause: this is only ever the post-match menu.
        menu_.add(MenuItem::action("rematch", "REMATCH"));
        menu_.add(MenuItem::action("fighters", "CHANGE FIGHTERS"));
        menu_.add(MenuItem::action("leave", "LEAVE ROOM"));
        return;
    }
    if (postMatch_) {
        menu_.add(MenuItem::action("restart", "REMATCH"));
    } else {
        menu_.add(MenuItem::action("resume", "RESUME"));
        if (setup.training) {
            menu_.add(MenuItem::action("reset", "RESET POSITIONS"));
            menu_.add(MenuItem::setting("dummy", "DUMMY", dummyChoices(), static_cast<int>(fight_.dummy())));
        }
        menu_.add(MenuItem::action("restart", setup.training ? "RESTART" : "RESTART MATCH"));
        menu_.add(MenuItem::action("moves", "MOVE LIST"));
        menu_.add(MenuItem::action("options", "OPTIONS"));
    }
    if (setup.fromSelect) menu_.add(MenuItem::action("select", "CHARACTER SELECT"));
    menu_.add(MenuItem::action("title", "QUIT TO TITLE"));
}

void FightMenuScene::handleMenu(const MenuEvent& event) {
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: choose(menu_.focused().id); break;
    case MenuResult::Changed:
        if (menu_.focused().id == "dummy") fight_.setDummy(sim::kDummyModes[static_cast<std::size_t>(menu_.focused().choice)]);
        break;
    case MenuResult::Back:
        if (!postMatch_) services_.scenes.pop(Transition::Cut); // after a match there is nothing to resume
        break;
    default: break;
    }
}

void FightMenuScene::fixedUpdate() {
    ++blink_;
    if (!fight_.online()) return;
    auto& online = *services_.online;
    auto& scenes = services_.scenes;
    fight_.pumpOnline(); // the opponent may still need this side's last inputs
    if (services_.settings.autoplay && blink_ == 90) choose("rematch"); // soak tests fight on
    if (online.hasMatchStart()) {
        scenes.pop(Transition::Cut); // the rematch: the fight starts it on the way back
        return;
    }
    // The opponent left, changed fighters, or the server went away: back to the room.
    if (!online.online() || !online.hasOpponent() || online.room()->phase == net::RoomPhase::Selecting)
        scenes.popTo(scenes.depth() - 2);
}

void FightMenuScene::choose(const std::string& id) {
    auto& scenes = services_.scenes;
    if (id == "rematch") {
        if (rematchSent_) return;
        services_.online->send(net::Rematch{});
        rematchSent_ = true;
        menu_.find("rematch")->label = "WAITING FOR THE OPPONENT";
    } else if (id == "fighters") {
        services_.online->wantSelect = true;
        scenes.popTo(scenes.depth() - 2); // the room opens the select
    } else if (id == "leave") {
        services_.online->leaveRoom();
        scenes.popTo(scenes.depth() - 2);
    } else if (id == "resume") {
        scenes.pop(Transition::Cut);
    } else if (id == "reset") {
        fight_.resetPositions();
        scenes.pop(Transition::Cut);
    } else if (id == "restart") {
        fight_.newMatch();
        scenes.pop(Transition::Cut);
    } else if (id == "moves") {
        scenes.push(std::make_unique<MoveListScene>(services_, fight_), Transition::Cut);
    } else if (id == "options") {
        scenes.push(std::make_unique<OptionsScene>(services_));
    } else if (id == "select") {
        scenes.popTo(scenes.depth() - 2); // this menu and the fight
    } else if (id == "title") {
        scenes.popToRoot();
    }
}

void FightMenuScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    // After a match the winner's banner stays readable above the menu.
    ui::dim(r, services_.width, services_.height, postMatch_ ? 90 : 150);
    const float panelH = 14.0f * static_cast<float>(menu_.items().size()) + 18;
    const float centre = postMatch_ ? 236.0f : 170.0f;
    const SDL_FRect panel{200, std::floor(centre - panelH / 2), 240, panelH};
    ui::frame(r, panel);
    ui::fill(r, panel.x, panel.y, panel.w, panel.h, ui::kNight);
    if (!postMatch_) data.large().draw(r, "PAUSED", 320, panel.y - 30, ui::kWhite, Align::Centre);
    MenuStyle style;
    style.y = panel.y + 11;
    style.width = 200;
    drawMenu(r, menu_, data.small(), style, blink_);
}

std::string FightMenuScene::status() const { return (postMatch_ ? "post-match menu: " : "paused: ") + fight_.status(); }

} // namespace iron
