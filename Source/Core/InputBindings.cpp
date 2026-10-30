#include "Core/InputBindings.h"

#include <algorithm>

namespace iron {
namespace {

std::string keyPref(int player, int action) {
    return "p" + std::to_string(player + 1) + ".key." + std::string(kActionIds[static_cast<std::size_t>(action)]);
}

std::string padPref(std::string_view guid, int action) {
    return "pad." + std::string(guid) + "." + std::string(kActionIds[static_cast<std::size_t>(action)]);
}

// Several names per action are stored joined with '|': "rightshoulder|righttrigger".
std::string join(const std::vector<std::string>& names) {
    std::string out;
    for (const auto& n : names) out += (out.empty() ? "" : "|") + n;
    return out;
}

std::vector<std::string> split(std::string_view text) {
    std::vector<std::string> out;
    while (!text.empty()) {
        const auto bar = text.find('|');
        if (bar != 0) out.emplace_back(text.substr(0, bar));
        text = bar == std::string_view::npos ? std::string_view{} : text.substr(bar + 1);
    }
    return out;
}

} // namespace

int actionIndex(std::string_view id) {
    for (int i = 0; i < kActionCount; ++i)
        if (kActionIds[static_cast<std::size_t>(i)] == id) return i;
    return -1;
}

InputBindings::InputBindings(std::array<ActionKeys, 2> keys, ActionKeys pad)
    : keys_(keys), defaultKeys_(std::move(keys)), defaultPad_(std::move(pad)) {}

const ActionKeys& InputBindings::pad(std::string_view guid) const {
    const auto it = pads_.find(guid);
    return it == pads_.end() ? defaultPad_ : it->second;
}

std::vector<std::string> InputBindings::bindKey(int player, int action, const std::string& key) {
    std::vector<std::string> taken;
    for (int p = 0; p < 2; ++p) {
        for (int a = 0; a < kActionCount; ++a) {
            if (p == player && a == action) continue;
            auto& names = keys_[static_cast<std::size_t>(p)][static_cast<std::size_t>(a)];
            const auto it = std::find(names.begin(), names.end(), key);
            if (it == names.end()) continue;
            names.erase(it);
            taken.push_back("p" + std::to_string(p + 1) + "." + std::string(kActionIds[static_cast<std::size_t>(a)]));
        }
    }
    keys_[static_cast<std::size_t>(player)][static_cast<std::size_t>(action)] = {key};
    return taken;
}

void InputBindings::bindPad(std::string_view guid, int action, const std::string& button) {
    auto it = pads_.find(guid);
    if (it == pads_.end()) it = pads_.emplace(std::string(guid), defaultPad_).first;
    // A button means one thing on one pad.
    for (auto& names : it->second) names.erase(std::remove(names.begin(), names.end(), button), names.end());
    it->second[static_cast<std::size_t>(action)] = {button};
}

void InputBindings::resetKeys(int player) {
    keys_[static_cast<std::size_t>(player)] = defaultKeys_[static_cast<std::size_t>(player)];
}

void InputBindings::resetPad(std::string_view guid) {
    if (const auto it = pads_.find(guid); it != pads_.end()) pads_.erase(it);
}

void InputBindings::load(const PlayerPrefs& prefs) {
    keys_ = defaultKeys_;
    pads_.clear();
    for (const auto& [key, value] : prefs.values()) {
        if (key.size() > 7 && key[0] == 'p' && (key[1] == '1' || key[1] == '2') && key.compare(2, 5, ".key.") == 0) {
            const int action = actionIndex(std::string_view(key).substr(7));
            if (action >= 0) keys_[static_cast<std::size_t>(key[1] - '1')][static_cast<std::size_t>(action)] = split(value);
        } else if (key.rfind("pad.", 0) == 0) {
            const auto dot = key.rfind('.');
            if (dot <= 4) continue;
            const int action = actionIndex(std::string_view(key).substr(dot + 1));
            if (action < kFirstButtonAction) continue;
            const auto guid = key.substr(4, dot - 4);
            auto it = pads_.find(guid);
            if (it == pads_.end()) it = pads_.emplace(guid, defaultPad_).first;
            it->second[static_cast<std::size_t>(action)] = split(value);
        }
    }
}

void InputBindings::store(PlayerPrefs& prefs) const {
    // Rewrite every binding key: drop the old ones, write what differs from the defaults.
    std::vector<std::string> old;
    for (const auto& [key, _] : prefs.values())
        if (key.rfind("p1.key.", 0) == 0 || key.rfind("p2.key.", 0) == 0 || key.rfind("pad.", 0) == 0) old.push_back(key);
    for (const auto& key : old) prefs.erase(key);
    for (int p = 0; p < 2; ++p)
        for (int a = 0; a < kActionCount; ++a) {
            const auto& now = keys_[static_cast<std::size_t>(p)][static_cast<std::size_t>(a)];
            if (now != defaultKeys_[static_cast<std::size_t>(p)][static_cast<std::size_t>(a)])
                prefs.set(keyPref(p, a), now.empty() ? "|" : join(now)); // "|": deliberately unbound
        }
    for (const auto& [guid, layout] : pads_)
        for (int a = kFirstButtonAction; a < kActionCount; ++a)
            if (layout[static_cast<std::size_t>(a)] != defaultPad_[static_cast<std::size_t>(a)])
                prefs.set(padPref(guid, a), layout[static_cast<std::size_t>(a)].empty() ? "|" : join(layout[static_cast<std::size_t>(a)]));
}

} // namespace iron
