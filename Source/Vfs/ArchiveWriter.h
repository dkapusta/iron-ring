#pragma once

#include "Vfs/Mount.h"

#include <filesystem>
#include <map>
#include <string>

namespace iron::vfs {

// Builds one archive in memory and writes it atomically.
// Output is deterministic: entries are sorted by path and carry a fixed
// timestamp, so identical inputs produce byte-identical archives.
class ArchiveWriter {
public:
    explicit ArchiveWriter(std::string archiveName);

    // Throws std::invalid_argument on an invalid or duplicate virtual path.
    void add(std::string_view virtualPath, Bytes data);
    [[nodiscard]] std::size_t size() const { return entries_.size(); }

    // Throws std::runtime_error on failure. Writes "<path>.tmp" then renames.
    void write(const std::filesystem::path& path) const;

private:
    std::string archiveName_;
    std::map<std::string, Bytes> entries_;
};

} // namespace iron::vfs
