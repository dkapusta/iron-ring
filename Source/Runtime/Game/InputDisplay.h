#pragma once

#include "Render/BitmapFont.h"
#include "Simulation/Round.h"

#include <array>
#include <deque>

namespace iron {

// Training: each player's recent inputs, newest on top, as numpad direction
// (relative to facing: 6 = forward, 2 = down) plus buttons and how many ticks
// they were held, e.g. "12 3 LP". A new row starts whenever the input changes.
class InputDisplay {
public:
    explicit InputDisplay(const BitmapFont& font) : font_(font) {}

    // Once per simulated tick, with the inputs the simulation received.
    void record(const sim::Round& round, const std::array<sim::Input, 2>& inputs);
    void clear();
    void draw(SDL_Renderer* r) const;

private:
    struct Row {
        int direction = 5;
        sim::Input buttons = 0;
        int ticks = 0;
    };
    static constexpr std::size_t kRows = 16;

    const BitmapFont& font_;
    std::array<std::deque<Row>, 2> history_;
};

} // namespace iron
