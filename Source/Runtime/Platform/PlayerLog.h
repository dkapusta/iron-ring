#pragma once

#include <filesystem>

namespace iron {

// Mirrors SDL_Log output into Player.log, like a Unity player.
// Default location: <SDL pref path for company/product>/Player.log; the
// previous session is kept as Player-prev.log.
class PlayerLog {
public:
    PlayerLog(const std::filesystem::path& explicitPath, const char* company, const char* product);
    ~PlayerLog();

    PlayerLog(const PlayerLog&) = delete;
    PlayerLog& operator=(const PlayerLog&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace iron
