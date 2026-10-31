#include "Game/GameData.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <stdexcept>

namespace iron {

GameData::GameData(AssetLibrary& assets, AudioSystem& audio, vfs::StreamingArchives* streaming)
    : settings_(assets.json("ProjectSettings/GameSettings.json")), assets_(assets), streaming_(streaming) {
    small_ = std::make_unique<BitmapFont>(assets, settings_.at("fonts").at("small").get<std::string>());
    large_ = std::make_unique<BitmapFont>(assets, settings_.at("fonts").at("large").get<std::string>());
    for (const auto& [id, path] : settings_.at("sounds").items()) audio.load(id, assets.bytes(path.get<std::string>()));
    if (settings_.contains("voice"))
        for (const auto& [id, path] : settings_.at("voice").items())
            audio.loadVoice(id, assets.bytes(path.get<std::string>()));
    if (settings_.contains("music")) loadMusic(assets);
    loadStages(assets);
    loadRoster(assets);
    const auto& ui = settings_.at("ui");
    logo_ = &assets.texture(ui.at("logo").get<std::string>());
    backdrop_ = &assets.texture(ui.at("backdrop").get<std::string>());
    menuTheme_ = ui.value("music", std::string{});
    if (settings_.contains("arcade")) {
        const auto path = settings_.at("arcade").get<std::string>();
        arcade_ = content::parseArcade(assets.json(path), path);
    }
}

std::vector<std::string> GameData::playableIds() const {
    std::vector<std::string> ids;
    for (const auto& e : roster_)
        if (e.playable()) ids.push_back(e.id);
    return ids;
}

void GameData::loadMusic(AssetLibrary& assets) {
    const auto musicPath = settings_.at("music").get<std::string>();
    const auto catalog = assets.json(musicPath); // keep alive: iterated below
    const auto folder = musicPath.substr(0, musicPath.find_last_of('/') + 1);
    for (const auto& [id, theme] : catalog.at("themes").items()) {
        AudioSystem::Music music;
        music.id = id;
        const auto file = folder + theme.at("file").get<std::string>();
        if (!streaming_ || !streaming_->streams(file)) music.encoded = assets.bytes(file); // else: useStage
        themeFiles_.emplace(id, file);
        music.loopStartFrame = theme.value("loopStartFrame", std::int64_t{0});
        music.gain = theme.value("gain", 1.0f);
        themes_.emplace(id, std::move(music));
        themeTitles_.emplace(id, theme.value("title", id));
    }
}

void GameData::loadStages(AssetLibrary& assets) {
    const auto path = settings_.at("stages").get<std::string>();
    const auto catalog = assets.json(path); // keep alive: range-for over a temporary's member dangles
    const auto start = settings_.value("startStage", std::string{});
    for (const auto& s : catalog.at("stages")) {
        StageInfo stage;
        stage.definition = content::parseStage(s, path);
        stage.id = stage.definition.id;
        stage.name = stage.definition.name;
        stage.description = stage.definition.description;
        stage.music = stage.definition.music;
        if (const auto it = themeTitles_.find(stage.music); it != themeTitles_.end()) stage.musicTitle = it->second;
        stage.image = &assets.texture(stage.definition.image); // previews stay resident
        if (stage.id == start) startStage_ = stages_.size();
        stages_.push_back(std::move(stage));
    }
    if (stages_.empty()) throw std::runtime_error(path + ": no stages");
}

void GameData::loadRoster(AssetLibrary& assets) {
    std::unordered_map<std::string, std::string> definitions; // fighter id -> definition path
    std::vector<std::string> order;
    for (const auto& item : settings_.at("fighters")) {
        const auto path = item.get<std::string>();
        const auto id = assets.json(path).at("id").get<std::string>();
        definitions.emplace(id, path);
        order.push_back(id);
    }
    if (definitions.size() < 2) throw std::runtime_error("GameSettings.json: two fighters are required");

    if (settings_.contains("roster")) {
        const auto roster = assets.json(settings_.at("roster").get<std::string>());
        for (const auto& f : roster.at("fighters")) {
            RosterEntry e;
            e.id = f.at("id").get<std::string>();
            e.name = f.value("name", e.id);
            e.role = f.value("role", std::string{});
            e.identity = f.value("identity", std::string{});
            if (const auto it = definitions.find(e.id); it != definitions.end()) {
                e.definition = it->second;
                definitions.erase(it);
            }
            // A boss (the Broker) fights in the arcade but has no slot on the select.
            (f.value("boss", false) ? bosses_ : roster_).push_back(std::move(e));
        }
    }
    // Defined fighters missing from the roster file still get a slot.
    for (const auto& id : order)
        if (const auto it = definitions.find(id); it != definitions.end())
            roster_.push_back({id, id, {}, {}, it->second});
}

std::size_t GameData::stageIndex(std::string_view id) const {
    for (std::size_t i = 0; i < stages_.size(); ++i)
        if (stages_[i].id == id) return i;
    if (!id.empty())
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown stage '%.*s'; using %s", static_cast<int>(id.size()),
                    id.data(), stages_[startStage_].name.c_str());
    return startStage_;
}

