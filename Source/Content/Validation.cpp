#include "Content/Validation.h"

#include "Content/Paths.h"
#include "Content/Stage.h"

#include <algorithm>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace iron::content {
namespace {

using nlohmann::json;

std::optional<json> readJson(const FileReader& read, std::string_view path, ValidationReport& report) {
    const auto bytes = read(path);
    if (!bytes) {
        report.errors.push_back(std::string(path) + ": file not found");
        return std::nullopt;
    }
    auto parsed = json::parse(reinterpret_cast<const char*>(bytes->data()),
                              reinterpret_cast<const char*>(bytes->data() + bytes->size()), nullptr, false);
    if (parsed.is_discarded()) {
        report.errors.push_back(std::string(path) + ": malformed JSON");
        return std::nullopt;
    }
    return parsed;
}

std::optional<std::pair<int, int>> readPng(const FileReader& read, std::string_view path, ValidationReport& report) {
    const auto bytes = read(path);
    if (!bytes) {
        report.errors.push_back(std::string(path) + ": file not found");
        return std::nullopt;
    }
    const auto size = pngSize(*bytes);
    if (!size) report.errors.push_back(std::string(path) + ": not a PNG file");
    return size;
}

void requireFile(const FileReader& read, std::string_view path, ValidationReport& report) {
    if (!read(path)) report.errors.push_back(std::string(path) + ": file not found");
}

void checkAtlasBounds(const AtlasDefinition& atlas, std::pair<int, int> size, std::string_view owner,
                      std::vector<std::string>& errors) {
    for (const auto& [name, f] : atlas.frames) {
        if (f.w <= 0 || f.h <= 0 || f.x < 0 || f.y < 0 || f.x + f.w > size.first || f.y + f.h > size.second)
            errors.push_back(std::string(owner) + ": frame '" + name + "' lies outside " + atlas.image);
    }
}

} // namespace

std::vector<std::string> validateFighter(const FighterDefinition& d, const AtlasDefinition& atlas,
                                         std::pair<int, int> imageSize) {
    std::vector<std::string> errors;
    const auto err = [&](const std::string& what) { errors.push_back(d.id + ": " + what); };

    checkAtlasBounds(atlas, imageSize, d.id, errors);
    for (const auto name : kRequiredAnimations)
        if (!d.hasAnimation(name)) err("missing animation '" + std::string(name) + "'");
    for (const auto frame : kSharedFrames)
        if (!atlas.find(frame)) err("atlas lacks shared frame '" + std::string(frame) + "'");

    for (const auto& [name, animation] : d.animations) {
        for (const auto& f : animation.frames) {
            if (!atlas.find(f.frame)) err("animation '" + name + "' uses unknown frame '" + f.frame + "'");
            if (!f.partnerFrame.empty() && !atlas.find(f.partnerFrame))
                err("animation '" + name + "' names partner frame '" + f.partnerFrame + "' that no atlas has");
            if (!f.partnerFrame.empty() && atlas.find(f.frame) && !atlas.find(f.frame)->partner)
                err("frame '" + f.frame + "' holds an opponent but has no partner offset in the atlas");
        }
    }

    // Animation timing must match the simulation exactly, or art and hitboxes drift apart.
    for (std::size_t slot = 0; slot < kMoveKeys.size(); ++slot) {
        const auto& m = d.spec.moves[slot];
        const auto& info = d.moves[slot];
        if (!m.enabled) continue;
        if (m.hitbox.empty()) err("moves." + info.key + ": empty hitbox");
        if (!d.hasAnimation(info.animation)) {
            err("moves." + info.key + ": unknown animation '" + info.animation + "'");
            continue;
        }
        const int ticks = d.animation(info.animation).totalTicks();
        if (ticks != m.duration())
            err("moves." + info.key + ": animation '" + info.animation + "' lasts " + std::to_string(ticks) +
                " ticks, the move " + std::to_string(m.duration()) + " (startup + active + recovery)");
        if (m.kind == sim::MoveKind::Throw) {
            if (!d.hasAnimation(info.holdAnimation)) {
                err("moves." + info.key + ": unknown holdAnimation '" + info.holdAnimation + "'");
            } else if (const int hold = d.animation(info.holdAnimation).totalTicks(); hold != m.throwDuration) {
                err("moves." + info.key + ": holdAnimation lasts " + std::to_string(hold) + " ticks, throwDuration is " +
                    std::to_string(m.throwDuration));
            }
        }
    }
    if (d.hasAnimation("knockdown") && d.animation("knockdown").totalTicks() != d.spec.knockdownTicks)
        err("animation 'knockdown' must last body.knockdownTicks (" + std::to_string(d.spec.knockdownTicks) + ")");
    if (d.spec.standHurt.empty() || d.spec.crouchHurt.empty()) err("empty hurt box");
    return errors;
}

