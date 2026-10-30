#pragma once

#include "Simulation/Round.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace iron {

// What menus react to. Pause is Escape or a pad's Start: it opens the pause
// menu in a fight and backs out of menus.
enum class MenuAction { Up, Down, Left, Right, Confirm, Back, Pause };

struct MenuEvent {
    int player = 0; // 0 or 1: whose keys or pad it came from
    MenuAction action = MenuAction::Confirm;
};

// Keyboard and gamepads -> simulation input bits and menu actions, configured
// by ProjectSettings/InputManager.json. Both players have keyboard bindings;
// the first two gamepads connected play as player 1 and player 2 (and the
// keyboard keeps working). Presses that start and end between two simulation
// ticks are latched so a quick tap is never lost.
//
// In menus a player's light punch confirms and light kick backs out (as on
// an arcade panel), alongside Return/Escape and the pad's south/east buttons.
class Controls {
public:
    explicit Controls(const nlohmann::json& inputManager);
    ~Controls();

    Controls(const Controls&) = delete;
    Controls& operator=(const Controls&) = delete;

    // Feeds an SDL event; returns the menu action it means, if any.
    std::optional<MenuEvent> onEvent(const SDL_Event& event);
    [[nodiscard]] sim::Input sample(int player) const;
    void endTick();

    [[nodiscard]] int padCount() const;
    [[nodiscard]] std::string describe() const; // "pads: P1 Xbox Series Controller"

private:
    struct Key {
        SDL_Scancode scancode;
        sim::Input bit;
    };
    struct Button {
        SDL_GamepadButton button;
        sim::Input bit;
    };
    struct Trigger {
        SDL_GamepadAxis axis;
        sim::Input bit;
    };
    void addPad(SDL_JoystickID id);
    void removePad(SDL_JoystickID id);
    [[nodiscard]] int padPlayer(SDL_JoystickID id) const;
    std::optional<MenuEvent> stickEdge(int player, SDL_GamepadAxis axis, Sint16 value);

    std::array<std::vector<Key>, 2> keys_;
    std::vector<Button> buttons_;   // the pad layout, the same for both players
    std::vector<Trigger> triggers_;
    std::array<SDL_Gamepad*, 2> pads_{};
    std::array<std::array<int, 2>, 2> stick_{}; // last menu direction per player and axis (-1, 0, 1)
    std::array<sim::Input, 2> latched_{};
};

} // namespace iron