const RosterEntry* GameData::fighter(std::string_view id) const {
    for (const auto& e : roster_)
        if (e.id == id) return &e;
    for (const auto& e : bosses_)
        if (e.id == id) return &e;
    return nullptr;
}

std::array<std::string, 2> GameData::defaultPair() const {
    std::array<std::string, 2> pair;
    std::size_t n = 0;
    for (const auto& e : roster_)
        if (e.playable() && n < 2) pair[n++] = e.id;
    return pair;
}

std::vector<std::string> GameData::stagePaths(const StageInfo& stage) const {
    std::vector<std::string> paths;
    for (const auto& layer : stage.definition.layers) paths.push_back(layer.image);
    if (const auto it = themeFiles_.find(stage.music); it != themeFiles_.end()) paths.push_back(it->second);
    return paths;
}

const StageInfo& GameData::useStage(std::size_t index) {
    auto& stage = stages_.at(index);
    if (const auto it = std::find(resident_.begin(), resident_.end(), index); it != resident_.end()) {
        resident_.erase(it);
        resident_.push_front(index);
        return stage;
    }
    const auto started = SDL_GetTicksNS();
    if (streaming_) streaming_->acquire(stage.id, stagePaths(stage));
    for (const auto& layer : stage.definition.layers) stage.layers.push_back(&assets_.texture(layer.image));
    if (const auto it = themes_.find(stage.music); it != themes_.end() && it->second.encoded.empty())
        it->second.encoded = assets_.bytes(themeFiles_.at(stage.music));
    resident_.push_front(index);
    while (resident_.size() > kResidentStages) {
        const auto oldest = resident_.back();
        resident_.pop_back();
        evictStage(oldest);
    }
    SDL_Log("Stage %s loaded in %.1f ms (%d resident)", stage.id.c_str(),
            static_cast<double>(SDL_GetTicksNS() - started) / 1e6, static_cast<int>(resident_.size()));
    return stage;
}

void GameData::evictStage(std::size_t index) {
    auto& stage = stages_.at(index);
    stage.layers.clear();
    for (const auto& layer : stage.definition.layers) {
        // A layer image shared with a resident stage stays.
        bool shared = false;
        for (const auto r : resident_)
            for (const auto& other : stages_[r].definition.layers) shared = shared || other.image == layer.image;
        if (!shared) assets_.unload(layer.image);
    }
    if (const auto it = themes_.find(stage.music); it != themes_.end() && streaming_ &&
                                                     streaming_->streams(themeFiles_.at(stage.music))) {
        bool shared = false;
        for (const auto r : resident_) shared = shared || stages_[r].music == stage.music;
        if (!shared) it->second.encoded = {};
    }
    if (streaming_) streaming_->release(stage.id);
}

void GameData::playTheme(AudioSystem& audio, std::string_view id) const {
    const auto it = themes_.find(std::string(id));
    if (it == themes_.end() || it->second.encoded.empty()) { // unknown, or a stage theme not loaded
        audio.stopMusic(300);
        return;
    }
    audio.playMusic(it->second); // a copy: the encoded bytes stay here
}

} // namespace iron
