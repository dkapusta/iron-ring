#include "Core/PlayerPrefs.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <system_error>

namespace iron {
namespace {

std::string number(float value) {
    char text[32];
    std::snprintf(text, sizeof text, "%.2f", static_cast<double>(value));
    return text;
}

bool isSettingKey(std::string_view key) {
    return std::find(std::begin(PlayerPrefs::kSettingKeys), std::end(PlayerPrefs::kSettingKeys), key) !=
           std::end(PlayerPrefs::kSettingKeys);
}

} // namespace

std::string settingText(const PlayerSettings& s, std::string_view key) {
    if (key == "window-mode") return s.windowMode == WindowMode::Fullscreen ? "fullscreen" : "windowed";
    if (key == "presentation") return s.presentation == Presentation::IntegerScale ? "integer" : "letterbox";
    if (key == "vsync") return s.vsync ? "1" : "0";
    if (key == "hdr") return s.hdr == HdrMode::Auto ? "auto" : s.hdr == HdrMode::On ? "on" : "off";
    if (key == "music-volume") return number(s.musicVolume);
    if (key == "sfx-volume") return number(s.sfxVolume);
    if (key == "voice-volume") return number(s.voiceVolume);
    return {};
}

std::string formatKeyValueText(const KeyValueMap& values, std::string_view header) {
    std::string text;
    if (!header.empty()) text += "# " + std::string(header) + "\n";
    for (const auto& [key, value] : values) {
        // Values are single-line; '#' would start a comment on the way back in.
        std::string clean;
        for (const char c : value)
            if (c != '\n' && c != '\r' && c != '#') clean += c;
        text += key + "=" + clean + "\n";
    }
    return text;
}

PlayerPrefs::PlayerPrefs(std::filesystem::path file) : file_(std::move(file)) {}

std::vector<std::string> PlayerPrefs::load() {
    values_.clear();
    std::vector<std::string> warnings;
    std::ifstream in(file_, std::ios::binary);
    if (!in) return warnings; // first run
    std::stringstream text;
    text << in.rdbuf();
    values_ = parseKeyValueText(text.str());
    return warnings;
}

bool PlayerPrefs::save() const {
    if (file_.empty()) return false;
    std::error_code error;
    std::filesystem::create_directories(file_.parent_path(), error);
    auto temporary = file_;
    temporary += ".tmp";
    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << formatKeyValueText(values_, "Iron Ring player preferences. Written by the game; safe to delete.");
        if (!out) return false;
    }
    std::filesystem::rename(temporary, file_, error); // replaces atomically on POSIX and NTFS
    return !error;
}

std::optional<std::string> PlayerPrefs::get(std::string_view key) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return std::nullopt;
    return it->second;
}

void PlayerPrefs::set(std::string_view key, std::string value) { values_[std::string(key)] = std::move(value); }

void PlayerPrefs::erase(std::string_view key) {
    if (const auto it = values_.find(key); it != values_.end()) values_.erase(it);
}

std::vector<std::string> PlayerPrefs::applyTo(PlayerSettings& settings) const {
    std::vector<std::string> warnings;
    for (const auto& [key, value] : values_)
        if (isSettingKey(key) && !applySetting(settings, key, value))
            warnings.push_back("prefs.cfg: ignored '" + key + "=" + value + "'");
    return warnings;
}

void PlayerPrefs::storeSettings(const PlayerSettings& settings) {
    for (const auto key : kSettingKeys) set(key, settingText(settings, key));
}

} // namespace iron
