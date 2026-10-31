#include "Game/CharacterSelectScene.h"

#include "Game/ArcadeScene.h"
#include "Game/MenuView.h"
#include "Game/MoveListScene.h"
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
}

void CharacterSelectScene::onEnter() {
    // Back from the stage select or a match: pick again, the cursors stay.
    picked_ = {};
    ready_ = 0;
    services_.data.playTheme(services_.audio, services_.data.menuTheme());
}

const FighterView* CharacterSelectScene::view(int index) const {
    const auto& e = services_.data.roster()[static_cast<std::size_t>(index)];
    const auto it = views_.find(e.id);
    return it == views_.end() ? nullptr : it->second.get();
}

int CharacterSelectScene::sideFor(const MenuEvent& event) const {
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
}

void CharacterSelectScene::showMoves(int side) {
    const auto* fighter = view(cursor_[static_cast<std::size_t>(side)]);
    if (!fighter) {
        services_.audio.play("menu_back"); // a locked slot has no moves yet
        return;
    }
    // Written as this side will face at the start of a round, in the keys of whoever picks it.
    const int keys = mode_ == SelectMode::TwoPlayers || side == 0 ? side : -1;
    const std::string label = keys < 0 ? "CPU" : side == 0 ? "PLAYER 1" : "PLAYER 2";
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

void CharacterSelectScene::fixedUpdate() {
    ++clock_;
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
        if (shared && !picked_[0] && !picked_[1] && (clock_ / 12) % 2 != side) continue;
        if (picked_[s] && (clock_ - pickedAt_[s]) < 20 && (clock_ / 3) % 2 == 0) continue; // flash on pick
        const SDL_FRect cell = cellRect(cursor_[s]);
        const float inset = shared && picked_[0] && picked_[1] ? static_cast<float>(side * 3) : 0;
        ui::outline(r, {cell.x - 2 + inset, cell.y - 2 + inset, cell.w + 4 - 2 * inset, cell.h + 4 - 2 * inset},
                    ui::kSide[side], 2);
        const bool cpu = mode_ != SelectMode::TwoPlayers && side == 1;
        const std::string tag = cpu ? (mode_ == SelectMode::Training ? "DUMMY" : "CPU") : side == 0 ? "1P" : "2P";
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
    const bool waiting = mode_ != SelectMode::TwoPlayers && side == 1 && !picked_[0];
    if (waiting) {
        if ((clock_ / 30) % 2 == 0)
            data.small().draw(r, mode_ == SelectMode::Training ? "THEN THE DUMMY" : "THEN THE OPPONENT", centre, 200,
                              ui::kFog, Align::Centre, true);
        return;
    }
    if (const auto* v = view(cursor_[s])) {
        const auto& def = v->definition();
        const bool won = picked_[s] && def.animations.count("win");
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
    data.large().draw(r, "SELECT FIGHTER", services_.width / 2, 14, ui::kWhite, Align::Centre);
    data.small().draw(r, title(mode_), services_.width / 2, 42, ui::kBrassLight, Align::Centre, true);
    drawSide(r, 0);
    if (mode_ == SelectMode::Arcade) drawLadder(r);
    else drawSide(r, 1);
    drawGrid(r);
    if (ready() && mode_ != SelectMode::Arcade) {
        if ((clock_ / 6) % 2 == 0) data.large().draw(r, "VS", services_.width / 2, 110, ui::kBrassLight, Align::Centre);
    } else {
        data.small().draw(r, "LIGHT PUNCH: PICK - LIGHT KICK: BACK - HEAVY PUNCH: MOVE LIST", services_.width / 2, 346,
                          ui::kFog, Align::Centre, true);
    }
}

std::string CharacterSelectScene::status() const {
    const auto& roster = services_.data.roster();
    return std::string("select: ") + title(mode_) + " | " + roster[static_cast<std::size_t>(cursor_[0])].id +
           (picked_[0] ? "*" : "") + " vs " + roster[static_cast<std::size_t>(cursor_[1])].id + (picked_[1] ? "*" : "");
}

} // namespace iron
