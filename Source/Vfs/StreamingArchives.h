#pragma once

#include "Vfs/VirtualFileSystem.h"

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace iron::vfs {

// Archives mounted on demand (per-stage sharedassetsN.assets and levelN,
// docs/asset-pipeline.md). Each archive is opened once at start-up to index
// its files, then closed. A user holds a set of virtual paths under a key
// (a stage id); the archives that contain them stay mounted until no key
// needs them. Streamed archives go into the mount stack at a fixed position,
// below anything mounted after the constructor ran (the loose dev folder),
// so overrides keep working.
class StreamingArchives {
  public:
    explicit StreamingArchives(VirtualFileSystem& vfs);

    // Indexes an archive. Throws if it cannot be opened or claims a path
    // another streamed archive already has.
    void add(const std::filesystem::path& archive);

    // True if the path lives in a streamed archive (mounted or not).
    [[nodiscard]] bool streams(std::string_view path) const;

    // Mounts the archives holding these paths and records them under the key.
    // Paths outside every streamed archive are ignored (base archives, loose
    // files). Acquiring an existing key replaces its set.
    void acquire(const std::string& key, const std::vector<std::string>& paths);
    // Drops the key; archives no other key needs are unmounted.
    void release(const std::string& key);

    [[nodiscard]] std::size_t archiveCount() const { return archives_.size(); }
    [[nodiscard]] std::size_t mountedCount() const;
    [[nodiscard]] bool held(const std::string& key) const { return holds_.count(key) != 0; }

  private:
    struct Archive {
        std::filesystem::path path;
        const Mount* mount = nullptr; // non-null while mounted
        int users = 0;
    };

    VirtualFileSystem& vfs_;
    std::size_t position_; // insertion point in the mount stack
    std::vector<Archive> archives_;
    std::unordered_map<std::string, std::size_t> owner_;    // virtual path -> archive index
    std::map<std::string, std::vector<std::size_t>> holds_; // key -> archive indices
};

} // namespace iron::vfs
