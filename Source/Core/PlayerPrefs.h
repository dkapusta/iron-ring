#pragma once

#include "Core/PlayerSettings.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace iron {

// What the player chose in the Options screen, kept between runs, like
// Unity's PlayerPrefs: <user folder>/prefs.cfg, in boot.config syntax.
// Settings keys use the boot.config names (music-volume=0.8, vsync=0), and
// sit between boot.config and the command line: defaults < boot.config <
// prefs < command line. Other keys (last picks, later the key bindings) are
// plain strings for the game to read back.
class PlayerPrefs {
public:
    PlayerPrefs() = default;
    explicit PlayerPrefs(std::filesystem::path file);

    // Reads the file; a missing file is an empty store. Returns warnings.
    std::vector<std::string> load();
    // Writes atomically (temporary file, then rename). False on I/O failure.
    bool save() const;

    [[nodiscard]] const std::filesystem::path& file() const { return file_; }
    [[nodiscard]] const KeyValueMap& values() const { return values_; }
    [[nodiscard]] std::optional<std::string> get(std::string_view key) const;
    void set(std::string_view key, std::string value);
    void erase(std::string_view key);

    // Settings stored here, by boot.config key: the ones the Options screen edits.
    static constexpr std::string_view kSettingKeys[]{"window-mode", "presentation", "vsync", "hdr",
                                                     "music-volume", "sfx-volume", "voice-volume",
                                                     "screen-effects"};
    // Applies the settings keys to `settings` (other keys are left alone).
    std::vector<std::string> applyTo(PlayerSettings& settings) const;
    // Stores the current value of every settings key.
    void storeSettings(const PlayerSettings& settings);

private:
    std::filesystem::path file_;
    KeyValueMap values_;
};

// The boot.config text of one setting ("windowed", "0.8", "1"); empty for unknown keys.
[[nodiscard]] std::string settingText(const PlayerSettings& settings, std::string_view key);
[[nodiscard]] std::string formatKeyValueText(const KeyValueMap& values, std::string_view header = {});

} // namespace iron
