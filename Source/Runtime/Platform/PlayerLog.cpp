#include "Platform/PlayerLog.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <mutex>

namespace iron {
namespace {

struct LogState {
    std::mutex mutex;
    std::FILE* file = nullptr;
    SDL_LogOutputFunction previous = nullptr;
    void* previousUserdata = nullptr;
};

LogState& state() {
    static LogState s;
    return s;
}

const char* priorityName(SDL_LogPriority priority) {
    switch (priority) {
    case SDL_LOG_PRIORITY_TRACE:    return "TRACE";
    case SDL_LOG_PRIORITY_VERBOSE:  return "VERBOSE";
    case SDL_LOG_PRIORITY_DEBUG:    return "DEBUG";
    case SDL_LOG_PRIORITY_INFO:     return "INFO";
    case SDL_LOG_PRIORITY_WARN:     return "WARN";
    case SDL_LOG_PRIORITY_ERROR:    return "ERROR";
    case SDL_LOG_PRIORITY_CRITICAL: return "CRITICAL";
    default:                        return "LOG";
    }
}

void SDLCALL writeLog(void* /*userdata*/, int category, SDL_LogPriority priority, const char* message) {
    auto& s = state();
    std::scoped_lock lock(s.mutex);
    if (s.file) {
        std::fprintf(s.file, "[%s] %s\n", priorityName(priority), message);
        std::fflush(s.file);
    }
    if (s.previous) s.previous(s.previousUserdata, category, priority, message);
}

std::FILE* openForWriting(const std::filesystem::path& path) {
#if defined(_WIN32)
    return _wfopen(path.c_str(), L"w");
#else
    return std::fopen(path.c_str(), "w");
#endif
}

} // namespace

PlayerLog::PlayerLog(const std::filesystem::path& explicitPath, const char* company, const char* product) {
    if (!explicitPath.empty()) {
        path_ = explicitPath;
    } else if (char* pref = SDL_GetPrefPath(company, product)) {
        path_ = std::filesystem::path(reinterpret_cast<const char8_t*>(pref)) / "Player.log";
        SDL_free(pref);
        std::error_code ec;
        if (std::filesystem::exists(path_, ec))
            std::filesystem::rename(path_, path_.parent_path() / "Player-prev.log", ec);
    }

    auto& s = state();
    SDL_GetLogOutputFunction(&s.previous, &s.previousUserdata);
    if (!path_.empty()) s.file = openForWriting(path_);
    SDL_SetLogOutputFunction(writeLog, nullptr);
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
}

PlayerLog::~PlayerLog() {
    auto& s = state();
    SDL_SetLogOutputFunction(s.previous, s.previousUserdata);
    std::scoped_lock lock(s.mutex);
    if (s.file) std::fclose(s.file);
    s.file = nullptr;
}

} // namespace iron
