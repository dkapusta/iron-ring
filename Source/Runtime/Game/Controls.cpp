#include "Game/Controls.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>

namespace iron {
namespace {

// Simulation bits in InputBindings action order (kActionIds).
constexpr std::array<sim::Input, kActionCount> kActionBits{sim::Left,      sim::Right,      sim::Up,
                                                           sim::Down,      sim::LightPunch, sim::HeavyPunch,
                                                           sim::LightKick, sim::HeavyKick,  sim::Guard};

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

ActionKeys readActions(const nlohmann::json& table, bool pad) {
    ActionKeys out;
    for (int a = 0; a < kActionCount; ++a) {
        const std::string id(kActionIds[static_cast<std::size_t>(a)]);
        for (const auto& item : table.value(id, nlohmann::json::array())) {
            const auto name = item.get<std::string>();
            const bool known = pad ? (SDL_GetGamepadButtonFromString(name.c_str()) != SDL_GAMEPAD_BUTTON_INVALID ||
                                      SDL_GetGamepadAxisFromString(name.c_str()) == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ||
                                      SDL_GetGamepadAxisFromString(name.c_str()) == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                                   : SDL_GetScancodeFromName(name.c_str()) != SDL_SCANCODE_UNKNOWN;
            if (!known)
                throw std::runtime_error("InputManager.json: unknown " + std::string(pad ? "gamepad input" : "key") +
                                         " '" + name + "'");
            out[static_cast<std::size_t>(a)].push_back(name);
        }
    }
    return out;
}

} // namespace

Controls::Controls(const nlohmann::json& inputManager) {
    const auto& players = inputManager.at("players");
    std::array<ActionKeys, 2> keys{};
    for (std::size_t p = 0; p < keys.size() && p < players.size(); ++p) keys[p] = readActions(players[p], false);
    bindings_ = InputBindings(keys, readActions(inputManager.value("gamepad", nlohmann::json::object()), true));
    rebuild();
    // Pads connected before start-up arrive as SDL_EVENT_GAMEPAD_ADDED events.
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) SDL_Log("Gamepads: disabled (%s)", SDL_GetError());
}

void Controls::rebuild() {
    for (int p = 0; p < 2; ++p) {
        auto& table = keys_[static_cast<std::size_t>(p)];
        table.clear();
        const auto& names = bindings_.keys(p);
        for (int a = 0; a < kActionCount; ++a)
            for (const auto& name : names[static_cast<std::size_t>(a)]) {
                const SDL_Scancode code = SDL_GetScancodeFromName(name.c_str());
                if (code != SDL_SCANCODE_UNKNOWN) table.push_back({code, kActionBits[static_cast<std::size_t>(a)]});
            }

        // The pad in this slot: its own profile, or the default layout. The d-pad always moves.
        auto& layout = padLayouts_[static_cast<std::size_t>(p)];
        layout = {};
        const auto& pad = bindings_.pad(padGuid(p));
        for (int a = kFirstButtonAction; a < kActionCount; ++a)
            for (const auto& name : pad[static_cast<std::size_t>(a)]) {
                const SDL_GamepadButton button = SDL_GetGamepadButtonFromString(name.c_str());
                const SDL_GamepadAxis axis = SDL_GetGamepadAxisFromString(name.c_str());
                const auto bit = kActionBits[static_cast<std::size_t>(a)];
                if (button != SDL_GAMEPAD_BUTTON_INVALID) layout.buttons.push_back({button, bit});
                else if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                    layout.triggers.push_back({axis, bit});
            }
        constexpr std::pair<SDL_GamepadButton, sim::Input> dpad[]{
            {SDL_GAMEPAD_BUTTON_DPAD_LEFT, sim::Left}, {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, sim::Right},
            {SDL_GAMEPAD_BUTTON_DPAD_UP, sim::Up}, {SDL_GAMEPAD_BUTTON_DPAD_DOWN, sim::Down}};
        for (const auto& [button, bit] : dpad) layout.buttons.push_back({button, bit});
    }
}

std::string Controls::padGuid(int player) const {
    SDL_Gamepad* pad = pads_[static_cast<std::size_t>(player)];
    if (!pad) return {};
    char text[64];
    SDL_GUIDToString(SDL_GetGamepadGUIDForID(SDL_GetGamepadID(pad)), text, sizeof text);
    return text;
}

void Controls::loadBindings(const PlayerPrefs& prefs) {
    bindings_.load(prefs);
    rebuild();
}

std::vector<std::string> Controls::bindKey(int player, int action, SDL_Scancode key) {
    auto taken = bindings_.bindKey(player, action, SDL_GetScancodeName(key));
    rebuild();
    return taken;
}

void Controls::bindPadButton(int player, int action, const std::string& button) {
    const auto guid = padGuid(player);
    if (guid.empty()) return;
    bindings_.bindPad(guid, action, button);
    rebuild();
}

void Controls::resetKeys(int player) {
    bindings_.resetKeys(player);
    rebuild();
}

void Controls::resetPad(int player) {
    bindings_.resetPad(padGuid(player));
    rebuild();
}

std::string Controls::padName(int player) const {
    SDL_Gamepad* pad = pads_[static_cast<std::size_t>(player)];
    const char* name = pad ? SDL_GetGamepadName(pad) : nullptr;
    return pad ? (name ? name : "gamepad") : "";
}

std::string Controls::keyLabel(int player, int action) const {
    const auto& names = bindings_.keys(player)[static_cast<std::size_t>(action)];
    if (names.empty()) return "-";
    std::string label = names.front();
    if (label.rfind("Keypad ", 0) == 0) label = "KP " + label.substr(7);
    for (char& c : label) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return label;
}

std::string Controls::padLabel(int player, int action) const {
    if (action < kFirstButtonAction) return "D-PAD";
    const auto& names = bindings_.pad(padGuid(player))[static_cast<std::size_t>(action)];
    if (names.empty()) return "-";
    static const std::pair<std::string_view, std::string_view> shortNames[]{
        {"leftshoulder", "LB"}, {"rightshoulder", "RB"}, {"lefttrigger", "LT"}, {"righttrigger", "RT"},
        {"leftstick", "LS"},    {"rightstick", "RS"},    {"back", "BACK"},     {"guide", "HOME"}};
    std::string label;
    for (const auto& name : names) {
        std::string part = name;
        for (const auto& [full, abbr] : shortNames)
            if (name == full) part = abbr;
        for (char& c : part) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        label += (label.empty() ? "" : "/") + part;
    }
    return label;
}

bool Controls::bindableKey(SDL_Scancode key) {
    // Escape cancels, F-keys are global; the rest is fair game.
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_UNKNOWN) return false;
    if (key >= SDL_SCANCODE_F1 && key <= SDL_SCANCODE_F12) return false;
    return SDL_GetScancodeName(key)[0] != '\0';
}

