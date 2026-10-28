// AssetPacker: bakes Assets/ and ProjectSettings/ into the player's data
// archives, or lists the contents of an existing archive.
//
//   AssetPacker --project <dir> --pipeline <AssetPipeline.json> --output <IronRing_Data>
//   AssetPacker --list <archive>

#include "Pipeline.h"

#include "Vfs/ArchiveWriter.h"
#include "Vfs/ZipMount.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace fs = std::filesystem;
using namespace iron;

namespace {

// miniz converts timestamps with localtime(); pin UTC for reproducible output.
void pinTimezoneToUtc() {
#if defined(_WIN32)
    _putenv_s("TZ", "UTC0");
    _tzset();
#else
    setenv("TZ", "UTC0", 1);
    tzset();
#endif
}

vfs::Bytes readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("Cannot read '" + path.string() + "'");
    vfs::Bytes bytes(static_cast<std::size_t>(in.tellg()));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return bytes;
}

int usage() {
    std::cerr << "usage:\n"
                 "  AssetPacker --project <dir> --pipeline <AssetPipeline.json> --output <dir>\n"
                 "  AssetPacker --list <archive>\n";
    return 2;
}

int listArchive(const fs::path& archive) {
    const vfs::ZipMount mount(archive);
    auto paths = mount.list();
    std::sort(paths.begin(), paths.end());
    for (const auto& p : paths) std::cout << p << '\n';
    std::cout << mount.describe() << '\n';
    return 0;
}

int pack(const fs::path& project, const fs::path& pipelineFile, const fs::path& output) {
    pinTimezoneToUtc();
    const auto config = tools::loadPipeline(pipelineFile);
    const auto plan = tools::planPack(config, project);
    for (const auto& w : plan.warnings) std::cout << "AssetPacker: warning: " << w << '\n';

    fs::create_directories(output);
    for (std::size_t i = 0; i < config.archives.size(); ++i) {
        const auto& rule = config.archives[i];
        vfs::ArchiveWriter writer(rule.output);
        std::uintmax_t bytes = 0;
        for (const auto& file : plan.archives[i]) {
            auto data = readFile(file.source);
            bytes += data.size();
            writer.add(file.virtualPath, std::move(data));
        }
        writer.write(output / rule.output);
        std::cout << "AssetPacker: " << rule.output << ": " << writer.size() << " files, "
                  << (bytes + 1023) / 1024 << " KiB source\n";
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::map<std::string, fs::path> options;
        for (int i = 1; i + 1 < argc; i += 2) options[argv[i]] = fs::path(argv[i + 1]);
        if (argc % 2 == 0) return usage();

        if (options.size() == 1 && options.contains("--list")) return listArchive(options["--list"]);
        if (options.size() == 3 && options.contains("--project") && options.contains("--pipeline") &&
            options.contains("--output"))
            return pack(options["--project"], options["--pipeline"], options["--output"]);
        return usage();
    } catch (const std::exception& e) {
        std::cerr << "AssetPacker: error: " << e.what() << '\n';
        return 1;
    }
}
