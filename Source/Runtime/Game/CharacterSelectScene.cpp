#include "Game/CharacterSelectScene.h"

#include "Game/ArcadeScene.h"
#include "Game/MenuView.h"
#include "Game/MoveListScene.h"
#include "Game/OnlineScenes.h"
#include "Game/OnlineSession.h"
#include "Game/StageSelectScene.h"
#include "Render/Ui.h"

#include <cmath>

namespace iron {
namespace {

const char* title(SelectMode mode) {
    switch (mode) {
    case SelectMode::VsCpu: return "VERSUS CPU";
    case SelectMode::TwoPlayers: return "2 PLAYERS";
    case SelectMode::Training: return "TRAINING";
    case SelectMode::Arcade: return "ARCADE";
    case SelectMode::Online: return "ONLINE";
    }
    return "";
}

} // namespace

CharacterSelectScene::CharacterSelectScene(Services& services, SelectMode mode) : services_(services), mode_(mode) {
    const auto& roster = services_.data.roster();
    for (const auto& e : roster)
        if (e.playable()) views_.emplace(e.id, std::make_unique<FighterView>(services_.assets, e.definition));
    // Cursors start on the default pair (player 1 on Dee, player 2 on Bram).
    const auto pair = services_.data.defaultPair();
    for (std::size_t side = 0; side < 2; ++side)
        for (std::size_t i = 0; i < roster.size(); ++i)
            if (roster[i].id == pair[side]) cursor_[side] = static_cast<int>(i);
    if (mode_ == SelectMode::Online) {
        localSide_ = services_.online->side();
        services_.online->pickSent = false;
    }
}

void CharacterSelectScene::onEnter() {
    // Back from the stage select or a match: pick again, the cursors stay.
    // Online, a pick the server has is kept (the host's stage went with it).
    picked_ = {};
    stageShown_ = false;
    if (mode_ == SelectMode::Online) picked_[static_cast<std::size_t>(localSide_)] = services_.online->pickSent;
    ready_ = 0;
    services_.data.playTheme(services_.audio, services_.data.menuTheme());
}

const FighterView* CharacterSelectScene::view(int index) const {
    const auto& e = services_.data.roster()[static_cast<std::size_t>(index)];
    const auto it = views_.find(e.id);
    return it == views_.end() ? nullptr : it->second.get();
}

int CharacterSelectScene::sideFor(const MenuEvent& event) const {
    if (mode_ == SelectMode::Online) return localSide_; // any keys steer this player's side
    if (mode_ == SelectMode::TwoPlayers) return event.player;
    if (mode_ == SelectMode::Arcade) return 0;
    // One player picks both, their own side first, then the opponent; either
    // set of keys (or pad) steers.
    return picked_[0] ? 1 : 0;
}

void CharacterSelectScene::pick(int side) {
    const auto s = static_cast<std::size_t>(side);
    if (!view(cursor_[s])) {
        services_.audio.play("menu_back"); // a locked slot
        return;
    }
    picked_[s] = true;
    pickedAt_[s] = clock_;
    services_.audio.play("menu_confirm");
    if (mode_ == SelectMode::Online) {
        const auto& id = services_.data.roster()[static_cast<std::size_t>(cursor_[s])].id;
        auto& online = *services_.online;
        if (online.hosting() && !services_.settings.autoplay) {
            // The host is READY (onlineUpdate tells the guest); once the guest is too,
            // the host goes on to pick the venue, and the stage select sends both.
        } else {
            online.send(net::Pick{id, ""}); // a guest's stage doesn't count; autoplay: a random venue
            online.pickSent = true;
        }
    }
}

void CharacterSelectScene::showMoves(int side) {
    const auto* fighter = view(cursor_[static_cast<std::size_t>(side)]);
    if (!fighter) {
        services_.audio.play("menu_back"); // a locked slot has no moves yet
        return;
    }
    // Written as this side will face at the start of a round, in the keys of whoever picks it.
    // Online, this PC's player plays either side with player 1's keys or pad.
    const bool online = mode_ == SelectMode::Online;
    const int keys = online ? (side == localSide_ ? 0 : -1) : mode_ == SelectMode::TwoPlayers || side == 0 ? side : -1;
    const std::string label = online ? (keys < 0 ? "OPPONENT" : "YOU")
                            : keys < 0 ? "CPU" : side == 0 ? "PLAYER 1" : "PLAYER 2";
    services_.audio.play("menu_confirm");
    services_.scenes.push(std::make_unique<MoveListScene>(
                              services_, std::vector<MoveListPanel>{{&fighter->definition(), side == 0 ? 1 : -1, keys, label}},
                              side == 0 ? "ARROWS AS PLAYER 1 FACES: RIGHT" : "ARROWS AS PLAYER 2 FACES: LEFT"),
                          Transition::Cut);
}

void CharacterSelectScene::unpick(int side) {
    picked_[static_cast<std::size_t>(side)] = false;
    ready_ = 0;
    services_.audio.play("menu_back");
}

void CharacterSelectScene::handleMenu(const MenuEvent& event) {
    if (ready_ > 0) return; // both picked: on to the stage select
    const int side = sideFor(event);
    const auto count = static_cast<int>(services_.data.roster().size());
    if (mode_ == SelectMode::Online && (event.action == MenuAction::Back || event.action == MenuAction::Pause)) {
        // A pick the server has stands; the host's READY can be taken back; before that, Back leaves the room.
        if (services_.online->pickSent) return;
        if (picked_[static_cast<std::size_t>(localSide_)]) return unpick(localSide_);
        services_.audio.play("menu_back");
        services_.online->leaveRoom();
        services_.scenes.pop();
        return;
    }
    if (event.action == MenuAction::Back || event.action == MenuAction::Pause) {
        // Step back: undo this side's pick, then (one player) the first pick, then leave.
        if (side >= 0 && picked_[static_cast<std::size_t>(side)]) return unpick(side);
        if (mode_ != SelectMode::TwoPlayers && picked_[0]) return unpick(0);
        services_.audio.play("menu_back");
        services_.scenes.pop();
        return;
    }
    if (picked_[static_cast<std::size_t>(side)] || ready()) return;
    int& cursor = cursor_[static_cast<std::size_t>(side)];
    const int before = cursor;
    switch (event.action) {
    case MenuAction::Left: cursor = gridMove(cursor, -1, 0, kColumns, count); break;
    case MenuAction::Right: cursor = gridMove(cursor, 1, 0, kColumns, count); break;
    case MenuAction::Up: cursor = gridMove(cursor, 0, -1, kColumns, count); break;
    case MenuAction::Down: cursor = gridMove(cursor, 0, 1, kColumns, count); break;
    case MenuAction::Confirm: pick(side); break;
    case MenuAction::Info: showMoves(side); break;
    default: break;
    }
    if (cursor != before) services_.audio.play("menu_move");
}

void CharacterSelectScene::onlineUpdate() {
    auto& online = *services_.online;
    if (!online.online() || !online.inRoom() || !online.hasOpponent()) {
        services_.scenes.pop(); // the room (or the lobby) takes it from here
        return;
    }
    if (followMatchStart(services_, true)) return;
    const auto& roster = services_.data.roster();
    const auto indexOf = [&roster](const std::string& id) {
        for (std::size_t i = 0; i < roster.size(); ++i)
            if (roster[i].id == id) return static_cast<int>(i);
        return -1;
    };
    // This side's cursor goes to the opponent as it moves (until the pick, which says it all).
    const auto local = static_cast<std::size_t>(localSide_);
    if (!online.pickSent) {
        // Picked but not sent: the host, on the way to the venue (or back from it).
        const auto& id = roster[static_cast<std::size_t>(cursor_[local])].id;
        const bool locked = picked_[local];
        if (id != hoverSent_ || locked != lockedSent_) {
            online.send(net::Hover{id, locked});
            hoverSent_ = id;
            lockedSent_ = locked;
        }
    }
    // The opponent's cursor as it moves, then their pick (a host choosing the venue counts as picked).
    const auto remote = static_cast<std::size_t>(1 - localSide_);
    const auto& member = online.room()->members[remote];
    if (member.picked && !picked_[remote]) {
        if (const int i = indexOf(member.fighter); i >= 0) cursor_[remote] = i;
        picked_[remote] = true;
        pickedAt_[remote] = clock_;
    }
    if (!member.picked) {
        picked_[remote] = false;
        if (const int i = indexOf(member.hover); i >= 0) cursor_[remote] = i;
        if (member.locked && !remoteLocked_) pickedAt_[remote] = clock_;
    }
    remoteSeen_ = member.picked || !member.hover.empty();
    remoteLocked_ = member.locked && !member.picked;
    // The host goes on to the venue only once both are READY.
    if (online.hosting() && !services_.settings.autoplay && picked_[local] && picked_[remote] && !stageShown_) {
        stageShown_ = true;
        FightSetup setup;
        setup.control = Control::Online;
        setup.fighters[local] = roster[static_cast<std::size_t>(cursor_[local])].id;
        services_.scenes.push(std::make_unique<StageSelectScene>(services_, std::move(setup)));
        return;
    }
    // -autoplay (soak tests): pick this side's default fighter after a beat.
    if (services_.settings.autoplay && !picked_[static_cast<std::size_t>(localSide_)] && clock_ > 40) pick(localSide_);
}

void CharacterSelectScene::fixedUpdate() {
    ++clock_;
    if (mode_ == SelectMode::Online) return onlineUpdate();
    if (!ready()) return;
    if (++ready_ < kReadyTicks) return;
    ready_ = kReadyTicks;
    if (mode_ == SelectMode::Arcade) {
        const auto& player = services_.data.roster()[static_cast<std::size_t>(cursor_[0])].id;
        services_.scenes.replaceTop(std::make_unique<ArcadeScene>(services_, player));
        return;
    }
    FightSetup setup;
    setup.control = mode_ == SelectMode::TwoPlayers ? Control::TwoPlayers : Control::VsCpu;
    setup.training = mode_ == SelectMode::Training;
    setup.fromSelect = true;
    setup.cpuLevel = services_.settings.cpuDifficulty + 1; // EASY BRAWLER, NORMAL CONTENDER, HARD VETERAN
    for (std::size_t side = 0; side < 2; ++side)
        setup.fighters[side] = services_.data.roster()[static_cast<std::size_t>(cursor_[side])].id;
    services_.scenes.push(std::make_unique<StageSelectScene>(services_, std::move(setup)));
}

void CharacterSelectScene::drawLadder(SDL_Renderer* r) const {
    // The road to the Crown, for the fighter under the cursor.
    const auto* ladder = services_.data.arcade();
    if (!ladder) return;
    const auto& data = services_.data;
    const auto& player = data.roster()[static_cast<std::size_t>(cursor_[0])];
    const float right = services_.width - 12;
    data.small().draw(r, "THE LADDER", right, 88, ui::kBrassLight, Align::Right, true);
    float y = 104;
    const auto opponents = content::arcadeOpponents(*ladder, player.id, data.playableIds());
    for (std::size_t i = 0; i < ladder->bouts.size(); ++i) {
        const auto& bout = ladder->bouts[i];
        const auto* entry = data.fighter(opponents[i]);
        const auto& stage = data.stages()[data.stageIndex(bout.stage)];
        const std::string who = bout.boss ? "???" : player.playable() && entry ? ui::upper(entry->name) : "?";
        data.small().draw(r, std::to_string(i + 1) + ". " + ui::upper(stage.name), right, y,
                          bout.final ? ui::kCoralLight : ui::kChalk, Align::Right, true);
        data.small().draw(r, "VS " + who, right, y + 9, ui::kFog, Align::Right, true);
        y += 24;
    }
}

SDL_FRect CharacterSelectScene::cellRect(int index) const {
    const float gridW = kColumns * kCell + (kColumns - 1) * kGap;
    const float x0 = std::floor((services_.width - gridW) / 2);
    const float y0 = 168;
    const int column = index % kColumns, row = index / kColumns;
    return {x0 + static_cast<float>(column * (kCell + kGap)), y0 + static_cast<float>(row * (kCell + kGap)),
            static_cast<float>(kCell), static_cast<float>(kCell)};
}

void CharacterSelectScene::drawGrid(SDL_Renderer* r) const {
    const auto& data = services_.data;
    const auto& roster = data.roster();
    for (int i = 0; i < static_cast<int>(roster.size()); ++i) {
        const SDL_FRect cell = cellRect(i);
        ui::frame(r, cell);
        const auto* v = view(i);
        if (v) {
            ui::fill(r, cell.x, cell.y, cell.w, cell.h, ui::kNight);
            // A drawn 48x48 select portrait, or the 32x32 HUD portrait scaled up.
            const SDL_FRect portrait{cell.x + 4, cell.y + 4, 48, 48};
            const Texture& image = v->selectPortrait() ? *v->selectPortrait() : v->portrait();
            SDL_RenderTexture(r, image.handle.get(), nullptr, &portrait);
        } else {
            ui::fill(r, cell.x, cell.y, cell.w, cell.h, ui::kInk);
            data.large().draw(r, "?", cell.x + cell.w / 2, cell.y + 10, {90, 103, 130, 255}, Align::Centre);
            data.small().draw(r, "SOON", cell.x + cell.w / 2, cell.y + cell.h - 12, ui::kSteel, Align::Centre);
        }
    }
    // Cursors: both on one cell take turns blinking.
    for (int side = 0; side < 2; ++side) {
        const auto s = static_cast<std::size_t>(side);
        if (side == 1 && mode_ == SelectMode::Arcade) continue;
        const bool shared = cursor_[0] == cursor_[1];
        // Online, the opponent's cursor shows once the server has relayed it.
        if (mode_ == SelectMode::Online && side != localSide_ && !remoteSeen_) continue;
        if (shared && !picked_[0] && !picked_[1] && (clock_ / 12) % 2 != side) continue;
        if (picked_[s] && (clock_ - pickedAt_[s]) < 20 && (clock_ / 3) % 2 == 0) continue; // flash on pick
        const SDL_FRect cell = cellRect(cursor_[s]);
        const float inset = shared && picked_[0] && picked_[1] ? static_cast<float>(side * 3) : 0;
        ui::outline(r, {cell.x - 2 + inset, cell.y - 2 + inset, cell.w + 4 - 2 * inset, cell.h + 4 - 2 * inset},
                    ui::kSide[side], 2);
        const bool cpu = mode_ != SelectMode::TwoPlayers && mode_ != SelectMode::Online && side == 1;
        const std::string tag = mode_ == SelectMode::Online ? (side == localSide_ ? "YOU" : "THEM")
                                : cpu                       ? (mode_ == SelectMode::Training ? "DUMMY" : "CPU")
                                : side == 0                 ? "1P"
                                                            : "2P";
        const float tagX = side == 0 ? cell.x : cell.x + cell.w;
        services_.data.small().draw(r, tag, tagX, cell.y - 12, ui::kSide[side], side == 0 ? Align::Left : Align::Right,
                                    true);
    }
}

void CharacterSelectScene::drawSide(SDL_Renderer* r, int side) const {
    const auto s = static_cast<std::size_t>(side);
    const auto& data = services_.data;
    const auto& entry = data.roster()[static_cast<std::size_t>(cursor_[s])];
    const float centre = side == 0 ? 112.0f : services_.width - 112.0f;
    const Align align = side == 0 ? Align::Left : Align::Right;
    const float edge = side == 0 ? 12.0f : services_.width - 12.0f;

    // The one-player flow shows the opponent's side only once it is being chosen.
    if (mode_ == SelectMode::Online && side != localSide_ && !remoteSeen_) {
        const auto& room = services_.online->room();
        const auto name =
            room && room->members.size() == 2 ? ui::upper(room->members[s].name) : std::string("OPPONENT");
        data.small().draw(r, name, centre, 186, ui::kSide[side], Align::Centre, true);
        if ((clock_ / 30) % 2 == 0) data.small().draw(r, "IS CHOOSING...", centre, 198, ui::kFog, Align::Centre, true);
        return;
    }
    const bool waiting = mode_ != SelectMode::TwoPlayers && mode_ != SelectMode::Online && side == 1 && !picked_[0];
    if (waiting) {
        if ((clock_ / 30) % 2 == 0)
            data.small().draw(r, mode_ == SelectMode::Training ? "THEN THE DUMMY" : "THEN THE OPPONENT", centre, 200,
                              ui::kFog, Align::Centre, true);
        return;
    }
    if (const auto* v = view(cursor_[s])) {
        const auto& def = v->definition();
        const bool locked = mode_ == SelectMode::Online && side != localSide_ && remoteLocked_;
        const bool won = (picked_[s] || locked) && def.animations.count("win");
        const auto& animation = def.animation(won ? "win" : "idle");
        const int tick = won ? std::min(clock_ - pickedAt_[s], animation.totalTicks() - 1) : clock_;
        v->sheet().draw(r, animation.at(tick).frame, centre, 262, side == 1);
    } else {
        data.large().draw(r, "?", centre, 150, ui::kSteel, Align::Centre);
    }
    ui::fill(r, side == 0 ? 0 : services_.width - 226, 268, 226, 1, ui::kBrass);
    data.large().draw(r, ui::upper(entry.name), edge, 274, picked_[s] ? ui::kWhite : ui::kChalk, align);
    data.small().draw(r, ui::upper(entry.role), edge, 300, ui::kSide[side], align, true);
    float y = 311;
    for (const auto& line : ui::wrap(data.small(), ui::upper(entry.identity), 238)) {
        if (y > 330) break;
        data.small().draw(r, line, edge, y, ui::kFog, align, true);
        y += 9;
    }
}

void CharacterSelectScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    const SDL_FRect all{0, 0, services_.width, services_.height};
    SDL_RenderTexture(r, data.backdrop().handle.get(), nullptr, &all);
    if (hostChoosingVenue()) return drawVenueWait(r);
    data.large().draw(r, "SELECT FIGHTER", services_.width / 2, 14, ui::kWhite, Align::Centre);
    data.small().draw(r, title(mode_), services_.width / 2, 42, ui::kBrassLight, Align::Centre, true);
    drawSide(r, 0);
    if (mode_ == SelectMode::Arcade) drawLadder(r);
    else drawSide(r, 1);
    drawGrid(r);
    if (mode_ == SelectMode::Online) {
        drawReady(r, 0);
        drawReady(r, 1);
    }
    if (mode_ == SelectMode::Online && picked_[static_cast<std::size_t>(localSide_)] && !ready()) {
        const auto& room = services_.online->room();
        const auto remote = static_cast<std::size_t>(1 - localSide_);
        const std::string name = room && room->members.size() == 2 ? ui::upper(room->members[remote].name) : "";
        const std::string line = name.empty() ? "WAITING FOR THE OPPONENT" : "WAITING FOR " + name + " TO BE READY";
        data.small().draw(r, line, services_.width / 2, 346, ui::kFog, Align::Centre, true);
        if (!services_.online->pickSent)
            data.small().draw(r, "LIGHT KICK: NOT READY", services_.width / 2, 334, ui::kSteel, Align::Centre, true);
    } else if (ready() && mode_ != SelectMode::Arcade) {
        if ((clock_ / 6) % 2 == 0) data.large().draw(r, "VS", services_.width / 2, 110, ui::kBrassLight, Align::Centre);
    } else {
        data.small().draw(r, "LIGHT PUNCH: PICK - LIGHT KICK: BACK - HEAVY PUNCH: MOVE LIST", services_.width / 2, 346,
                          ui::kFog, Align::Centre, true);
    }
}

