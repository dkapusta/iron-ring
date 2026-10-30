#include "Core/Menu.h"

#include <algorithm>
#include <cmath>

namespace iron {

const std::string& MenuItem::value() const {
    static const std::string none;
    if (choices.empty()) return none;
    return choices.at(static_cast<std::size_t>(std::clamp(choice, 0, static_cast<int>(choices.size()) - 1)));
}

MenuItem MenuItem::action(std::string id, std::string label, bool enabled) {
    MenuItem item;
    item.id = std::move(id);
    item.label = std::move(label);
    item.enabled = enabled;
    return item;
}

MenuItem MenuItem::setting(std::string id, std::string label, std::vector<std::string> choices, int choice) {
    MenuItem item = action(std::move(id), std::move(label));
    item.choices = std::move(choices);
    item.choice = choice;
    return item;
}

MenuItem MenuItem::level(std::string id, std::string label, int steps, int choice) {
    MenuItem item = setting(std::move(id), std::move(label), levelChoices(steps), choice);
    item.wrap = false;
    return item;
}

MenuItem& Menu::add(MenuItem item) {
    items_.push_back(std::move(item));
    // Keep the focus off a disabled first item.
    if (!items_[static_cast<std::size_t>(focus_)].enabled) move(1);
    return items_.back();
}

MenuItem* Menu::find(std::string_view id) {
    for (auto& item : items_)
        if (item.id == id) return &item;
    return nullptr;
}

const MenuItem* Menu::find(std::string_view id) const {
    for (const auto& item : items_)
        if (item.id == id) return &item;
    return nullptr;
}

bool Menu::move(int delta) {
    const int count = static_cast<int>(items_.size());
    if (count == 0 || delta == 0) return false;
    const int step = delta > 0 ? 1 : -1;
    int index = focus_;
    for (int tries = 0; tries < count; ++tries) {
        index = ((index + step) % count + count) % count;
        if (items_[static_cast<std::size_t>(index)].enabled) break;
    }
    if (!items_[static_cast<std::size_t>(index)].enabled || index == focus_) return false;
    focus_ = index;
    return true;
}

bool Menu::adjust(int delta) {
    if (items_.empty()) return false;
    auto& item = items_[static_cast<std::size_t>(focus_)];
    const int count = static_cast<int>(item.choices.size());
    if (count < 2 || !item.enabled || delta == 0) return false;
    const int next = item.wrap ? ((item.choice + delta) % count + count) % count
                               : std::clamp(item.choice + delta, 0, count - 1);
    if (next == item.choice) return false;
    item.choice = next;
    return true;
}

void Menu::focusOn(std::string_view id) {
    for (std::size_t i = 0; i < items_.size(); ++i)
        if (items_[i].id == id && items_[i].enabled) focus_ = static_cast<int>(i);
}

int gridMove(int index, int dx, int dy, int columns, int count) {
    if (count <= 0 || columns <= 0) return 0;
    const int rows = (count + columns - 1) / columns;
    int column = index % columns;
    int row = index / columns;
    const auto cellsInRow = [&](int r) { return std::min(columns, count - r * columns); };
    if (dx != 0) {
        const int width = cellsInRow(row);
        column = ((column + dx) % width + width) % width;
    }
    if (dy != 0) {
        // Rows that don't reach this column are skipped.
        for (int tries = 0; tries < rows; ++tries) {
            row = ((row + dy) % rows + rows) % rows;
            if (column < cellsInRow(row)) break;
        }
    }
    return row * columns + column;
}

std::vector<std::string> levelChoices(int steps) {
    std::vector<std::string> choices;
    for (int i = 0; i <= steps; ++i) choices.push_back(std::to_string(i));
    return choices;
}

int levelIndex(float value, int steps) {
    return std::clamp(static_cast<int>(std::lround(value * static_cast<float>(steps))), 0, steps);
}

float levelValue(int index, int steps) { return static_cast<float>(index) / static_cast<float>(steps); }

} // namespace iron
