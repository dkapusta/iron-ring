#pragma once

#include <filesystem>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace iron {

enum class WindowMode { Fullscreen, Windowed };
enum class HdrMode { Auto, On, Off };
enum class Presentation { IntegerScale, Letterbox };

// Effective player configuration: defaults, then <name>_Data/boot.config,
// then command-line arguments (Unity-compatible names where they exist).
struct PlayerSettings {
    WindowMode windowMode = WindowMode::Fullscreen;
    int windowWidth = 1280;   // windowed mode only; fullscreen uses the desktop mode
    int windowHeight = 720;
    Presentation presentation = Presentation::IntegerScale;
    bool vsync = true;
    HdrMode hdr = HdrMode::Auto;
    float hdrMaxBoost = 2.0f; // highlight multiplier ceiling, in SDR-white units
    std::string renderDriver; // empty = let SDL choose

    bool batchMode = false;   // -batchmode: offscreen video, fixed frame count, then quit
    int batchFrames = 180;
    std::filesystem::path screenshotPath;
    std::filesystem::path logFile;
    std::filesystem::path dataFolder;   // overrides <exe dir>/<product>_Data
    std::filesystem::path assetsFolder; // loose Assets/ mounted over the archives (dev)
};

// Parsed "key=value" lines; '#' starts a comment. Later keys override earlier ones.
using KeyValueMap = std::map<std::string, std::string, std::less<>>;
[[nodiscard]] KeyValueMap parseKeyValueText(std::string_view text);

// Both functions return human-readable warnings for unknown or malformed entries.
std::vector<std::string> applyBootConfig(PlayerSettings& settings, const KeyValueMap& values);
std::vector<std::string> applyCommandLine(PlayerSettings& settings, std::span<const std::string> args);

} // namespace iron
