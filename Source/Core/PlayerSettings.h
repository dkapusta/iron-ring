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
    float musicVolume = 1.0f; // 0..1, music mix group
    float sfxVolume = 1.0f;   // 0..1, sound-effect mix group
    float voiceVolume = 1.0f; // 0..1, announcer mix group
    bool reducedEffects = false; // screen-effects=reduced: gentler shake, no full-screen flashes
    int cpuDifficulty = 1;       // cpu-level=easy|normal|hard (0..2): Versus CPU, and the arcade ladder's offset

    bool batchMode = false;   // -batchmode: offscreen video, fixed frame count, then quit
    bool demo = false;        // -demo: CPU vs CPU
    bool training = false;    // -training: hitbox / frame-advantage overlay on
    std::string stage;        // -stage <id>: start stage (default: GameSettings startStage)
    std::string dummy;        // -dummy <mode>: training dummy (stand, crouch, jump, guard, guard-all, cpu)
    std::string fighters;     // -fighters <p1>,<p2>: fighter ids for -demo / -training / -stage (default: the first two)
    std::string scene;        // -scene <name>: open title, select, stage or options (screenshots, testing)
    bool autoplay = false;    // -autoplay: a CHAMPION CPU plays player 1 (soak tests of the arcade ladder)
    std::filesystem::path replay; // -replay <file>: play a recorded match back (and verify it)
    std::string connect;          // -connect <server[:port][/code]>: open ONLINE and connect (and join)
    std::string onlineName;       // -online-name <name>: the online name for this run
    int netsimLatencyMs = 0;      // -netsim <ms>[,<loss %>]: hold the opponent's inputs this long and drop
    int netsimLossPercent = 0;    //   some of them (testing rollback over a fast link)
    std::string keys;             // -keys "30:Return,45:Down": key taps at given frames (batch testing)
    int batchFrames = 180;
    std::filesystem::path screenshotPath;
    std::filesystem::path logFile;
    std::filesystem::path dataFolder;   // overrides <exe dir>/<product>_Data
    std::filesystem::path assetsFolder; // loose Assets/ mounted over the archives (dev)
};

// Parsed "key=value" lines; '#' starts a comment. Later keys override earlier ones.
using KeyValueMap = std::map<std::string, std::string, std::less<>>;
[[nodiscard]] KeyValueMap parseKeyValueText(std::string_view text);

// Applies one setting shared by boot.config, prefs and the command line
// (boot.config key names). False when the key is unknown or the value malformed.
bool applySetting(PlayerSettings& settings, std::string_view key, std::string_view value);

// Both functions return human-readable warnings for unknown or malformed entries.
std::vector<std::string> applyBootConfig(PlayerSettings& settings, const KeyValueMap& values);
std::vector<std::string> applyCommandLine(PlayerSettings& settings, std::span<const std::string> args);

} // namespace iron
