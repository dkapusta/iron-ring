#include "Game/MoveListScene.h"

#include "Game/FightScene.h"
#include "Render/Ui.h"

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

MoveListScene::MoveListScene(Services& services, const FightScene& fight) : services_(services), fight_(fight) {}

void MoveListScene::handleMenu(const MenuEvent& event) {
    if (event.action == MenuAction::Confirm || event.action == MenuAction::Back || event.action == MenuAction::Pause) {
        services_.audio.play("menu_back");
        services_.scenes.pop(Transition::Cut);
    }
}

std::string MoveListScene::button(int side, int action) const {
    if (!fight_.human(side)) return kButtonNames[action];
    return services_.controls.keyLabel(side, action);
}

void MoveListScene::drawSide(SDL_Renderer* r, int side, float x) const {
    const auto& data = services_.data;
    const auto& font = data.small();
    const auto& def = fight_.view(side).definition();
    const int facing = fight_.facing(side);
    const float width = 296, right = x + width;

    ui::frame(r, {x, 44, width, 270});
    ui::fill(r, x, 44, width, 270, ui::kNight);
    font.draw(r, def.displayName, x + 6, 50, ui::kSide[side], Align::Left, true);
    font.draw(r, fight_.human(side) ? (side == 0 ? "PLAYER 1" : "PLAYER 2") : "CPU", right - 6, 50, ui::kFog,
              Align::Right, true);
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
            const std::string punch = button(side, kLP) + "/" + button(side, kHP);
            const std::string kick = button(side, kLK) + "/" + button(side, kHK);
            const std::string press = buttons == "P" ? punch : buttons == "K" ? kick
                                    : buttons == "LP" ? button(side, kLP) : buttons == "HP" ? button(side, kHP)
                                    : buttons == "LK" ? button(side, kLK) : button(side, kHK);
            input = ui::motionArrows(motion, facing) + " + " + press;
        } else if (slot == sim::ThrowSlot) {
            input = button(side, kLP) + " + " + button(side, kLK);
            tags = tags.empty() ? "CLOSE" : "CLOSE, " + tags;
        } else {
            input = button(side, slotAction[slot]);
        }
        row(info.name, tags, input, slot == sim::SpecialSlot);
    }
    ui::fill(r, x + 4, y + 2, width - 8, 1, ui::kSteel);
    y += 8;
    row("Throw tech", "WHEN GRABBED", button(side, kLP) + " + " + button(side, kLK), false);
    row("Breakaway", "50 HEAT, WHILE GUARDING",
        button(side, kGuard) + " + " + button(side, kHP) + " + " + button(side, kHK), false);
}

void MoveListScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    ui::dim(r, services_.width, services_.height, 200);
    data.large().draw(r, "MOVE LIST", services_.width / 2, 12, ui::kWhite, Align::Centre);
    drawSide(r, 0, 16);
    drawSide(r, 1, services_.width - 16 - 296);
    data.small().draw(r, "ARROWS AS EACH SIDE FACES NOW - HOLD BACK OR GUARD TO BLOCK", services_.width / 2, 324,
                      ui::kFog, Align::Centre, true);
    data.small().draw(r, "ANY BUTTON TO RETURN", services_.width / 2, 340, ui::kFog, Align::Centre, true);
}

} // namespace iron
