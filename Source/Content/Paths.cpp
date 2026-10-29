#include "Content/Paths.h"

#include <array>
#include <cstdint>

namespace iron::content {

std::string siblingPath(std::string_view assetPath, std::string_view relative) {
    const auto slash = assetPath.rfind('/');
    if (slash == std::string_view::npos) return std::string(relative);
    return std::string(assetPath.substr(0, slash + 1)) + std::string(relative);
}

std::optional<std::pair<int, int>> pngSize(const std::vector<std::byte>& bytes) {
    constexpr std::array<unsigned char, 8> signature{0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    if (bytes.size() < 24) return std::nullopt;
    for (std::size_t i = 0; i < signature.size(); ++i)
        if (std::to_integer<unsigned char>(bytes[i]) != signature[i]) return std::nullopt;
    const auto be32 = [&](std::size_t at) {
        std::uint32_t v = 0;
        for (std::size_t i = 0; i < 4; ++i) v = (v << 8) | std::to_integer<std::uint32_t>(bytes[at + i]);
        return static_cast<int>(v);
    };
    // The first chunk is IHDR: length(4) type(4) width(4) height(4).
    return std::pair{be32(16), be32(20)};
}

} // namespace iron::content
