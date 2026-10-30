#pragma once

#include "Core/InputBindings.h"
#include "Simulation/Round.h"

#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

#include <array>
#include <optional>
#include <utility>
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

// Keyboard and gamepads -> simulation input bits and menu actions. Defaults
// come from ProjectSettings/InputManager.json, changes from prefs.cfg
// (InputBindings). Both players have keyboard bindings; the first two
// gamepads connected play as player 1 and player 2 (and the keyboard keeps
// working), each with its own device's layout. Presses that start and end
// between two simulation ticks are latched so a quick tap is never lost.
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

    // Remapping (Options -> Controls).
    [[nodiscard]] const InputBindings& bindings() const { return bindings_; }
    void loadBindings(const PlayerPrefs& prefs);
    void storeBindings(PlayerPrefs& prefs) const { bindings_.store(prefs); }
    std::vector<std::string> bindKey(int player, int action, SDL_Scancode key);
    void bindPadButton(int player, int action, const std::string& button);
    void resetKeys(int player);
    void resetPad(int player);
    [[nodiscard]] bool hasPad(int player) const { return pads_[static_cast<std::size_t>(player)] != nullptr; }
    [[nodiscard]] std::string padName(int player) const;
    // Labels for screens ("F", "KP 1", "RB"); "-" when unbound.
    [[nodiscard]] std::string keyLabel(int player, int action) const;
    [[nodiscard]] std::string padLabel(int player, int action) const;
    // What a raw event would bind: a key that may be bound, or a pad button or
    // trigger, and whose pad it is. Empty for anything else (Escape, F-keys, Start).
    [[nodiscard]] static bool bindableKey(SDL_Scancode key);
    [[nodiscard]] std::optional<std::pair<int, std::string>> padInputFor(const SDL_Event& event) const;

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
    struct PadLayout {
        std::vector<Button> buttons;
        std::vector<Trigger> triggers;
    };
    void addPad(SDL_JoystickID id);
    void removePad(SDL_JoystickID id);
    [[nodiscard]] int padPlayer(SDL_JoystickID id) const;
    [[nodiscard]] std::string padGuid(int player) const;
    std::optional<MenuEvent> stickEdge(int player, SDL_GamepadAxis axis, Sint16 value);
    void rebuild(); // binding names -> scancodes and pad buttons

    InputBindings bindings_;
    std::array<std::vector<Key>, 2> keys_;
    std::array<PadLayout, 2> padLayouts_; // for the pad in each slot
    std::array<SDL_Gamepad*, 2> pads_{};
    std::array<std::array<int, 2>, 2> stick_{}; // last menu direction per player and axis (-1, 0, 1)
    std::array<sim::Input, 2> latched_{};
};

} // namespace iron
