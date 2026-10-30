#include "Game/MenuView.h"

#include "Render/Ui.h"

namespace iron {

MenuResult navigate(Menu& menu, const MenuEvent& event, AudioSystem& audio) {
    switch (event.action) {
    case MenuAction::Up:
    case MenuAction::Down:
        if (!menu.move(event.action == MenuAction::Up ? -1 : 1)) return MenuResult::None;
        audio.play("menu_move");
        return MenuResult::Moved;
    case MenuAction::Left:
    case MenuAction::Right:
        if (!menu.adjust(event.action == MenuAction::Left ? -1 : 1)) return MenuResult::None;
        audio.play("menu_move");
        return MenuResult::Changed;
    case MenuAction::Confirm:
        if (menu.items().empty()) return MenuResult::None;
        if (!menu.focused().isAction()) { // Confirm on a setting steps it forward
            if (!menu.adjust(1)) return MenuResult::None;
            audio.play("menu_move");
            return MenuResult::Changed;
        }
        audio.play("menu_confirm");
        return MenuResult::Chosen;
    case MenuAction::Back:
    case MenuAction::Pause:
        audio.play("menu_back");
        return MenuResult::Back;
    }
    return MenuResult::None;
}

void drawMenu(SDL_Renderer* r, const Menu& menu, const BitmapFont& font, const MenuStyle& style, int blink) {
    const float lineHeight = static_cast<float>(font.lineHeight());
    float y = style.y;
    const float left = style.x - style.width / 2;
    const float right = style.x + style.width / 2;
    for (int i = 0; i < static_cast<int>(menu.items().size()); ++i) {
        const auto& item = menu.items()[static_cast<std::size_t>(i)];
        const bool focused = i == menu.focus();
        if (focused) {
            ui::fill(r, left - 6, y - 3, style.width + 12, lineHeight + 5, ui::kInk);
            ui::fill(r, left - 5, y - 2, style.width + 10, lineHeight + 3, ui::kSteel);
            ui::fill(r, left - 5, y - 2, 2, lineHeight + 3, ui::kBrassLight);
        }
        const SDL_Color colour = !item.enabled ? ui::kSteel : focused ? ui::kBrassLight : ui::kChalk;
        if (item.isAction()) {
            font.draw(r, item.label, style.x, y, colour, Align::Centre, true);
        } else {
            font.draw(r, item.label, left, y, colour, Align::Left, true);
            const bool arrows = focused && (blink / 16) % 2 == 0;
            const std::string value = arrows ? "< " + item.value() + " >" : "  " + item.value() + "  ";
            font.draw(r, value, right, y, focused ? ui::kChalk : ui::kFog, Align::Right, true);
        }
        y += style.spacing;
    }
    if (!menu.items().empty() && !menu.focused().note.empty())
        font.draw(r, menu.focused().note, style.x, y + style.noteGap, ui::kTealLight, Align::Centre, true);
}

} // namespace iron
