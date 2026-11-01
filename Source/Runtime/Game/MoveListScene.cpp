#include "Game/MoveListScene.h"

#include "Game/FightScene.h"
#include "Render/Ui.h"

#include <cmath>

namespace iron {
namespace {

constexpr int kLP = 4, kHP = 5, kLK = 6, kHK = 7, kGuard = 8; // InputBindings action indices
constexpr const char* kButtonNames[]{"", "", "", "", "LP", "HP", "LK", "HK", "GUARD"};

// "236P" -> ("236", "P").
std::pair<std::string, std::string> splitInput(const std::string& input) {
    std::size_t i = 0;
    while (i < input.size() && input[i] >= '0' && input[i] <= '9') ++i;
    return {input.substr(0, i), input.substr(i)};
}

} // namespace

MoveListScene::MoveListScene(Services& services, std::vector<MoveListPanel> panels, std::string footer)
    : services_(services), panels_(std::move(panels)), footer_(std::move(footer)) {}

MoveListScene::MoveListScene(Services& services, const FightScene& fight)
    : MoveListScene(services, {}, "ARROWS AS EACH SIDE FACES NOW - HOLD BACK OR GUARD TO BLOCK") {
    for (int side = 0; side < 2; ++side) {
        const bool human = fight.human(side);
        panels_.push_back({&fight.view(side).definition(), fight.facing(side), human ? side : -1,
                           human ? (side == 0 ? "PLAYER 1" : "PLAYER 2") : "CPU"});
    }
}

std::string MoveListScene::status() const {
    std::string s = "move list:";
    for (const auto& p : panels_) s += " " + p.fighter->id;
    return s;
}

void MoveListScene::handleMenu(const MenuEvent& event) {
    if (event.action == MenuAction::Confirm || event.action == MenuAction::Back || event.action == MenuAction::Pause ||
        event.action == MenuAction::Info) {
        services_.audio.play("menu_back");
        services_.scenes.pop(Transition::Cut);
    }
}

std::string MoveListScene::button(const MoveListPanel& panel, int action) const {
    if (panel.player < 0) return kButtonNames[action];
    return services_.controls.label(panel.player, action); // keys or pad, whichever was used last
}

void MoveListScene::drawPanel(SDL_Renderer* r, const MoveListPanel& panel, int colour, float x) const {
    const auto& data = services_.data;
    const auto& font = data.small();
    const auto& def = *panel.fighter;
    const int facing = panel.facing;
    const float width = 296, right = x + width;
    const auto key = [&](int action) { return button(panel, action); };

    ui::frame(r, {x, 44, width, 270});
    ui::fill(r, x, 44, width, 270, ui::kNight);
    font.draw(r, def.displayName, x + 6, 50, ui::kSide[colour], Align::Left, true);
    font.draw(r, panel.label, right - 6, 50, ui::kFog, Align::Right, true);
    ui::fill(r, x + 4, 61, width - 8, 1, ui::kBrass);

    float y = 68;
    const auto row = [&](const std::string& name, const std::string& tags, const std::string& input, bool special) {
        font.draw(r, ui::upper(name), x + 6, y, special ? ui::kBrassLight : ui::kChalk, Align::Left, true);
        font.draw(r, input, right - 6, y, special ? ui::kWhite : ui::kChalk, Align::Right, true);
        if (!tags.empty()) font.draw(r, tags, x + 12, y + 9, ui::kFog, Align::Left, true);
        y += tags.empty() ? 14.0f : 22.0f;
    };
    constexpr int slotAction[]{kLP, kHP, kLK, kHK};
    for (int slot = 0; slot < sim::kSlotCount; ++slot) {
        const auto& info = def.moves[static_cast<std::size_t>(slot)];
        const auto& m = def.spec.moves[static_cast<std::size_t>(slot)];
        if (!m.enabled) continue;
        std::string tags;
        const auto tag = [&tags](const char* t) { tags += (tags.empty() ? "" : ", ") + std::string(t); };
        if (m.low) tag("LOW");
        if (m.knockdown) tag("KNOCKDOWN");
        if (m.kind == sim::MoveKind::Throw) tag(m.techable ? "THROW" : "COMMAND THROW, NO TECH");
        std::string input;
        if (slot == sim::SpecialSlot) {
            const auto [motion, buttons] = splitInput(info.input);
            const std::string punch = key(kLP) + "/" + key(kHP);
            const std::string kick = key(kLK) + "/" + key(kHK);
            const std::string press = buttons == "P" ? punch : buttons == "K" ? kick
                                    : buttons == "LP" ? key(kLP) : buttons == "HP" ? key(kHP)
                                    : buttons == "LK" ? key(kLK) : key(kHK);
            input = ui::motionArrows(motion, facing) + " + " + press;
        } else if (slot == sim::ThrowSlot) {
            input = key(kLP) + " + " + key(kLK);
            tags = tags.empty() ? "CLOSE" : "CLOSE, " + tags;
        } else {
            input = key(slotAction[slot]);
        }
        row(info.name, tags, input, slot == sim::SpecialSlot);
    }
    ui::fill(r, x + 4, y + 2, width - 8, 1, ui::kSteel);
    y += 8;
    row("Throw tech", "WHEN GRABBED", key(kLP) + " + " + key(kLK), false);
    row("Breakaway", "50 HEAT, WHILE GUARDING",
        key(kGuard) + " + " + key(kHP) + " + " + key(kHK), false);
}

void MoveListScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    ui::dim(r, services_.width, services_.height, 200);
    ui::fill(r, 0, 0, services_.width, 40, {7, 9, 15, 230}); // the title sits over whatever title is behind
    data.large().draw(r, "MOVE LIST", services_.width / 2, 12, ui::kWhite, Align::Centre);
    if (panels_.size() == 1) {
        drawPanel(r, panels_[0], panels_[0].facing < 0 ? 1 : 0, std::floor((services_.width - 296) / 2));
    } else {
        for (std::size_t i = 0; i < panels_.size() && i < 2; ++i)
            drawPanel(r, panels_[i], static_cast<int>(i), i == 0 ? 16 : services_.width - 16 - 296);
    }
    data.small().draw(r, footer_, services_.width / 2, 324, ui::kFog, Align::Centre, true);
    data.small().draw(r, "ANY BUTTON TO RETURN", services_.width / 2, 340, ui::kFog, Align::Centre, true);
}

} // namespace iron
