#pragma once

#include "Vfs/Mount.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace iron::vfs {

// Mounts one packed archive (*.assets, *.resource, globalgamemanagers).
// Throws std::runtime_error if the file is missing, not a ZIP, or lacks the
// Iron Ring archive header.
class ZipMount final : public Mount {
public:
    explicit ZipMount(std::filesystem::path archivePath);
    ~ZipMount() override;

    ZipMount(const ZipMount&) = delete;
    ZipMount& operator=(const ZipMount&) = delete;

    [[nodiscard]] std::string describe() const override;
    [[nodiscard]] bool contains(std::string_view path) const override;
    [[nodiscard]] std::optional<Bytes> read(std::string_view path) const override;
    [[nodiscard]] std::vector<std::string> list() const override;

private:
    struct State;
    std::filesystem::path path_;
    std::unique_ptr<State> state_;
    std::unordered_map<std::string, unsigned> index_;
    mutable std::mutex mutex_; // miniz readers are not thread-safe
};

} // namespace iron::vfs
