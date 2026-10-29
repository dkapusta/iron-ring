#pragma once

#include <string>
#include <string_view>

// Shared conventions of the asset archives (docs/adr/0004-asset-archives.md).
//
// An archive is a standard ZIP file with a Unity-flavoured file name
// (resources.assets, sharedassets0.assets, sharedassets0.resource,
// globalgamemanagers). Its first entry is a small JSON header that marks the
// file as an Iron Ring archive and records the format version.
namespace iron::vfs {

inline constexpr std::string_view kHeaderEntryName = ".archive";
inline constexpr int kFormatVersion = 1;

// Virtual paths are relative, '/'-separated and case-sensitive:
//   "Fighters/Dee/dee_pose_atlas.png"
// Returns an empty string when the path is invalid (absolute, contains "..",
// or empty after normalisation).
[[nodiscard]] std::string normalizePath(std::string_view path);

// Already-compressed formats are stored without deflate to keep loads cheap.
[[nodiscard]] bool isPrecompressed(std::string_view path);

} // namespace iron::vfs
