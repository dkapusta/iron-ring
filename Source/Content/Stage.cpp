#include "Content/Stage.h"

#include "Content/Fighter.h" // ContentError

#include <cmath>

namespace iron::content {

int StageDefinition::requiredWidth(const StageLayer& layer) const {
    const float scroll = static_cast<float>(width - 640) * layer.parallax;
    return 640 + static_cast<int>(std::ceil(scroll)) - layer.x;
}

StageDefinition parseStage(const nlohmann::json& j, std::string_view path) {
    const auto where = [&](const std::string& what) { return std::string(path) + ": " + what; };
    StageDefinition s;
    s.id = j.at("id").get<std::string>();
    s.name = j.value("name", s.id);
    s.description = j.value("description", std::string{});
    s.image = j.at("image").get<std::string>();
    s.music = j.value("music", std::string{});
    s.width = j.value("width", 640);
    if (s.width < 640 || s.width > 2048) throw ContentError(where(s.id + ": width must be 640..2048"));
    if (j.contains("layers")) {
        for (const auto& l : j.at("layers")) {
            StageLayer layer;
            layer.image = l.at("image").get<std::string>();
            layer.parallax = l.value("parallax", 1.0f);
            layer.x = l.value("x", 0);
            layer.y = l.value("y", 0);
            layer.drift = l.value("drift", 0.0f);
            layer.frames = l.value("frames", 1);
            layer.frameTicks = l.value("frameTicks", 10);
            layer.crowd = l.value("crowd", false);
            layer.front = l.value("front", false);
            if (layer.parallax < 0.0f || layer.parallax > 2.0f)
                throw ContentError(where(s.id + ": " + layer.image + ": parallax must be 0..2"));
            if (layer.frames < 1 || layer.frameTicks < 1)
                throw ContentError(where(s.id + ": " + layer.image + ": frames and frameTicks must be >= 1"));
            if (layer.crowd && layer.frames % 2 != 0)
                throw ContentError(where(s.id + ": " + layer.image + ": a crowd needs an even frame count (rest, cheer)"));
            s.layers.push_back(std::move(layer));
        }
    }
    return s;
}

} // namespace iron::content
