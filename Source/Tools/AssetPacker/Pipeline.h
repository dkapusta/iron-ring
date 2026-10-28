#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace iron::tools {

// One output archive, as declared in ProjectSettings/AssetPipeline.json.
struct ArchiveRule {
    std::string output;                      // file name inside <name>_Data
    std::filesystem::path root;              // virtual paths are relative to this
    std::vector<std::filesystem::path> include; // folders (relative to root) to scan
    std::vector<std::string> extensions;        // if non-empty: only these
    std::vector<std::string> excludeExtensions;

    [[nodiscard]] bool accepts(const std::filesystem::path& file) const;
};

struct PipelineConfig {
    std::vector<ArchiveRule> archives;
};

[[nodiscard]] PipelineConfig loadPipeline(const std::filesystem::path& jsonFile);

// A source file assigned to an archive.
struct PlannedFile {
    std::filesystem::path source;
    std::string virtualPath;
};

struct PackPlan {
    std::vector<std::vector<PlannedFile>> archives; // parallel to PipelineConfig::archives
    std::vector<std::string> warnings;
};

// Resolves the rules against the project tree. Throws std::runtime_error on
// errors (missing folders, a virtual path claimed by two archives).
[[nodiscard]] PackPlan planPack(const PipelineConfig& config, const std::filesystem::path& projectDir);

} // namespace iron::tools
