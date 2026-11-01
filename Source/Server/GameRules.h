#pragma once

// Builds the server's GameRules from game data: the same Fighters and stages
// the player loads, so both compute the same net::gameDataHash().

#include "Server/Server.h"
#include "Vfs/VirtualFileSystem.h"

#include <filesystem>
#include <string>

namespace iron::server {

// From a mounted VFS holding ProjectSettings/GameSettings.json and what it
// names. Throws std::runtime_error with the file and the problem.
[[nodiscard]] GameRules loadGameRules(const vfs::VirtualFileSystem& vfs, std::string version);

// Mounts every archive in a player data folder (IronRing_Data) and loads from it.
[[nodiscard]] GameRules loadGameRules(const std::filesystem::path& dataFolder, std::string version);

} // namespace iron::server
