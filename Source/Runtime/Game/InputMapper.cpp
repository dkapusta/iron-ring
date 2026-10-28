#include "Game/InputMapper.h"

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

} // namespace

InputMapper::InputMapper(const nlohmann::json& inputManager) {
    const auto& players = inputManager.at("players");
    for (std::size_t p = 0; p < bindings_.size() && p < players.size(); ++p) {
        for (const auto& [action, bit] : kActions) {
            for (const auto& keyName : players[p].value(action, nlohmann::json::array())) {
                const auto name = keyName.get<std::string>();
                const SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
                if (scancode == SDL_SCANCODE_UNKNOWN)
                    throw std::runtime_error("InputManager.json: unknown key '" + name + "'");
                bindings_[p].push_back({scancode, bit});
            }
        }
    }
}

void InputMapper::onKeyDown(const SDL_KeyboardEvent& key) {
    if (key.repeat) return;
    for (std::size_t p = 0; p < bindings_.size(); ++p)
        for (const auto& b : bindings_[p])
            if (b.scancode == key.scancode) latched_[p] = static_cast<sim::Input>(latched_[p] | b.bit);
}

sim::Input InputMapper::sample(int player) const {
    const auto p = static_cast<std::size_t>(player);
    const bool* keys = SDL_GetKeyboardState(nullptr);
    sim::Input held = latched_[p];
    for (const auto& b : bindings_[p])
        if (keys[b.scancode]) held = static_cast<sim::Input>(held | b.bit);
    return held;
}

void InputMapper::endTick() { latched_ = {}; }

} // namespace iron
