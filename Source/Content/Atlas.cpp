#include "Content/Atlas.h"

#include "Content/Fighter.h"
#include "Content/Paths.h"

namespace iron::content {

const AtlasFrame* AtlasDefinition::find(std::string_view name) const {
    const auto it = frames.find(name);
    return it == frames.end() ? nullptr : &it->second;
}

AtlasDefinition parseAtlas(const nlohmann::json& json, std::string_view path) {
    try {
        AtlasDefinition atlas;
        atlas.image = siblingPath(path, json.at("image").get<std::string>());
        for (const auto& f : json.at("frames")) {
            const auto& r = f.at("source_rect");
            const auto& p = f.at("pivot_pixels");
            AtlasFrame frame{r[0].get<int>(), r[1].get<int>(), r[2].get<int>(), r[3].get<int>(),
                             static_cast<int>(p[0].get<double>()), static_cast<int>(p[1].get<double>()), {}};
            if (const auto it = f.find("partner"); it != f.end())
                frame.partner = std::pair{(*it)[0].get<int>(), (*it)[1].get<int>()};
            const auto name = f.at("name").get<std::string>();
            if (!atlas.frames.emplace(name, frame).second)
                throw ContentError(std::string(path) + ": duplicate frame '" + name + "'");
        }
        return atlas;
    } catch (const nlohmann::json::exception& e) {
        throw ContentError(std::string(path) + ": " + e.what());
    }
}

} // namespace iron::content
