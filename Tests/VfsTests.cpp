#include "Vfs/ArchiveFormat.h"
#include "Vfs/ArchiveWriter.h"
#include "Vfs/DirectoryMount.h"
#include "Vfs/VirtualFileSystem.h"
#include "Vfs/ZipMount.h"

#include <doctest/doctest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace iron::vfs;
namespace fs = std::filesystem;

namespace {

Bytes bytesOf(std::string_view text) {
    Bytes b(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) b[i] = static_cast<std::byte>(text[i]);
    return b;
}

std::string textOf(const Bytes& b) { return {reinterpret_cast<const char*>(b.data()), b.size()}; }

Bytes fileBytes(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return bytesOf(s);
}

struct TempDir {
    fs::path path;
    TempDir() : path(fs::temp_directory_path() / ("ironring-test-" + std::to_string(std::rand()))) {
        fs::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

} // namespace

TEST_CASE("virtual paths are normalised and sandboxed") {
    CHECK(normalizePath("Fighters/Mara/atlas.png") == "Fighters/Mara/atlas.png");
    CHECK(normalizePath("Fighters\\Mara\\atlas.png") == "Fighters/Mara/atlas.png");
    CHECK(normalizePath("./Fighters//Mara/./atlas.png") == "Fighters/Mara/atlas.png");
    CHECK(normalizePath("../secrets.txt").empty());
    CHECK(normalizePath("Fighters/../../x").empty());
    CHECK(normalizePath("/etc/passwd").empty());
    CHECK(normalizePath("C:/Windows").empty());
    CHECK(normalizePath("").empty());
}

TEST_CASE("pre-compressed formats are stored, others deflated") {
    CHECK(isPrecompressed("Stages/a.PNG"));
    CHECK(isPrecompressed("Audio/theme.ogg"));
    CHECK_FALSE(isPrecompressed("Data/moves.json"));
    CHECK_FALSE(isPrecompressed("Audio/hit.wav"));
}

TEST_CASE("archive round trip through ArchiveWriter and ZipMount") {
    TempDir dir;
    const auto archive = dir.path / "sharedassets0.assets";

    ArchiveWriter writer("sharedassets0.assets");
    writer.add("Data/moves.json", bytesOf(R"({"jab":6})"));
    writer.add("Stages/pier.png", bytesOf("not really a png"));
    CHECK_THROWS_AS(writer.add("Data/moves.json", {}), std::invalid_argument);
    CHECK_THROWS_AS(writer.add("../escape", {}), std::invalid_argument);
    writer.write(archive);

    ZipMount mount(archive);
    CHECK(mount.contains("Data/moves.json"));
    CHECK_FALSE(mount.contains("Data/missing.json"));
    CHECK(textOf(*mount.read("Data/moves.json")) == R"({"jab":6})");
    CHECK(mount.list().size() == 2);
}

TEST_CASE("archives are byte-identical for identical inputs") {
    TempDir dir;
    const auto build = [&](const fs::path& out) {
        ArchiveWriter w("resources.assets");
        w.add("b.txt", bytesOf("B"));
        w.add("a.txt", bytesOf("A"));
        w.write(out);
    };
    build(dir.path / "one.assets");
    build(dir.path / "two.assets");
    CHECK(fileBytes(dir.path / "one.assets") == fileBytes(dir.path / "two.assets"));
}

TEST_CASE("a plain ZIP without the archive header is rejected") {
    TempDir dir;
    const auto bogus = dir.path / "bogus.assets";
    std::ofstream(bogus, std::ios::binary) << "PK this is not a zip";
    CHECK_THROWS(ZipMount(bogus));
    CHECK_THROWS(ZipMount(dir.path / "missing.assets"));
}

TEST_CASE("later mounts override earlier ones") {
    TempDir dir;
    ArchiveWriter base("resources.assets");
    base.add("UI/title.txt", bytesOf("packed"));
    base.add("UI/only-packed.txt", bytesOf("p"));
    base.write(dir.path / "resources.assets");

    fs::create_directories(dir.path / "Loose/UI");
    std::ofstream(dir.path / "Loose/UI/title.txt") << "loose";

    VirtualFileSystem vfs;
    vfs.mount(std::make_unique<ZipMount>(dir.path / "resources.assets"));
    vfs.mount(std::make_unique<DirectoryMount>(dir.path / "Loose"));

    CHECK(vfs.readText("UI/title.txt") == "loose");
    CHECK(vfs.readText("UI/only-packed.txt") == "p");
    CHECK(vfs.exists("UI\\title.txt"));
    CHECK_FALSE(vfs.exists("../Loose/UI/title.txt"));
    CHECK_THROWS_AS((void)vfs.read("UI/missing.txt"), std::runtime_error);
}
