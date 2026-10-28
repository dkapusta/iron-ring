#pragma once

#include "Vfs/Mount.h"

#include <filesystem>

namespace iron::vfs {

// Mounts a loose folder (e.g. the source Assets/ tree) for fast iteration:
// edited files are visible without re-packing. Development use only.
class DirectoryMount final : public Mount {
public:
    explicit DirectoryMount(std::filesystem::path root);

    [[nodiscard]] std::string describe() const override;
    [[nodiscard]] bool contains(std::string_view path) const override;
    [[nodiscard]] std::optional<Bytes> read(std::string_view path) const override;
    [[nodiscard]] std::vector<std::string> list() const override;

private:
    [[nodiscard]] std::filesystem::path resolve(std::string_view path) const;

    std::filesystem::path root_;
};

} // namespace iron::vfs
