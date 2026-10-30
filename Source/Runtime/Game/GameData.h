#pragma once

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Content/Arcade.h"
#include "Content/Stage.h"
#include "Render/BitmapFont.h"

#include <nlohmann/json.hpp>

#include <array>
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
    std::vector<const Texture*> layers;  // one per definition.layers entry
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

// Content every scene shares, loaded once at start-up from
// ProjectSettings/GameSettings.json: fonts, sounds, announcer lines, themes,
// stages, the roster and the menu art.
class GameData {
public:
    GameData(AssetLibrary& assets, AudioSystem& audio);

    [[nodiscard]] const nlohmann::json& settings() const { return settings_; }
    [[nodiscard]] const BitmapFont& small() const { return *small_; }
    [[nodiscard]] const BitmapFont& large() const { return *large_; }
    [[nodiscard]] const std::vector<StageInfo>& stages() const { return stages_; }
    // Index of a stage by id; the start stage when the id is empty or unknown.
    [[nodiscard]] std::size_t stageIndex(std::string_view id) const;
    [[nodiscard]] const std::vector<RosterEntry>& roster() const { return roster_; }
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

private:
    void loadStages(AssetLibrary& assets);
    void loadRoster(AssetLibrary& assets);
    void loadMusic(AssetLibrary& assets);

    nlohmann::json settings_;
    std::unique_ptr<BitmapFont> small_, large_;
    std::vector<StageInfo> stages_;
    std::size_t startStage_ = 0;
    std::vector<RosterEntry> roster_;
    std::unordered_map<std::string, AudioSystem::Music> themes_;
    std::unordered_map<std::string, std::string> themeTitles_;
    std::string menuTheme_;
    std::optional<content::ArcadeLadder> arcade_;
    const Texture* logo_ = nullptr;
    const Texture* backdrop_ = nullptr;
};

} // namespace iron
