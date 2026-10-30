#include "Game/Controls.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace iron {
namespace {

constexpr std::pair<const char*, sim::Input> kActions[]{
    {"left", sim::Left},           {"right", sim::Right},           {"up", sim::Up},
    {"down", sim::Down},           {"lightPunch", sim::LightPunch}, {"heavyPunch", sim::HeavyPunch},
    {"lightKick", sim::LightKick}, {"heavyKick", sim::HeavyKick},   {"guard", sim::Guard},
};

constexpr Sint16 kStickHeld = 16000;    // a stick past this counts as a held direction
constexpr Sint16 kStickRelease = 9000;  // menus: back under this re-arms the next step
constexpr Sint16 kTriggerHeld = 12000;

std::optional<MenuAction> keyAction(SDL_Scancode key) {
    switch (key) {
    case SDL_SCANCODE_RETURN:
    case SDL_SCANCODE_KP_ENTER: return MenuAction::Confirm;
    case SDL_SCANCODE_BACKSPACE: return MenuAction::Back;
    case SDL_SCANCODE_ESCAPE: return MenuAction::Pause;
    default: return std::nullopt;
    }
}

std::optional<MenuAction> bitAction(sim::Input bit) {
    switch (bit) {
    case sim::Up: return MenuAction::Up;
    case sim::Down: return MenuAction::Down;
    case sim::Left: return MenuAction::Left;
    case sim::Right: return MenuAction::Right;
    case sim::LightPunch: return MenuAction::Confirm;
    case sim::LightKick: return MenuAction::Back;
    default: return std::nullopt;
    }
}

std::optional<MenuAction> padAction(SDL_GamepadButton button) {
    switch (button) {
    case SDL_GAMEPAD_BUTTON_DPAD_UP: return MenuAction::Up;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return MenuAction::Down;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return MenuAction::Left;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return MenuAction::Right;
    case SDL_GAMEPAD_BUTTON_SOUTH: return MenuAction::Confirm;
    case SDL_GAMEPAD_BUTTON_EAST: return MenuAction::Back;
    case SDL_GAMEPAD_BUTTON_START: return MenuAction::Pause;
    default: return std::nullopt;
    }
}

} // namespace

Controls::Controls(const nlohmann::json& inputManager) {
    const auto& players = inputManager.at("players");
    for (std::size_t p = 0; p < keys_.size() && p < players.size(); ++p) {
        for (const auto& [action, bit] : kActions) {
            for (const auto& keyName : players[p].value(action, nlohmann::json::array())) {
                const auto name = keyName.get<std::string>();
                const SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
                if (scancode == SDL_SCANCODE_UNKNOWN)
                    throw std::runtime_error("InputManager.json: unknown key '" + name + "'");
                keys_[p].push_back({scancode, bit});
            }
        }
    }
    // Pad layout by SDL gamepad names ("x", "rightshoulder", "righttrigger").
    // The d-pad and the left stick always move.
    const auto pad = inputManager.value("gamepad", nlohmann::json::object());
    for (const auto& [action, bit] : kActions) {
        for (const auto& item : pad.value(action, nlohmann::json::array())) {
            const auto name = item.get<std::string>();
            const SDL_GamepadButton button = SDL_GetGamepadButtonFromString(name.c_str());
            const SDL_GamepadAxis axis = SDL_GetGamepadAxisFromString(name.c_str());
            if (button != SDL_GAMEPAD_BUTTON_INVALID) buttons_.push_back({button, bit});
            else if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                triggers_.push_back({axis, bit});
            else throw std::runtime_error("InputManager.json: unknown gamepad input '" + name + "'");
        }
    }
    constexpr std::pair<SDL_GamepadButton, sim::Input> dpad[]{
        {SDL_GAMEPAD_BUTTON_DPAD_LEFT, sim::Left}, {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, sim::Right},
        {SDL_GAMEPAD_BUTTON_DPAD_UP, sim::Up}, {SDL_GAMEPAD_BUTTON_DPAD_DOWN, sim::Down}};
    for (const auto& [button, bit] : dpad) buttons_.push_back({button, bit});

    // Pads connected before start-up arrive as SDL_EVENT_GAMEPAD_ADDED events.
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) SDL_Log("Gamepads: disabled (%s)", SDL_GetError());
}

