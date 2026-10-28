#include "Vfs/ZipMount.h"

#include "Vfs/ArchiveFormat.h"

#include <miniz.h>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <stdexcept>

namespace iron::vfs {
namespace {

std::FILE* openForReading(const std::filesystem::path& path) {
#if defined(_WIN32)
    return _wfopen(path.c_str(), L"rb");
#else
    return std::fopen(path.c_str(), "rb");
#endif
}

} // namespace

struct ZipMount::State {
    mz_zip_archive zip{};
    std::FILE* file = nullptr;

    ~State() {
        mz_zip_reader_end(&zip);
        if (file) std::fclose(file);
    }
};

ZipMount::ZipMount(std::filesystem::path archivePath)
    : path_(std::move(archivePath)), state_(std::make_unique<State>()) {
    const auto fail = [this](const std::string& why) {
        throw std::runtime_error("Cannot mount archive '" + path_.string() + "': " + why);
    };

    state_->file = openForReading(path_);
    if (!state_->file) fail("file not found or unreadable");
    if (!mz_zip_reader_init_cfile(&state_->zip, state_->file, 0, 0))
        fail(mz_zip_get_error_string(mz_zip_get_last_error(&state_->zip)));

    const mz_uint count = mz_zip_reader_get_num_files(&state_->zip);
    for (mz_uint i = 0; i < count; ++i) {
        if (mz_zip_reader_is_file_a_directory(&state_->zip, i)) continue;
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&state_->zip, i, &stat)) fail("corrupt central directory");
        index_.emplace(stat.m_filename, i);
    }

    const auto header = read(kHeaderEntryName);
    if (!header) fail("missing archive header (not built by AssetPacker?)");
    const auto json = nlohmann::json::parse(
        reinterpret_cast<const char*>(header->data()),
        reinterpret_cast<const char*>(header->data() + header->size()), nullptr, false);
    if (json.is_discarded() || json.value("format", 0) != kFormatVersion)
        fail("unsupported archive format version");
}

ZipMount::~ZipMount() = default;

std::string ZipMount::describe() const {
    return path_.filename().string() + " (" + std::to_string(index_.size() - 1) + " files)";
}

bool ZipMount::contains(std::string_view path) const {
    return index_.find(std::string(path)) != index_.end();
}

std::optional<Bytes> ZipMount::read(std::string_view path) const {
    const auto it = index_.find(std::string(path));
    if (it == index_.end()) return std::nullopt;

    std::scoped_lock lock(mutex_);
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&state_->zip, it->second, &stat)) return std::nullopt;

    Bytes bytes(static_cast<std::size_t>(stat.m_uncomp_size));
    if (!mz_zip_reader_extract_to_mem(&state_->zip, it->second, bytes.data(), bytes.size(), 0))
        return std::nullopt;
    return bytes;
}

std::vector<std::string> ZipMount::list() const {
    std::vector<std::string> paths;
    paths.reserve(index_.size());
    for (const auto& [path, _] : index_)
        if (path != kHeaderEntryName) paths.push_back(path);
    return paths;
}

} // namespace iron::vfs
