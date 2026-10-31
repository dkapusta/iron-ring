#pragma once

#include "Platform/SdlHandles.h"
#include "Vfs/VirtualFileSystem.h"

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>

namespace iron {

struct Texture {
    TextureHandle handle;
    float width = 0;
    float height = 0;
};

// Turns virtual paths into engine objects. Textures are decoded once and
// cached until unloaded (streamed stage art) or the library goes.
class AssetLibrary {
public:
    AssetLibrary(const vfs::VirtualFileSystem& vfs, SDL_Renderer* renderer, SDL_ScaleMode filter);

    [[nodiscard]] nlohmann::json json(std::string_view path) const;
    [[nodiscard]] vfs::Bytes bytes(std::string_view path) const { return vfs_.read(path); }
    const Texture& texture(std::string_view path);
    // Frees a cached texture; references to it become invalid. False if it wasn't loaded.
    bool unload(std::string_view path);
    [[nodiscard]] bool loaded(std::string_view path) const { return textures_.count(std::string(path)) != 0; }
    [[nodiscard]] std::size_t textureCount() const { return textures_.size(); }

    // Renderer recreation (HDR on/off): textures belong to a renderer, so they
    // are released before it goes and decoded again for the new one. Texture
    // objects keep their addresses, so fonts and sprite sheets stay valid.
    void releaseTextures();
    void recreateTextures(SDL_Renderer* renderer);

private:
    TextureHandle decode(const std::string& path, float& width, float& height) const;

    const vfs::VirtualFileSystem& vfs_;
    SDL_Renderer* renderer_;
    SDL_ScaleMode filter_;
    std::unordered_map<std::string, Texture> textures_;
};

// "linear" | "nearest" | "pixelart"; unknown values fall back to linear.
[[nodiscard]] SDL_ScaleMode parseScaleMode(std::string_view name);

} // namespace iron
