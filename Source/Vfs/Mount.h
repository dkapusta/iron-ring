#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace iron::vfs {

using Bytes = std::vector<std::byte>;

// A read-only source of files addressed by normalised virtual paths.
// Implementations must be safe to call from multiple threads.
class Mount {
public:
    virtual ~Mount() = default;

    [[nodiscard]] virtual std::string describe() const = 0;
    [[nodiscard]] virtual bool contains(std::string_view path) const = 0;
    [[nodiscard]] virtual std::optional<Bytes> read(std::string_view path) const = 0;
    [[nodiscard]] virtual std::vector<std::string> list() const = 0;
};

} // namespace iron::vfs
