#include "Assets/AssetLibrary.h"

#include <stdexcept>

namespace iron {

AssetLibrary::AssetLibrary(const vfs::VirtualFileSystem& vfs, SDL_Renderer* renderer, SDL_ScaleMode filter)
    : vfs_(vfs), renderer_(renderer), filter_(filter) {}

nlohmann::json AssetLibrary::json(std::string_view path) const {
    const auto text = vfs_.readText(path);
    auto parsed = nlohmann::json::parse(text, nullptr, false);
    if (parsed.is_discarded()) throw std::runtime_error("Malformed JSON: '" + std::string(path) + "'");
    return parsed;
}

const Texture& AssetLibrary::texture(std::string_view path) {
    const std::string key(path);
    if (const auto it = textures_.find(key); it != textures_.end()) return it->second;

    const auto data = vfs_.read(path);
    SurfaceHandle surface(SDL_LoadPNG_IO(SDL_IOFromConstMem(data.data(), data.size()), true));
    if (!surface) throw std::runtime_error("Cannot decode '" + key + "': " + SDL_GetError());

    Texture t;
    t.width = static_cast<float>(surface->w);
    t.height = static_cast<float>(surface->h);
    t.handle.reset(SDL_CreateTextureFromSurface(renderer_, surface.get()));
    sdlCheck(t.handle != nullptr, "SDL_CreateTextureFromSurface");
    SDL_SetTextureScaleMode(t.handle.get(), filter_);
    SDL_SetTextureBlendMode(t.handle.get(), SDL_BLENDMODE_BLEND);
    return textures_.emplace(key, std::move(t)).first->second;
}

SDL_ScaleMode parseScaleMode(std::string_view name) {
    if (name == "nearest") return SDL_SCALEMODE_NEAREST;
    if (name == "pixelart") return SDL_SCALEMODE_PIXELART;
    return SDL_SCALEMODE_LINEAR;
}

std::string siblingPath(std::string_view assetPath, std::string_view relative) {
    const auto slash = assetPath.rfind('/');
    if (slash == std::string_view::npos) return std::string(relative);
    return std::string(assetPath.substr(0, slash + 1)) + std::string(relative);
}

} // namespace iron