ValidationReport validateProject(const FileReader& read) {
    ValidationReport report;
    const auto settings = readJson(read, "ProjectSettings/GameSettings.json", report);
    if (!settings) return report;
    (void)readJson(read, "ProjectSettings/InputManager.json", report);

    try {
        // Fighters.
        std::vector<std::pair<FighterDefinition, AtlasDefinition>> fighters;
        for (const auto& entry : settings->at("fighters")) {
            const auto path = entry.get<std::string>();
            const auto json = readJson(read, path, report);
            if (!json) continue;
            try {
                auto fighter = parseFighter(*json, path);
                const auto atlasJson = readJson(read, fighter.atlas, report);
                if (!atlasJson) continue;
                auto atlas = parseAtlas(*atlasJson, fighter.atlas);
                (void)readPng(read, fighter.portrait, report);
                if (const auto size = readPng(read, atlas.image, report)) {
                    for (auto& e : validateFighter(fighter, atlas, *size)) report.errors.push_back(path + ": " + e);
                }
                fighters.emplace_back(std::move(fighter), std::move(atlas));
            } catch (const ContentError& e) {
                report.errors.push_back(e.what());
            }
        }
        if (fighters.size() < 2) report.errors.push_back("GameSettings.json: at least two fighters are required");

        // Stages.
        const auto stagesPath = settings->at("stages").get<std::string>();
        std::optional<std::vector<std::pair<std::string, std::string>>> stageMusic;
        if (const auto stages = readJson(read, stagesPath, report)) {
            stageMusic.emplace();
            for (const auto& entry : stages->at("stages")) {
                const auto stage = parseStage(entry, stagesPath);
                if (!stage.music.empty()) stageMusic->emplace_back(stage.id, stage.music);
                if (const auto size = readPng(read, stage.image, report); size && *size != std::pair{640, 360})
                    report.warnings.push_back(stage.image + ": stages (and layered stages' previews) are expected to be 640x360");
                if (stage.width > 640 && !stage.layered())
                    report.errors.push_back(stagesPath + ": " + stage.id + ": a stage wider than 640 needs layers");
                for (const auto& layer : stage.layers) {
                    const auto size = readPng(read, layer.image, report);
                    if (!size) continue;
                    const auto [w, h] = *size;
                    if (h % layer.frames != 0)
                        report.errors.push_back(layer.image + ": height " + std::to_string(h) + " is not " +
                                                std::to_string(layer.frames) + " equal frames");
                    // A drifting layer repeats; any other must cover the whole camera travel.
                    if (layer.drift == 0.0f && w < stage.requiredWidth(layer))
                        report.errors.push_back(layer.image + ": " + std::to_string(w) + " px wide, needs " +
                                                std::to_string(stage.requiredWidth(layer)) + " for parallax " +
                                                std::to_string(layer.parallax).substr(0, 4) + " on a " +
                                                std::to_string(stage.width) + " px stage");
                }
            }
        }

        // Front end: the roster file, the title logo, the menu backdrop and theme.
        if (settings->contains("roster")) {
            const auto rosterPath = settings->at("roster").get<std::string>();
            if (const auto roster = readJson(read, rosterPath, report)) {
                std::vector<std::string> listed;
                for (const auto& f : roster->at("fighters")) listed.push_back(f.at("id").get<std::string>());
                for (const auto& [fighter, _] : fighters)
                    if (std::find(listed.begin(), listed.end(), fighter.id) == listed.end())
                        report.warnings.push_back(rosterPath + ": '" + fighter.id + "' is not listed (it goes last)");
            }
        }
        std::string menuTheme;
        if (!settings->contains("ui")) {
            report.errors.push_back("GameSettings.json: ui (logo, backdrop, music) is required");
        } else {
            const auto& ui = settings->at("ui");
            for (const auto key : {"logo", "backdrop"}) {
                if (!ui.contains(key)) {
                    report.errors.push_back(std::string("GameSettings.json: ui: missing '") + key + "'");
                    continue;
                }
                const auto size = readPng(read, ui.at(key).get<std::string>(), report);
                if (size && std::string_view(key) == "backdrop" && *size != std::pair{640, 360})
                    report.warnings.push_back("GameSettings.json: ui: the backdrop is expected to be 640x360");
            }
            menuTheme = ui.value("music", std::string{});
        }

        // Effects.
        const auto effectsPath = settings->at("effects").get<std::string>();
        if (const auto json = readJson(read, effectsPath, report)) {
            const auto atlas = parseAtlas(*json, effectsPath);
            if (const auto size = readPng(read, atlas.image, report)) {
                std::vector<std::string> errors;
                checkAtlasBounds(atlas, *size, effectsPath, errors);
                report.errors.insert(report.errors.end(), errors.begin(), errors.end());
            }
            for (const auto effect : kEffects)
                if (!atlas.find(std::string(effect) + "_0"))
                    report.errors.push_back(effectsPath + ": missing effect '" + std::string(effect) + "'");
        }

        // Fonts and sounds.
        for (const auto& [name, path] : settings->at("fonts").items()) {
            const auto fontPath = path.get<std::string>();
            if (const auto json = readJson(read, fontPath, report))
                (void)readPng(read, siblingPath(fontPath, json->at("image").get<std::string>()), report);
        }
        for (const auto& [id, path] : settings->at("sounds").items()) requireFile(read, path.get<std::string>(), report);
        if (settings->contains("voice")) {
            const auto& voice = settings->at("voice");
            for (const auto& [id, path] : voice.items()) requireFile(read, path.get<std::string>(), report);
            for (const auto id : kVoiceLines)
                if (!voice.contains(id))
                    report.errors.push_back("GameSettings.json: voice: missing '" + std::string(id) + "'");
            for (const auto& [fighter, _] : fighters)
                if (!voice.contains(fighter.id + "_wins"))
                    report.warnings.push_back("GameSettings.json: voice: no '" + fighter.id + "_wins' line");
        }
        for (const auto id : kSounds)
            if (!settings->at("sounds").contains(id))
                report.errors.push_back("GameSettings.json: sounds: missing '" + std::string(id) + "'");
        if (settings->contains("music")) {
            const auto musicPath = settings->at("music").get<std::string>();
            if (const auto music = readJson(read, musicPath, report)) {
                for (const auto& [id, theme] : music->at("themes").items()) {
                    requireFile(read, siblingPath(musicPath, theme.at("file").get<std::string>()), report);
                    if (theme.value("loopStartFrame", std::int64_t{0}) >= theme.value("lengthFrames", std::int64_t{0}))
                        report.errors.push_back(musicPath + ": " + id + ": loopStartFrame must be inside the track");
                }
                if (!menuTheme.empty() && !music->at("themes").contains(menuTheme))
                    report.errors.push_back("GameSettings.json: ui: unknown music '" + menuTheme + "'");
                if (stageMusic) {
                    for (const auto& [stage, theme] : *stageMusic)
                        if (!music->at("themes").contains(theme))
                            report.errors.push_back(stagesPath + ": " + stage + ": unknown music '" + theme + "'");
                }
            }
        }
    } catch (const nlohmann::json::exception& e) {
        report.errors.push_back(std::string("GameSettings.json: ") + e.what());
    } catch (const ContentError& e) {
        report.errors.push_back(e.what());
    }
    return report;
}

} // namespace iron::content
