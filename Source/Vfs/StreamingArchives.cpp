#include "Vfs/StreamingArchives.h"

#include "Vfs/ArchiveFormat.h"
#include "Vfs/ZipMount.h"

#include <algorithm>
#include <stdexcept>

namespace iron::vfs {

StreamingArchives::StreamingArchives(VirtualFileSystem& vfs) : vfs_(vfs), position_(vfs.mounts().size()) {}

void StreamingArchives::add(const std::filesystem::path& archive) {
    const ZipMount zip(archive); // opened only to read the index
    const auto index = archives_.size();
    for (const auto& path : zip.list()) {
        const auto [it, inserted] = owner_.emplace(path, index);
        if (!inserted)
            throw std::runtime_error(
                "'" + path + "' is in two streamed archives: " + archives_[it->second].path.filename().string() +
                " and " + archive.filename().string());
    }
    archives_.push_back({archive, nullptr, 0});
}

bool StreamingArchives::streams(std::string_view path) const {
    return owner_.count(normalizePath(path)) != 0;
}

void StreamingArchives::acquire(const std::string& key, const std::vector<std::string>& paths) {
    std::vector<std::size_t> needed;
    for (const auto& p : paths)
        if (const auto it = owner_.find(normalizePath(p)); it != owner_.end()) needed.push_back(it->second);
    std::sort(needed.begin(), needed.end());
    needed.erase(std::unique(needed.begin(), needed.end()), needed.end());

    for (const auto i : needed) {
        auto& a = archives_[i];
        if (a.users++ == 0) a.mount = &vfs_.insert(position_, std::make_unique<ZipMount>(a.path));
    }
    release(key); // after mounting the new set: archives in both stay mounted
    holds_.emplace(key, std::move(needed));
}

void StreamingArchives::release(const std::string& key) {
    const auto it = holds_.find(key);
    if (it == holds_.end()) return;
    for (const auto i : it->second) {
        auto& a = archives_[i];
        if (--a.users == 0) {
            vfs_.unmount(*a.mount);
            a.mount = nullptr;
        }
    }
    holds_.erase(it);
}

std::size_t StreamingArchives::mountedCount() const {
    return static_cast<std::size_t>(
        std::count_if(archives_.begin(), archives_.end(), [](const Archive& a) { return a.mount != nullptr; }));
}

} // namespace iron::vfs
