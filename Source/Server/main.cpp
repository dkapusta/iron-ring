// IronRingServer: the Iron Ring dedicated server (ADR-0026, docs/server.md).
// A console program with no SDL: lobby, rooms, relay and referee.

#include "Net/Net.h"
#include "Server/GameRules.h"
#include "Server/Server.h"
#include "Server/Settings.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

std::atomic<bool> stopRequested{false};

void onSignal(int) {
    stopRequested = true;
}

std::filesystem::path executableFolder(const char* argv0) {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) > 0) return std::filesystem::path(buffer).parent_path();
#else
    std::error_code ec;
    if (const auto self = std::filesystem::read_symlink("/proc/self/exe", ec); !ec) return self.parent_path();
#endif
    return std::filesystem::absolute(argv0).parent_path();
}

std::string timestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm t{};
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    localtime_r(&now, &t);
#endif
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &t);
    return buf;
}

} // namespace

int main(int argc, char** argv) {
    using namespace iron::server;
    const std::vector<std::string> args(argv + 1, argv + argc);
    const auto folder = executableFolder(argv[0]);

    // Defaults, then server.cfg (--config, or next to the executable), then the command line.
    Settings settings;
    Settings probe;
    (void)applyArguments(probe, args); // only to learn --config
    const auto config = probe.config.empty() ? folder / "server.cfg" : probe.config;
    std::vector<std::string> warnings;
    if (std::ifstream in(config); in) {
        std::stringstream text;
        text << in.rdbuf();
        warnings = applyConfigText(settings, text.str());
        settings.config = config;
    } else if (!probe.config.empty()) {
        std::cerr << "cannot read " << config.string() << "\n";
        return 2;
    }
    const auto errors = applyArguments(settings, args);
    if (settings.help) {
        std::cout << usage();
        return 0;
    }
    if (settings.version) {
        std::cout << "IronRingServer " << IR_VERSION_STRING << " (protocol " << iron::net::kProtocolVersion << ")\n";
        return 0;
    }
    if (!errors.empty()) {
        for (const auto& e : errors)
            std::cerr << e << "\n";
        std::cerr << "\n" << usage();
        return 2;
    }

    std::ofstream logFile;
    if (!settings.log.empty())
        logFile.open(settings.log.is_absolute() ? settings.log : folder / settings.log, std::ios::app);
    std::mutex logMutex;
    const auto log = [&](const std::string& line) {
        const std::lock_guard lock(logMutex);
        const auto stamped = "[" + timestamp() + "] " + line;
        std::cout << stamped << std::endl;
        if (logFile) logFile << stamped << std::endl;
    };
    log("IronRingServer " IR_VERSION_STRING " (protocol " + std::to_string(iron::net::kProtocolVersion) + ")" +
        (settings.config.empty() ? std::string{} : ", config " + settings.config.string()));
    for (const auto& w : warnings)
        log("server.cfg: " + w);

    GameRules rules;
    rules.version = IR_VERSION_STRING;
    if (settings.server.verify) {
        const auto data = settings.data.is_absolute() ? settings.data : folder / settings.data;
        try {
            rules = loadGameRules(data, IR_VERSION_STRING);
            log("game data " + data.string() + ": " + std::to_string(rules.fighters.size()) + " fighters, " +
                std::to_string(rules.stages.size()) + " stages");
        } catch (const std::exception& e) {
            log(std::string("cannot load the game data (") + e.what() +
                "); copy the game's IronRing_Data folder next to the server, set data=, or set verify=0");
            return 1;
        }
    }

    Server server(settings.server, std::move(rules), log);
    std::string error;
    if (!server.start(&error)) {
        log("error: " + error);
        return 1;
    }
    if (settings.check) {
        log("check ok: '" + settings.server.name + "' on UDP " + std::to_string(server.port()));
        return 0;
    }

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
#ifdef SIGBREAK
    std::signal(SIGBREAK, onSignal);
#endif
    auto lastReport = std::chrono::steady_clock::now();
    while (!stopRequested) {
        server.poll(20);
        if (std::chrono::steady_clock::now() - lastReport > std::chrono::minutes(10)) {
            lastReport = std::chrono::steady_clock::now();
            log(std::to_string(server.players()) + " players, " + std::to_string(server.rooms()) + " rooms");
        }
    }
    log("stopping");
    server.shutdown();
    return 0;
}
