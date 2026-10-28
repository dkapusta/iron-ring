#pragma once

#include "Vfs/Mount.h"

#include <memory>
#include <string_view>
#include <vector>

namespace iron::vfs {

// Ordered stack of mounts. Lookups search the most recently added mount
// first, so later mounts (patches, mods, a loose dev folder) override earlier
// ones.
class VirtualFileSystem {
public:
    void mount(std::unique_ptr<Mount> mount);

    [[nodiscard]] bool exists(std::string_view path) const;
    [[nodiscard]] std::optional<Bytes> tryRead(std::string_view path) const;
    // Throws std::runtime_error naming the missing path.
    [[nodiscard]] Bytes read(std::string_view path) const;
    [[nodiscard]] std::string readText(std::string_view path) const;

    [[nodiscard]] const std::vector<std::unique_ptr<Mount>>& mounts() const { return mounts_; }

private:
    std::vector<std::unique_ptr<Mount>> mounts_;
};

} // namespace iron::vfs
