#include "Game/InputDisplay.h"

#include <string>
#include <utility>

namespace iron {
namespace {

constexpr sim::Input kButtons = sim::LightPunch | sim::HeavyPunch | sim::LightKick | sim::HeavyKick | sim::Guard;

std::string describe(int direction, sim::Input buttons) {
    std::string text(1, static_cast<char>('0' + direction));
    constexpr std::pair<sim::Input, const char*> names[]{{sim::LightPunch, "LP"}, {sim::HeavyPunch, "HP"},
                                                         {sim::LightKick, "LK"},  {sim::HeavyKick, "HK"},
                                                         {sim::Guard, "G"}};
    bool first = true;
    for (const auto& [bit, name] : names) {
        if (!(buttons & bit)) continue;
        text += first ? " " : "+";
        text += name;
        first = false;
    }
    return text;
}

} // namespace

void InputDisplay::record(const sim::Round& round, const std::array<sim::Input, 2>& inputs) {
    for (std::size_t side = 0; side < 2; ++side) {
        const int direction = sim::numpadDirection(inputs[side], round.fighters[side].facing);
        const sim::Input buttons = inputs[side] & kButtons;
        auto& rows = history_[side];
        if (!rows.empty() && rows.front().direction == direction && rows.front().buttons == buttons) {
            if (rows.front().ticks < 99) ++rows.front().ticks;
            continue;
        }
        rows.push_front({direction, buttons, 1});
        if (rows.size() > kRows) rows.pop_back();
    }
}

void InputDisplay::clear() {
    for (auto& rows : history_) rows.clear();
}

void InputDisplay::draw(SDL_Renderer* r) const {
    constexpr float top = 72, lineHeight = 9;
    constexpr SDL_Color newest{241, 234, 216, 255}, older{138, 147, 168, 255};
    for (std::size_t side = 0; side < 2; ++side) {
        float y = top;
        for (std::size_t i = 0; i < history_[side].size(); ++i) {
            const auto& row = history_[side][i];
            const std::string line = std::to_string(row.ticks) + " " + describe(row.direction, row.buttons);
            const float x = side == 0 ? 6.0f : 634.0f;
            font_.draw(r, line, x, y, i == 0 ? newest : older, side == 0 ? Align::Left : Align::Right, true);
            y += lineHeight;
        }
    }
}

} // namespace iron
