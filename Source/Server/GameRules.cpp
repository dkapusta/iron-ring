#include "Server/GameRules.h"

#include "Content/Fighter.h"
#include "Content/Stage.h"
#include "Net/Net.h"
#include "Vfs/ZipMount.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>

namespace iron::server {
namespace {

nlohmann::json readJson(const vfs::VirtualFileSystem& vfs, const std::string& path) {
    auto parsed = nlohmann::json::parse(vfs.readText(path), nullptr, false);
    if (parsed.is_discarded()) throw std::runtime_error(path + ": malformed JSON");
    return parsed;
}

bool isArchive(const std::filesystem::path& p) {
    const auto name = p.filename().string();
    const auto ext = p.extension().string();
    return ext == ".assets" || ext == ".resource" || name == "globalgamemanagers" || name.rfind("level", 0) == 0;
}

} // namespace

GameRules loadGameRules(const vfs::VirtualFileSystem& vfs, std::string version) {
    GameRules rules;
    rules.version = std::move(version);
    const auto settings = readJson(vfs, "ProjectSettings/GameSettings.json");
    std::vector<std::pair<std::string, std::uint64_t>> fighterHashes;
    for (const auto& item : settings.at("fighters")) {
        const auto path = item.get<std::string>();
        auto definition = content::parseFighter(readJson(vfs, path), path);
        fighterHashes.emplace_back(definition.id, sim::specHash(definition.spec));
        rules.fighters.emplace(definition.id, definition.spec);
    }
    const auto stagesPath = settings.at("stages").get<std::string>();
    const auto catalog = readJson(vfs, stagesPath);
    std::vector<std::pair<std::string, int>> stageWidths;
    for (const auto& s : catalog.at("stages")) {
        const auto stage = content::parseStage(s, stagesPath);
        stageWidths.emplace_back(stage.id, stage.width);
        rules.stages.emplace(stage.id, stage.width);
    }
    rules.dataHash = net::gameDataHash(std::move(fighterHashes), std::move(stageWidths));
    return rules;
}

GameRules loadGameRules(const std::filesystem::path& dataFolder, std::string version) {
    if (!std::filesystem::is_directory(dataFolder))
        throw std::runtime_error("game data folder not found: " + dataFolder.string());
    std::vector<std::filesystem::path> archives;
    for (const auto& e : std::filesystem::directory_iterator(dataFolder))
        if (e.is_regular_file() && isArchive(e.path())) archives.push_back(e.path());
    std::sort(archives.begin(), archives.end());
    vfs::VirtualFileSystem vfs;
    for (const auto& a : archives)
        vfs.mount(std::make_unique<vfs::ZipMount>(a));
    return loadGameRules(vfs, std::move(version));
}

} // namespace iron::server
