#pragma once

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Content/Arcade.h"
#include "Content/Stage.h"
#include "Render/BitmapFont.h"
#include "Vfs/StreamingArchives.h"

#include <nlohmann/json.hpp>

#include <array>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace iron {

struct StageInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string music;      // theme id in music.json; empty = no music
    std::string musicTitle; // the theme's title, for the stage select
    const Texture* image = nullptr; // the flat stage, or a layered stage's preview
    content::StageDefinition definition; // world width and layers
    std::vector<const Texture*> layers;  // one per definition.layers entry; empty until GameData::useStage
};

// One slot on the character select: every fighter in Data/roster.json, in
// that order. Only those with a definition (GameSettings.json -> fighters)
// can be picked; the others show as locked.
struct RosterEntry {
    std::string id;
    std::string name;
    std::string role;
    std::string identity;
    std::string definition; // Fighters/<Name>/<id>.fighter.json; empty = not playable yet
    [[nodiscard]] bool playable() const { return !definition.empty(); }
};

// Content every scene shares, from ProjectSettings/GameSettings.json: fonts,
// sounds, announcer lines, themes, stages, the roster and the menu art. Most
// of it loads at start-up. A stage's layers and theme live in streamed
// archives and load when a fight uses the stage (useStage); the last
// kResidentStages stages stay loaded.
class GameData {
public:
    static constexpr std::size_t kResidentStages = 2;

    // streaming may be null (everything mounted, or loose files only).
    GameData(AssetLibrary& assets, AudioSystem& audio, vfs::StreamingArchives* streaming = nullptr);

    [[nodiscard]] const nlohmann::json& settings() const { return settings_; }
    [[nodiscard]] const BitmapFont& small() const { return *small_; }
    [[nodiscard]] const BitmapFont& large() const { return *large_; }
    [[nodiscard]] const std::vector<StageInfo>& stages() const { return stages_; }
    // Index of a stage by id; the start stage when the id is empty or unknown.
    [[nodiscard]] std::size_t stageIndex(std::string_view id) const;
    // Loads a stage's layers and theme (mounting its archives) and marks it
    // most recently used; evicts the least recently used beyond kResidentStages.
    const StageInfo& useStage(std::size_t index);
    // Loaded stages, most recently used first.
    [[nodiscard]] const std::deque<std::size_t>& residentStages() const { return resident_; }
    [[nodiscard]] const std::vector<RosterEntry>& roster() const { return roster_; } // selectable fighters
    [[nodiscard]] const RosterEntry* fighter(std::string_view id) const;
    // The first two playable fighters (the default pairing).
    [[nodiscard]] std::array<std::string, 2> defaultPair() const;
    [[nodiscard]] const content::ArcadeLadder* arcade() const { return arcade_ ? &*arcade_ : nullptr; }
    [[nodiscard]] std::vector<std::string> playableIds() const;
    [[nodiscard]] const Texture& logo() const { return *logo_; }
    [[nodiscard]] const Texture& backdrop() const { return *backdrop_; }

    // Plays a theme by id (no-op if it is already playing); empty or unknown stops the music.
    void playTheme(AudioSystem& audio, std::string_view id) const;
    [[nodiscard]] const std::string& menuTheme() const { return menuTheme_; }
    // net::gameDataHash() of every fighter's gameplay data and every stage's
    // width: what IronRingServer compares before letting this game in.
    [[nodiscard]] std::uint64_t simulationHash() const { return simulationHash_; }
    // GameSettings.json -> defaultServer: what the ONLINE server field starts with (usually empty).
    [[nodiscard]] std::string defaultServer() const { return settings_.value("defaultServer", std::string{}); }

private:
    void loadStages(AssetLibrary& assets);
    void loadRoster(AssetLibrary& assets);
    void loadMusic(AssetLibrary& assets);
    void evictStage(std::size_t index);
    [[nodiscard]] std::vector<std::string> stagePaths(const StageInfo& stage) const;

    nlohmann::json settings_;
    std::unique_ptr<BitmapFont> small_, large_;
    std::vector<StageInfo> stages_;
    std::size_t startStage_ = 0;
    std::vector<RosterEntry> roster_;  // the character select's slots
    std::vector<RosterEntry> bosses_;  // "boss": true in roster.json: arcade only
    AssetLibrary& assets_;
    vfs::StreamingArchives* streaming_;
    std::deque<std::size_t> resident_; // stage indices, most recently used first
    std::unordered_map<std::string, AudioSystem::Music> themes_; // streamed themes: empty bytes until used
    std::unordered_map<std::string, std::string> themeFiles_;   // theme id -> virtual path
    std::unordered_map<std::string, std::string> themeTitles_;
    std::string menuTheme_;
    std::uint64_t simulationHash_ = 0;
    std::optional<content::ArcadeLadder> arcade_;
    const Texture* logo_ = nullptr;
    const Texture* backdrop_ = nullptr;
};

} // namespace iron
