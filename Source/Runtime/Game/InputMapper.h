#pragma once

#include "Simulation/Round.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

#include <array>
#include <vector>

namespace iron {

// Keyboard -> simulation input bits, configured by ProjectSettings/InputManager.json.
// Presses that start and end between two simulation ticks are latched so a
// quick tap is never lost.
class InputMapper {
public:
    explicit InputMapper(const nlohmann::json& inputManager);

    void onKeyDown(const SDL_KeyboardEvent& key);
    [[nodiscard]] sim::Input sample(int player) const;
    void endTick();

private:
    struct Binding {
        SDL_Scancode scancode;
        sim::Input bit;
    };
    std::array<std::vector<Binding>, 2> bindings_;
    std::array<sim::Input, 2> latched_{};
};

} // namespace iron
