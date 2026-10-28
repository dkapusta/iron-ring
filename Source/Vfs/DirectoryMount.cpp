#include "Vfs/DirectoryMount.h"

#include "Vfs/ArchiveFormat.h"

#include <fstream>
#include <stdexcept>

namespace iron::vfs {

DirectoryMount::DirectoryMount(std::filesystem::path root) : root_(std::move(root)) {
    if (!std::filesystem::is_directory(root_))
        throw std::runtime_error("Cannot mount folder '" + root_.string() + "': not a directory");
}

std::string DirectoryMount::describe() const {
    return root_.string() + " (loose folder)";
}

std::filesystem::path DirectoryMount::resolve(std::string_view path) const {
    const auto normalized = normalizePath(path);
    if (normalized.empty()) return {};
    return root_ / std::filesystem::path(std::u8string(normalized.begin(), normalized.end()));
}

bool DirectoryMount::contains(std::string_view path) const {
    const auto full = resolve(path);
    std::error_code ec;
    return !full.empty() && std::filesystem::is_regular_file(full, ec);
}

std::optional<Bytes> DirectoryMount::read(std::string_view path) const {
    const auto full = resolve(path);
    if (full.empty()) return std::nullopt;
    std::ifstream file(full, std::ios::binary | std::ios::ate);
    if (!file) return std::nullopt;
    Bytes bytes(static_cast<std::size_t>(file.tellg()));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!file) return std::nullopt;
    return bytes;
}

std::vector<std::string> DirectoryMount::list() const {
    std::vector<std::string> paths;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root_)) {
        if (!entry.is_regular_file()) continue;
        const auto relative = entry.path().lexically_relative(root_).generic_u8string();
        paths.emplace_back(relative.begin(), relative.end());
    }
    return paths;
}

} // namespace iron::vfs
