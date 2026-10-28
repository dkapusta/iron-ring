#include "Pipeline.h"

#include "Vfs/ArchiveFormat.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>

namespace iron::tools {
namespace fs = std::filesystem;
namespace {

std::string lowerExtension(const fs::path& p) {
    auto ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

bool contains(const std::vector<std::string>& list, const std::string& value) {
    return std::find(list.begin(), list.end(), value) != list.end();
}

// Editor and OS clutter that never ships.
bool isIgnored(const fs::path& file) {
    const auto name = file.filename().string();
    return name.empty() || name.front() == '.' || name == "Thumbs.db" || name == "desktop.ini" ||
           lowerExtension(file) == ".lnk";
}

std::string toVirtualPath(const fs::path& relative) {
    const auto u8 = relative.generic_u8string();
    return vfs::normalizePath(std::string(u8.begin(), u8.end()));
}

} // namespace

bool ArchiveRule::accepts(const fs::path& file) const {
    const auto ext = lowerExtension(file);
    if (!extensions.empty() && !contains(extensions, ext)) return false;
    return !contains(excludeExtensions, ext);
}

PipelineConfig loadPipeline(const fs::path& jsonFile) {
    std::ifstream in(jsonFile);
    if (!in) throw std::runtime_error("Cannot open pipeline file '" + jsonFile.string() + "'");
    const auto json = nlohmann::json::parse(in);

    PipelineConfig config;
    for (const auto& a : json.at("archives")) {
        ArchiveRule rule;
        rule.output = a.at("output").get<std::string>();
        rule.root = a.at("root").get<std::string>();
        for (const auto& folder : a.at("include")) rule.include.emplace_back(folder.get<std::string>());
        rule.extensions = a.value("extensions", std::vector<std::string>{});
        rule.excludeExtensions = a.value("excludeExtensions", std::vector<std::string>{});
        if (rule.output.empty() || rule.output.find_first_of("/\\") != std::string::npos)
            throw std::runtime_error("Invalid archive output name '" + rule.output + "'");
        config.archives.push_back(std::move(rule));
    }
    return config;
}

PackPlan planPack(const PipelineConfig& config, const fs::path& projectDir) {
    PackPlan plan;
    plan.archives.resize(config.archives.size());
    std::map<std::string, std::string> owner; // virtual path -> archive
    std::set<fs::path> claimed;

    for (std::size_t i = 0; i < config.archives.size(); ++i) {
        const auto& rule = config.archives[i];
        const auto root = fs::weakly_canonical(projectDir / rule.root);
        for (const auto& folder : rule.include) {
            const auto dir = root / folder;
            if (!fs::is_directory(dir))
                throw std::runtime_error(rule.output + ": include folder not found: " + dir.string());

            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (!entry.is_regular_file() || isIgnored(entry.path()) || !rule.accepts(entry.path())) continue;
                const auto virtualPath = toVirtualPath(entry.path().lexically_relative(root));
                if (virtualPath.empty())
                    throw std::runtime_error("Cannot map '" + entry.path().string() + "' to a virtual path");

                const auto [it, inserted] = owner.emplace(virtualPath, rule.output);
                if (!inserted)
                    throw std::runtime_error("'" + virtualPath + "' is claimed by both " + it->second +
                                             " and " + rule.output);
                claimed.insert(fs::weakly_canonical(entry.path()));
                plan.archives[i].push_back({entry.path(), virtualPath});
            }
        }
        std::sort(plan.archives[i].begin(), plan.archives[i].end(),
                  [](const PlannedFile& a, const PlannedFile& b) { return a.virtualPath < b.virtualPath; });
    }

    // Anything under Assets/ that no rule picked up is almost always a mistake.
    const auto assets = projectDir / "Assets";
    if (fs::is_directory(assets)) {
        for (const auto& entry : fs::recursive_directory_iterator(assets)) {
            if (entry.is_regular_file() && !isIgnored(entry.path()) &&
                !claimed.contains(fs::weakly_canonical(entry.path())))
                plan.warnings.push_back("not packed: " + entry.path().lexically_relative(projectDir).generic_string());
        }
    }
    return plan;
}

} // namespace iron::tools
