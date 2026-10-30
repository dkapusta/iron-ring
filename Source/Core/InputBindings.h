#pragma once

#include "Core/PlayerPrefs.h"

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace iron {

// The nine things a player can bind, in Options -> Controls order.
inline constexpr int kActionCount = 9;
inline constexpr std::array<std::string_view, kActionCount> kActionIds{
    "left", "right", "up", "down", "lightPunch", "heavyPunch", "lightKick", "heavyKick", "guard"};
inline constexpr std::array<std::string_view, kActionCount> kActionLabels{
    "LEFT", "RIGHT", "UP", "DOWN", "LIGHT PUNCH", "HEAVY PUNCH", "LIGHT KICK", "HEAVY KICK", "GUARD"};
// Pads always move with the d-pad and left stick; only the buttons are rebindable.
inline constexpr int kFirstButtonAction = 4;
[[nodiscard]] int actionIndex(std::string_view id); // -1 if unknown

using ActionKeys = std::array<std::vector<std::string>, kActionCount>; // names per action

// Who presses what, by name: SDL scancode names for keys ("F", "Keypad 1"),
// SDL gamepad names for pad buttons ("x", "rightshoulder", "righttrigger").
// Defaults come from InputManager.json; changes are stored in prefs.cfg:
//   p1.key.lightPunch=F            (keyboard, per player)
//   pad.<guid>.lightPunch=x        (gamepad, per device: a pad keeps its layout in any slot)
// No SDL here, so the conflict rules are unit-tested.
class InputBindings {
public:
    InputBindings() = default;
    InputBindings(std::array<ActionKeys, 2> keys, ActionKeys pad);

    [[nodiscard]] const ActionKeys& keys(int player) const { return keys_.at(static_cast<std::size_t>(player)); }
    // The layout for a pad: its own profile if it has one, else the default.
    [[nodiscard]] const ActionKeys& pad(std::string_view guid) const;
    [[nodiscard]] const ActionKeys& defaultPad() const { return defaultPad_; }

    // Binds one key to an action, replacing the action's keys. The key is
    // taken away from any other action of either player, so one press never
    // means two things. Returns the actions that lost it, as "p<n>.<action>".
    std::vector<std::string> bindKey(int player, int action, const std::string& key);
    // Binds a pad button for one device; it leaves the device's other actions.
    void bindPad(std::string_view guid, int action, const std::string& button);
    void resetKeys(int player);
    void resetPad(std::string_view guid);

    // prefs.cfg: only what differs from the defaults is stored.
    void load(const PlayerPrefs& prefs);
    void store(PlayerPrefs& prefs) const;

private:
    std::array<ActionKeys, 2> keys_{};
    std::array<ActionKeys, 2> defaultKeys_{};
    ActionKeys defaultPad_{};
    std::map<std::string, ActionKeys, std::less<>> pads_; // by device GUID
};

} // namespace iron
