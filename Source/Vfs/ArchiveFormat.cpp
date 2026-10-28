#include "Vfs/ArchiveFormat.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace iron::vfs {

std::string normalizePath(std::string_view path) {
    std::string result;
    result.reserve(path.size());

    std::size_t start = 0;
    while (start <= path.size()) {
        auto end = path.find_first_of("/\\", start);
        if (end == std::string_view::npos) end = path.size();
        const auto segment = path.substr(start, end - start);
        start = end + 1;

        if (segment.empty() || segment == ".") continue;
        if (segment == ".." || segment.find(':') != std::string_view::npos) return {};
        if (!result.empty()) result += '/';
        result += segment;
    }
    // A leading separator means an absolute path: reject it rather than guess.
    if (!path.empty() && (path.front() == '/' || path.front() == '\\')) return {};
    return result;
}

bool isPrecompressed(std::string_view path) {
    constexpr std::array<std::string_view, 7> extensions{".png", ".jpg", ".jpeg", ".ogg", ".flac", ".mp3", ".zip"};
    const auto dot = path.rfind('.');
    if (dot == std::string_view::npos) return false;
    std::string ext(path.substr(dot));
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

} // namespace iron::vfs