std::optional<std::pair<int, std::string>> Controls::padInputFor(const SDL_Event& event) const {
    if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        const int p = padPlayer(event.gbutton.which);
        const auto button = static_cast<SDL_GamepadButton>(event.gbutton.button);
        // Start pauses and the d-pad moves: neither can become an attack.
        if (p < 0 || button == SDL_GAMEPAD_BUTTON_START || (button >= SDL_GAMEPAD_BUTTON_DPAD_UP &&
                                                            button <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
            return std::nullopt;
        return std::pair{p, std::string(SDL_GetGamepadStringForButton(button))};
    }
    if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && event.gaxis.value > kTriggerHeld) {
        const auto axis = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
        const int p = padPlayer(event.gaxis.which);
        if (p >= 0 && (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER))
            return std::pair{p, std::string(SDL_GetGamepadStringForAxis(axis))};
    }
    return std::nullopt;
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
        rebuild(); // this device's own layout, if it has one
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
    rebuild();
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
    case SDL_EVENT_KEY_UP:
        if (event.key.scancode < SDL_SCANCODE_COUNT) held_[event.key.scancode] = false;
        return std::nullopt;
    case SDL_EVENT_KEY_DOWN: {
        const auto& key = event.key;
        if (key.scancode < SDL_SCANCODE_COUNT) held_[key.scancode] = true;
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
        for (const auto& b : padLayouts_[static_cast<std::size_t>(p)].buttons)
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
        if (keys[k.scancode] || held_[k.scancode]) add(k.bit);
    if (SDL_Gamepad* pad = pads_[p]) {
        for (const auto& b : padLayouts_[p].buttons)
            if (SDL_GetGamepadButton(pad, b.button)) add(b.bit);
        for (const auto& t : padLayouts_[p].triggers)
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
