#include "Core/PlayerSettings.h"

#include <charconv>
#include <optional>

namespace iron {
namespace {

std::string_view trim(std::string_view s) {
    constexpr std::string_view whitespace = " \t\r\n";
    const auto first = s.find_first_not_of(whitespace);
    if (first == std::string_view::npos) return {};
    const auto last = s.find_last_not_of(whitespace);
    return s.substr(first, last - first + 1);
}

std::optional<int> toInt(std::string_view s) {
    int value = 0;
    const auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (error != std::errc{} || end != s.data() + s.size()) return std::nullopt;
    return value;
}

std::optional<float> toFloat(std::string_view s) {
    // std::from_chars for float is not universally available on older libc++.
    try {
        std::size_t used = 0;
        const float value = std::stof(std::string(s), &used);
        if (used != s.size()) return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> toBool(std::string_view s) {
    if (s == "1" || s == "true" || s == "on") return true;
    if (s == "0" || s == "false" || s == "off") return false;
    return std::nullopt;
}

std::optional<HdrMode> toHdrMode(std::string_view s) {
    if (s == "auto") return HdrMode::Auto;
    if (s == "on" || s == "1") return HdrMode::On;
    if (s == "off" || s == "0") return HdrMode::Off;
    return std::nullopt;
}

} // namespace

bool applySetting(PlayerSettings& s, std::string_view key, std::string_view value) {
    if (key == "window-mode") {
        if (value == "fullscreen") s.windowMode = WindowMode::Fullscreen;
        else if (value == "windowed") s.windowMode = WindowMode::Windowed;
        else return false;
        return true;
    }
    if (key == "presentation") {
        if (value == "integer") s.presentation = Presentation::IntegerScale;
        else if (value == "letterbox") s.presentation = Presentation::Letterbox;
        else return false;
        return true;
    }
    if (key == "vsync") {
        const auto v = toBool(value);
        if (v) s.vsync = *v;
        return v.has_value();
    }
    if (key == "hdr") {
        const auto v = toHdrMode(value);
        if (v) s.hdr = *v;
        return v.has_value();
    }
    if (key == "hdr-max-boost") {
        const auto v = toFloat(value);
        if (!v || *v < 1.0f) return false;
        s.hdrMaxBoost = *v;
        return true;
    }
    if (key == "music-volume" || key == "sfx-volume" || key == "voice-volume") {
        const auto v = toFloat(value);
        if (!v || *v < 0.0f || *v > 1.0f) return false;
        (key == "music-volume" ? s.musicVolume : key == "sfx-volume" ? s.sfxVolume : s.voiceVolume) = *v;
        return true;
    }
    if (key == "cpu-level") {
        if (value == "easy") s.cpuDifficulty = 0;
        else if (value == "normal") s.cpuDifficulty = 1;
        else if (value == "hard") s.cpuDifficulty = 2;
        else return false;
        return true;
    }
    if (key == "screen-effects") {
        if (value == "full") s.reducedEffects = false;
        else if (value == "reduced") s.reducedEffects = true;
        else return false;
        return true;
    }
    if (key == "render-driver") {
        s.renderDriver = std::string(value);
        return true;
    }
    return false;
}

KeyValueMap parseKeyValueText(std::string_view text) {
    KeyValueMap result;
    while (!text.empty()) {
        const auto newline = text.find('\n');
        auto line = text.substr(0, newline);
        text = newline == std::string_view::npos ? std::string_view{} : text.substr(newline + 1);

        if (const auto hash = line.find('#'); hash != std::string_view::npos) line = line.substr(0, hash);
        const auto equals = line.find('=');
        if (equals == std::string_view::npos) continue;
        const auto key = trim(line.substr(0, equals));
        if (key.empty()) continue;
        result[std::string(key)] = std::string(trim(line.substr(equals + 1)));
    }
    return result;
}

std::vector<std::string> applyBootConfig(PlayerSettings& settings, const KeyValueMap& values) {
    std::vector<std::string> warnings;
    for (const auto& [key, value] : values) {
        if (key == "player-version") continue; // informational
        if (!applySetting(settings, key, value)) warnings.push_back("boot.config: ignored '" + key + "=" + value + "'");
    }
    return warnings;
}

std::vector<std::string> applyCommandLine(PlayerSettings& settings, std::span<const std::string> args) {
    std::vector<std::string> warnings;
    const auto next = [&](std::size_t& i) -> std::optional<std::string_view> {
        if (i + 1 >= args.size()) return std::nullopt;
        return std::string_view(args[++i]);
    };

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        bool ok = true;
        if (arg == "-batchmode") {
            settings.batchMode = true;
        } else if (arg == "-demo") {
            settings.demo = true;
        } else if (arg == "-training") {
            settings.training = true;
        } else if (arg == "-autoplay") {
            settings.autoplay = true;
        } else if (arg == "-stage" || arg == "-dummy" || arg == "-scene" || arg == "-keys" || arg == "-fighters") {
            const auto v = next(i);
            ok = v && !v->empty();
            if (ok) {
                auto& target = arg == "-stage"   ? settings.stage
                             : arg == "-dummy"   ? settings.dummy
                             : arg == "-scene"   ? settings.scene
                             : arg == "-fighters" ? settings.fighters
                                                 : settings.keys;
                target = std::string(*v);
            }
        } else if (arg == "-replay") {
            const auto v = next(i);
            ok = v && !v->empty();
            if (ok) settings.replay = std::filesystem::path(std::string(*v));
        } else if (arg == "-screen-fullscreen") {
            const auto v = next(i);
            const auto b = v ? toBool(*v) : std::nullopt;
            if (b) settings.windowMode = *b ? WindowMode::Fullscreen : WindowMode::Windowed;
            ok = b.has_value();
        } else if (arg == "-screen-width" || arg == "-screen-height") {
            const auto v = next(i);
            const auto n = v ? toInt(*v) : std::nullopt;
            ok = n && *n > 0;
            if (ok) (arg == "-screen-width" ? settings.windowWidth : settings.windowHeight) = *n;
        } else if (arg == "-frames") {
            const auto v = next(i);
            const auto n = v ? toInt(*v) : std::nullopt;
            ok = n && *n > 0;
            if (ok) settings.batchFrames = *n;
        } else if (arg == "-logFile" || arg == "-screenshot" || arg == "-data-folder" || arg == "-assets-folder") {
            const auto v = next(i);
            ok = v.has_value();
            if (ok) {
                std::filesystem::path p{std::u8string(v->begin(), v->end())};
                if (arg == "-logFile") settings.logFile = p;
                else if (arg == "-screenshot") settings.screenshotPath = p;
                else if (arg == "-data-folder") settings.dataFolder = p;
                else settings.assetsFolder = p;
            }
        } else if (arg == "-hdr" || arg == "-vsync" || arg == "-presentation" || arg == "-render-driver" ||
                   arg == "-music-volume" || arg == "-sfx-volume" || arg == "-voice-volume" ||
                   arg == "-screen-effects" || arg == "-cpu-level") {
            const auto v = next(i);
            ok = v && applySetting(settings, arg.substr(1), *v);
        } else {
            ok = false;
        }
        if (!ok) warnings.push_back("command line: ignored '" + std::string(arg) + "'");
    }
    return warnings;
}

} // namespace iron
