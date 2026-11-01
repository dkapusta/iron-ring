#pragma once

// server.cfg and the command line (docs/server.md): the same keys, in
// boot.config's key=value syntax, the command line winning.

#include "Server/Server.h"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace iron::server {

struct Settings {
    Config server;
    std::filesystem::path data = "IronRing_Data"; // relative to the executable's folder unless absolute
    std::filesystem::path log;                    // also written here, when set
    std::filesystem::path config;                 // the server.cfg that was read, if any
    bool check = false;                           // --check: open the port, report, exit
    bool help = false, version = false;
};

// One key. False if the key is unknown or the value invalid.
bool applySetting(Settings& settings, std::string_view key, std::string_view value);
// key=value text (server.cfg). Returns warnings for lines it ignored.
std::vector<std::string> applyConfigText(Settings& settings, std::string_view text);
// --key value / --key=value / --check / --help / --version / --config <file>.
// Returns errors; the config file is read by the caller in between (see main.cpp).
std::vector<std::string> applyArguments(Settings& settings, std::span<const std::string> args);

[[nodiscard]] std::string usage();

} // namespace iron::server
