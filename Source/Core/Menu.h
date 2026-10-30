#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace iron {

// One line of a menu: an action (no choices) or a setting with a list of
// choices that Left/Right step through.
struct MenuItem {
    std::string id;
    std::string label;
    std::vector<std::string> choices; // empty: an action, chosen with Confirm
    int choice = 0;
    bool enabled = true;
    bool wrap = true;                 // false for levels: stop at the ends instead of wrapping
    std::string note;                 // a hint shown while the item is focused

    static MenuItem action(std::string id, std::string label, bool enabled = true);
    static MenuItem setting(std::string id, std::string label, std::vector<std::string> choices, int choice);
    static MenuItem level(std::string id, std::string label, int steps, int choice); // no wrap

    [[nodiscard]] bool isAction() const { return choices.empty(); }
    [[nodiscard]] const std::string& value() const;
};

// Focus and value logic of a vertical menu, independent of drawing and input,
// so the front-end scenes share it and tests can drive it.
class Menu {
public:
    MenuItem& add(MenuItem item);
    [[nodiscard]] const std::vector<MenuItem>& items() const { return items_; }
    [[nodiscard]] int focus() const { return focus_; }
    [[nodiscard]] const MenuItem& focused() const { return items_.at(static_cast<std::size_t>(focus_)); }
    [[nodiscard]] MenuItem* find(std::string_view id);
    [[nodiscard]] const MenuItem* find(std::string_view id) const;

    // Moves the focus by delta, skipping disabled items and wrapping. True if it moved.
    bool move(int delta);
    // Steps the focused setting's choice. True if the value changed.
    bool adjust(int delta);
    void focusOn(std::string_view id);

private:
    std::vector<MenuItem> items_;
    int focus_ = 0;
};

// Cursor movement on a grid of `count` cells, `columns` wide, filled row by
// row: wraps within the row (dx) and the column (dy). Cells that are missing
// in a short last row are skipped.
[[nodiscard]] int gridMove(int index, int dx, int dy, int columns, int count);

// A choice list of levels "0".."steps" for a 0..1 value, and back.
[[nodiscard]] std::vector<std::string> levelChoices(int steps);
[[nodiscard]] int levelIndex(float value, int steps);
[[nodiscard]] float levelValue(int index, int steps);

} // namespace iron
