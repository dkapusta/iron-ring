#include "IronRing/Player.h"

#include "Core/PlayerPrefs.h"
#include "Platform/PlayerLog.h"
#include "Player/Application.h"

#include <SDL3/SDL.h>
// main() lives in the launcher; the engine only declares itself ready.
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>

#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
namespace fs = std::filesystem;
using namespace iron;

fs::path fromUtf8(const char* s) { return fs::path(reinterpret_cast<const char8_t*>(s)); }

fs::path executableFolder() {
    const char* base = SDL_GetBasePath(); // owned by SDL, cached
    return base ? fromUtf8(base) : fs::current_path();
}

// IRONRING_USER_FOLDER replaces the per-user folder (prefs, screenshots, replays), so test
// runs keep their files apart on every platform (Tools/Server/soak.sh).
fs::path userFolder(const ProductInfo& info) {
    if (const char* forced = SDL_getenv("IRONRING_USER_FOLDER"); forced && *forced) {
        std::error_code ignored;
        fs::create_directories(fromUtf8(forced), ignored);
        return fromUtf8(forced);
    }
    char* pref = SDL_GetPrefPath(info.company.c_str(), info.product.c_str());
    if (!pref) return fs::current_path();
    fs::path path = fromUtf8(pref);
    SDL_free(pref);
    return path;
}

std::string readText(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// <name>_Data/app.info: line 1 company, line 2 product (same as Unity).
ProductInfo readAppInfo(const fs::path& dataFolder) {
    ProductInfo info{IR_COMPANY_NAME, IR_PRODUCT_DISPLAY_NAME, {}};
    std::ifstream in(dataFolder / "app.info");
    std::string company, product;
    if (std::getline(in, company) && std::getline(in, product) && !company.empty() && !product.empty()) {
        info.company = company;
        info.product = product;
    }
    return info;
}

int runPlayer(const std::vector<std::string>& args) {
    SDL_SetMainReady();

    // The data folder can come from the command line, and boot.config lives
    // inside it, so resolve it before applying boot.config.
    PlayerSettings probe;
    (void)applyCommandLine(probe, args);
    const fs::path dataFolder = probe.dataFolder.empty() ? executableFolder() / IR_DATA_DIR_NAME : probe.dataFolder;

    auto info = readAppInfo(dataFolder);
    info.userFolder = userFolder(info);

    // defaults < boot.config < prefs.cfg (the Options screen) < command line.
    // Batch runs ignore prefs.cfg, so they come out the same on every machine.
    PlayerSettings settings;
    settings.dataFolder = dataFolder;
    auto warnings = applyBootConfig(settings, parseKeyValueText(readText(dataFolder / "boot.config")));
    PlayerPrefs prefs(probe.batchMode ? fs::path{} : info.userFolder / "prefs.cfg");
    if (!probe.batchMode) {
        const auto loadWarnings = prefs.load();
        const auto prefWarnings = prefs.applyTo(settings);
        warnings.insert(warnings.end(), loadWarnings.begin(), loadWarnings.end());
        warnings.insert(warnings.end(), prefWarnings.begin(), prefWarnings.end());
    }
    const auto argWarnings = applyCommandLine(settings, args);
    warnings.insert(warnings.end(), argWarnings.begin(), argWarnings.end());
    const PlayerLog log(settings.logFile, info.company.c_str(), info.product.c_str());
    SDL_Log("%s %s (engine %s), SDL %d.%d.%d", info.product.c_str(), IR_VERSION_STRING, IR_PLAYER_LIBRARY_NAME,
            SDL_VERSIONNUM_MAJOR(SDL_GetVersion()), SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
            SDL_VERSIONNUM_MICRO(SDL_GetVersion()));
    SDL_Log("Data folder: %s", dataFolder.string().c_str());
    if (!prefs.file().empty()) SDL_Log("Preferences: %s", prefs.file().string().c_str());
    for (const auto& w : warnings) SDL_Log("Warning: %s", w.c_str());

    try {
        Application app(settings, std::move(prefs), info);
        return app.run();
    } catch (const std::exception& e) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fatal: %s", e.what());
        if (!settings.batchMode) {
            const std::string message = std::string(e.what()) + "\n\nSee " + log.path().string();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, info.product.c_str(), message.c_str(), nullptr);
        }
        return 1;
    }
}

// The exported functions are a C ABI boundary: the launcher and the engine
// may each carry their own statically linked C++ runtime (MinGW, ADR-0009),
// so no exception may escape across it.
int guardedRun(const std::vector<std::string>& args) noexcept {
    try {
        return runPlayer(args);
    } catch (const std::exception& e) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fatal (startup): %s", e.what());
    } catch (...) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Fatal (startup): unknown exception");
    }
    return 1;
}

} // namespace

extern "C" IR_PLAYER_API int PlayerMain(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    return guardedRun(args);
}

#if defined(_WIN32)
extern "C" IR_PLAYER_API int UnityMain(void* /*hInstance*/, void* /*hPrevInstance*/, wchar_t* /*lpCmdLine*/,
                                       int /*nShowCmd*/) {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        const int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        std::string utf8(static_cast<std::size_t>(size > 0 ? size - 1 : 0), '\0');
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, utf8.data(), size, nullptr, nullptr);
        args.push_back(std::move(utf8));
    }
    LocalFree(argvW);
    return guardedRun(args);
}
#endif
