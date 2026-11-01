#include "Server/Settings.h"

#include "Core/PlayerSettings.h"
#include "Net/Net.h"

#include <charconv>

namespace iron::server {
namespace {

std::optional<int> number(std::string_view v, int lo, int hi) {
    int n = 0;
    const auto [end, ec] = std::from_chars(v.data(), v.data() + v.size(), n);
    if (ec != std::errc{} || end != v.data() + v.size() || n < lo || n > hi) return std::nullopt;
    return n;
}

std::optional<bool> flag(std::string_view v) {
    if (v == "1" || v == "true" || v == "yes" || v == "on") return true;
    if (v == "0" || v == "false" || v == "no" || v == "off") return false;
    return std::nullopt;
}

std::string upper(std::string_view v, std::size_t max) {
    std::string s;
    for (const char c : v) {
        if (s.size() == max) break;
        s += static_cast<char>(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);
    }
    return s;
}

} // namespace

bool applySetting(Settings& s, std::string_view key, std::string_view value) {
    auto& c = s.server;
    const auto setInt = [&](int& target, int lo, int hi) {
        const auto n = number(value, lo, hi);
        if (n) target = *n;
        return n.has_value();
    };
    if (key == "port") {
        int p = c.port;
        if (!setInt(p, 0, 65535)) return false;
        c.port = static_cast<std::uint16_t>(p);
        return true;
    }
    if (key == "bind") {
        c.bind = std::string(value);
        return true;
    }
    if (key == "name") {
        c.name = upper(value, 40);
        return !c.name.empty();
    }
    if (key == "motd") {
        c.motd = upper(value, 60);
        return true;
    }
    if (key == "maxPlayers") return setInt(c.maxPlayers, 2, 4000);
    if (key == "maxRooms") return setInt(c.maxRooms, 1, 2000);
    if (key == "roomsPerAddress") return setInt(c.roomsPerAddress, 1, 100);
    if (key == "idleMinutes") return setInt(c.idleMinutes, 1, 24 * 60);
    if (key == "joinAttemptsPerMinute") return setInt(c.joinAttemptsPerMinute, 1, 1000);
    if (key == "verify") {
        const auto b = flag(value);
        if (b) c.verify = *b;
        return b.has_value();
    }
    if (key == "data") {
        s.data = std::filesystem::path(std::string(value));
        return !value.empty();
    }
    if (key == "log") {
        s.log = std::filesystem::path(std::string(value));
        return true;
    }
    return false;
}

std::vector<std::string> applyConfigText(Settings& settings, std::string_view text) {
    std::vector<std::string> warnings;
    for (const auto& [key, value] : parseKeyValueText(text))
        if (!applySetting(settings, key, value)) warnings.push_back("ignored '" + key + "=" + value + "'");
    return warnings;
}

std::vector<std::string> applyArguments(Settings& settings, std::span<const std::string> args) {
    std::vector<std::string> errors;
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view a = args[i];
        if (a == "--check") {
            settings.check = true;
            continue;
        }
        if (a == "--help" || a == "-h" || a == "/?") {
            settings.help = true;
            continue;
        }
        if (a == "--version") {
            settings.version = true;
            continue;
        }
        if (a.rfind("--", 0) != 0) {
            errors.push_back("unexpected argument '" + std::string(a) + "'");
            continue;
        }
        a.remove_prefix(2);
        std::string key, value;
        if (const auto eq = a.find('='); eq != std::string_view::npos) {
            key = std::string(a.substr(0, eq));
            value = std::string(a.substr(eq + 1));
        } else if (i + 1 < args.size()) {
            key = std::string(a);
            value = args[++i];
        } else {
            errors.push_back("--" + std::string(a) + " needs a value");
            continue;
        }
        if (key == "config")
            settings.config = value;
        else if (!applySetting(settings, key, value))
            errors.push_back("bad option --" + key + " " + value);
    }
    return errors;
}

std::string usage() {
    return "IronRingServer: the Iron Ring dedicated server (docs/server.md)\n"
           "\n"
           "  IronRingServer [--config server.cfg] [--key value ...] [--check]\n"
           "\n"
           "Keys (in server.cfg as key=value, or as --key value):\n"
           "  port=7727              UDP port (0: any free one)\n"
           "  bind=                  address to listen on (empty: all)\n"
           "  name=IRON RING SERVER  shown in the lobby\n"
           "  motd=                  message of the day, one line\n"
           "  maxPlayers=256  maxRooms=128  roomsPerAddress=3  idleMinutes=30  joinAttemptsPerMinute=5\n"
           "  verify=1               referee fights with the game's simulation (needs data)\n"
           "  data=IronRing_Data     the game's data folder (relative to the executable)\n"
           "  log=                   also append the log to this file\n"
           "\n"
           "  --check    open the port, print what would run, and exit (0 = ok)\n"
           "  --version  print the version\n";
}

} // namespace iron::server
