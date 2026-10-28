#include "Vfs/ArchiveWriter.h"

#include "Vfs/ArchiveFormat.h"

#include <miniz.h>
#include <nlohmann/json.hpp>

#include <ctime>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace iron::vfs {
namespace {

// 2000-01-01 00:00:00. miniz converts through localtime(), so AssetPacker
// pins TZ=UTC before writing to keep archives byte-identical across machines.
constexpr MZ_TIME_T kFixedTimestamp = 946684800;

} // namespace

ArchiveWriter::ArchiveWriter(std::string archiveName) : archiveName_(std::move(archiveName)) {}

void ArchiveWriter::add(std::string_view virtualPath, Bytes data) {
    auto path = normalizePath(virtualPath);
    if (path.empty() || path == kHeaderEntryName)
        throw std::invalid_argument("Invalid virtual path: '" + std::string(virtualPath) + "'");
    if (!entries_.emplace(std::move(path), std::move(data)).second)
        throw std::invalid_argument("Duplicate virtual path: '" + std::string(virtualPath) + "'");
}

void ArchiveWriter::write(const std::filesystem::path& path) const {
    mz_zip_archive zip{};
    if (!mz_zip_writer_init_heap(&zip, 0, 1u << 20))
        throw std::runtime_error("miniz: cannot initialise archive writer");
    const auto cleanup = std::unique_ptr<mz_zip_archive, decltype(&mz_zip_writer_end)>(&zip, mz_zip_writer_end);

    const auto addEntry = [&](const std::string& name, const void* data, std::size_t size, mz_uint level) {
        MZ_TIME_T timestamp = kFixedTimestamp;
        if (!mz_zip_writer_add_mem_ex_v2(&zip, name.c_str(), data, size, nullptr, 0, level, 0, 0,
                                         &timestamp, nullptr, 0, nullptr, 0))
            throw std::runtime_error("miniz: cannot add '" + name + "': " +
                                     mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
    };

    const std::string header = nlohmann::json{
        {"format", kFormatVersion},
        {"archive", archiveName_},
        {"entries", entries_.size()},
    }.dump();
    addEntry(std::string(kHeaderEntryName), header.data(), header.size(), MZ_NO_COMPRESSION);

    for (const auto& [name, bytes] : entries_) {
        const mz_uint level = isPrecompressed(name) ? MZ_NO_COMPRESSION : MZ_DEFAULT_LEVEL;
        addEntry(name, bytes.data(), bytes.size(), level);
    }

    void* buffer = nullptr;
    std::size_t size = 0;
    if (!mz_zip_writer_finalize_heap_archive(&zip, &buffer, &size))
        throw std::runtime_error("miniz: cannot finalise archive");
    const auto release = std::unique_ptr<void, decltype(&mz_free)>(buffer, mz_free);

    const auto temp = std::filesystem::path(path).concat(".tmp");
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        out.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(size));
        if (!out) throw std::runtime_error("Cannot write '" + temp.string() + "'");
    }
    std::filesystem::rename(temp, path);
}

} // namespace iron::vfs