bool CharacterSelectScene::hostChoosingVenue() const {
    // The guest has picked and the host is READY: the host is on the stage select.
    return mode_ == SelectMode::Online && !services_.online->hosting() &&
           picked_[static_cast<std::size_t>(localSide_)] && remoteLocked_;
}

void CharacterSelectScene::drawReady(SDL_Renderer* r, int side) const {
    const auto s = static_cast<std::size_t>(side);
    const bool ready = picked_[s] || (side != localSide_ && remoteLocked_);
    if (!ready) return;
    // A READY badge over the side's fighter, in the side's colour.
    const float centre = side == 0 ? 112.0f : services_.width - 112.0f;
    const auto& font = services_.data.small();
    const float w = static_cast<float>(font.measure("READY")) + 12;
    const SDL_FRect box{centre - w / 2, 70, w, 13};
    ui::fill(r, box.x, box.y, box.w, box.h, ui::kSide[side]);
    font.draw(r, "READY", centre, 73, ui::kInk, Align::Centre);
}

void CharacterSelectScene::drawVenueWait(SDL_Renderer* r) const {
    // The guest's screen while the host picks the venue: both fighters, READY, and what is going on.
    const auto& data = services_.data;
    const auto& room = services_.online->room();
    const std::string host =
        room && !room->members.empty() ? ui::upper(room->members[0].name) : std::string("THE HOST");
    data.large().draw(r, "GET READY", services_.width / 2, 14, ui::kWhite, Align::Centre);
    data.small().draw(r, "ONLINE", services_.width / 2, 42, ui::kBrassLight, Align::Centre, true);
    drawSide(r, 0);
    drawSide(r, 1);
    drawReady(r, 0);
    drawReady(r, 1);
    if ((clock_ / 6) % 2 == 0) data.large().draw(r, "VS", services_.width / 2, 120, ui::kBrassLight, Align::Centre);
    const SDL_FRect panel{services_.width / 2 - 96, 168, 192, 44};
    ui::fill(r, panel.x, panel.y, panel.w, panel.h, ui::kNight);
    ui::frame(r, panel);
    data.small().draw(r, host, services_.width / 2, 178, ui::kSide[0], Align::Centre, true);
    const int dots = (clock_ / 20) % 4;
    // Left-aligned, so the dots grow without the line moving.
    const float left = services_.width / 2 - static_cast<float>(data.small().measure("IS CHOOSING THE STAGE")) / 2;
    data.small().draw(r, "IS CHOOSING THE STAGE" + std::string(static_cast<std::size_t>(dots), '.'), left, 192,
                      ui::kChalk, Align::Left, true);
    data.small().draw(r, "THE FIGHT STARTS WHEN THE STAGE IS PICKED", services_.width / 2, 346, ui::kFog, Align::Centre,
                      true);
}

std::string CharacterSelectScene::status() const {
    const auto& roster = services_.data.roster();
    return std::string("select: ") + title(mode_) + " | " + roster[static_cast<std::size_t>(cursor_[0])].id +
           (picked_[0] ? "*" : "") + " vs " + roster[static_cast<std::size_t>(cursor_[1])].id + (picked_[1] ? "*" : "");
}

} // namespace iron
