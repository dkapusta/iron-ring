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

TextureHandle AssetLibrary::decode(const std::string& path, float& width, float& height) const {
    const auto data = vfs_.read(path);
    SurfaceHandle surface(SDL_LoadPNG_IO(SDL_IOFromConstMem(data.data(), data.size()), true));
    if (!surface) throw std::runtime_error("Cannot decode '" + path + "': " + SDL_GetError());
    width = static_cast<float>(surface->w);
    height = static_cast<float>(surface->h);
    TextureHandle handle(SDL_CreateTextureFromSurface(renderer_, surface.get()));
    sdlCheck(handle != nullptr, "SDL_CreateTextureFromSurface");
    SDL_SetTextureScaleMode(handle.get(), filter_);
    SDL_SetTextureBlendMode(handle.get(), SDL_BLENDMODE_BLEND);
    return handle;
}

const Texture& AssetLibrary::texture(std::string_view path) {
    const std::string key(path);
    if (const auto it = textures_.find(key); it != textures_.end()) return it->second;
    Texture t;
    t.handle = decode(key, t.width, t.height);
    return textures_.emplace(key, std::move(t)).first->second;
}

bool AssetLibrary::unload(std::string_view path) { return textures_.erase(std::string(path)) != 0; }

void AssetLibrary::releaseTextures() {
    for (auto& [_, t] : textures_) t.handle.reset();
}

void AssetLibrary::recreateTextures(SDL_Renderer* renderer) {
    renderer_ = renderer;
    for (auto& [path, t] : textures_) t.handle = decode(path, t.width, t.height);
}

SDL_ScaleMode parseScaleMode(std::string_view name) {
    if (name == "nearest") return SDL_SCALEMODE_NEAREST;
    if (name == "pixelart") return SDL_SCALEMODE_PIXELART;
    return SDL_SCALEMODE_LINEAR;
}

} // namespace iron
