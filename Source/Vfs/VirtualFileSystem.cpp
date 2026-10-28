#include "Vfs/VirtualFileSystem.h"

#include "Vfs/ArchiveFormat.h"

#include <ranges>
#include <stdexcept>
#include <string>

namespace iron::vfs {

void VirtualFileSystem::mount(std::unique_ptr<Mount> mount) {
    mounts_.push_back(std::move(mount));
}

bool VirtualFileSystem::exists(std::string_view path) const {
    const auto normalized = normalizePath(path);
    if (normalized.empty()) return false;
    for (const auto& m : std::views::reverse(mounts_))
        if (m->contains(normalized)) return true;
    return false;
}

std::optional<Bytes> VirtualFileSystem::tryRead(std::string_view path) const {
    const auto normalized = normalizePath(path);
    if (normalized.empty()) return std::nullopt;
    for (const auto& m : std::views::reverse(mounts_))
        if (m->contains(normalized)) return m->read(normalized);
    return std::nullopt;
}

Bytes VirtualFileSystem::read(std::string_view path) const {
    auto bytes = tryRead(path);
    if (!bytes) throw std::runtime_error("Asset not found: '" + std::string(path) + "'");
    return std::move(*bytes);
}

std::string VirtualFileSystem::readText(std::string_view path) const {
    const auto bytes = read(path);
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

} // namespace iron::vfs