Controls::~Controls() {
    for (auto*& pad : pads_)
        if (pad) SDL_CloseGamepad(pad);
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

int Controls::padPlayer(SDL_JoystickID id) const {
    for (std::size_t p = 0; p < pads_.size(); ++p)
        if (pads_[p] && SDL_GetGamepadID(pads_[p]) == id) return static_cast<int>(p);
    return -1;
}

void Controls::addPad(SDL_JoystickID id) {
    if (padPlayer(id) >= 0) return;
    for (std::size_t p = 0; p < pads_.size(); ++p) {
        if (pads_[p]) continue;
        pads_[p] = SDL_OpenGamepad(id);
        if (!pads_[p]) {
            SDL_Log("Gamepad: cannot open (%s)", SDL_GetError());
            return;
        }
        SDL_SetGamepadPlayerIndex(pads_[p], static_cast<int>(p));
        const char* name = SDL_GetGamepadName(pads_[p]);
        SDL_Log("Gamepad: %s is player %d", name ? name : "gamepad", static_cast<int>(p) + 1);
        return;
    }
    const char* name = SDL_GetGamepadNameForID(id);
    SDL_Log("Gamepad: %s ignored (two are already connected)", name ? name : "gamepad");
}

void Controls::removePad(SDL_JoystickID id) {
    const int p = padPlayer(id);
    if (p < 0) return;
    auto& pad = pads_[static_cast<std::size_t>(p)];
    SDL_Log("Gamepad: player %d disconnected", p + 1);
    SDL_CloseGamepad(pad);
    pad = nullptr;
}

std::optional<MenuEvent> Controls::stickEdge(int player, SDL_GamepadAxis axis, Sint16 value) {
    if (axis != SDL_GAMEPAD_AXIS_LEFTX && axis != SDL_GAMEPAD_AXIS_LEFTY) return std::nullopt;
    const std::size_t a = axis == SDL_GAMEPAD_AXIS_LEFTX ? 0 : 1;
    int& last = stick_[static_cast<std::size_t>(player)][a];
    const int now = value > kStickHeld ? 1 : value < -kStickHeld ? -1 : 0;
    if (now == 0) {
        if (value > -kStickRelease && value < kStickRelease) last = 0;
        return std::nullopt;
    }
    if (now == last) return std::nullopt;
    last = now;
    const MenuAction action = a == 0 ? (now < 0 ? MenuAction::Left : MenuAction::Right)
                                     : (now < 0 ? MenuAction::Up : MenuAction::Down);
    return MenuEvent{player, action};
}

std::optional<MenuEvent> Controls::onEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_GAMEPAD_ADDED: addPad(event.gdevice.which); return std::nullopt;
    case SDL_EVENT_GAMEPAD_REMOVED: removePad(event.gdevice.which); return std::nullopt;
    case SDL_EVENT_KEY_DOWN: {
        const auto& key = event.key;
        std::optional<MenuEvent> menu;
        if (const auto action = keyAction(key.scancode)) menu = MenuEvent{0, *action};
        for (std::size_t p = 0; p < keys_.size(); ++p) {
            for (const auto& k : keys_[p]) {
                if (k.scancode != key.scancode) continue;
                if (!key.repeat) latched_[p] = static_cast<sim::Input>(latched_[p] | k.bit);
                if (const auto action = bitAction(k.bit); action && !menu)
                    menu = MenuEvent{static_cast<int>(p), *action};
            }
        }
        // Held keys repeat for menu movement, never for Confirm/Back.
        if (menu && key.repeat && menu->action != MenuAction::Up && menu->action != MenuAction::Down &&
            menu->action != MenuAction::Left && menu->action != MenuAction::Right)
            return std::nullopt;
        return menu;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        const int p = padPlayer(event.gbutton.which);
        if (p < 0) return std::nullopt;
        const auto button = static_cast<SDL_GamepadButton>(event.gbutton.button);
        for (const auto& b : buttons_)
            if (b.button == button)
                latched_[static_cast<std::size_t>(p)] = static_cast<sim::Input>(latched_[static_cast<std::size_t>(p)] | b.bit);
        if (const auto action = padAction(button)) return MenuEvent{p, *action};
        return std::nullopt;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const int p = padPlayer(event.gaxis.which);
        if (p < 0) return std::nullopt;
        return stickEdge(p, static_cast<SDL_GamepadAxis>(event.gaxis.axis), event.gaxis.value);
    }
    default: return std::nullopt;
    }
}

sim::Input Controls::sample(int player) const {
    const auto p = static_cast<std::size_t>(player);
    const bool* keys = SDL_GetKeyboardState(nullptr);
    auto held = latched_[p];
    const auto add = [&held](sim::Input bit) { held = static_cast<sim::Input>(held | bit); };
    for (const auto& k : keys_[p])
        if (keys[k.scancode]) add(k.bit);
    if (SDL_Gamepad* pad = pads_[p]) {
        for (const auto& b : buttons_)
            if (SDL_GetGamepadButton(pad, b.button)) add(b.bit);
        for (const auto& t : triggers_)
            if (SDL_GetGamepadAxis(pad, t.axis) > kTriggerHeld) add(t.bit);
        const Sint16 x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
        const Sint16 y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
        if (x < -kStickHeld) add(sim::Left);
        if (x > kStickHeld) add(sim::Right);
        if (y < -kStickHeld) add(sim::Up);
        if (y > kStickHeld) add(sim::Down);
    }
    return held;
}

void Controls::endTick() { latched_ = {}; }

int Controls::padCount() const {
    int count = 0;
    for (const auto* pad : pads_) count += pad ? 1 : 0;
    return count;
}

std::string Controls::describe() const {
    std::string text = "pads:";
    for (std::size_t p = 0; p < pads_.size(); ++p)
        if (pads_[p]) {
            const char* name = SDL_GetGamepadName(pads_[p]);
            text += " P" + std::to_string(p + 1) + " " + (name ? name : "gamepad");
        }
    return padCount() == 0 ? "pads: none (keyboard)" : text;
}

} // namespace iron
