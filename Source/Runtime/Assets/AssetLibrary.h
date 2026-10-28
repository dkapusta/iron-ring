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
// cached for the lifetime of the library.
class AssetLibrary {
public:
    AssetLibrary(const vfs::VirtualFileSystem& vfs, SDL_Renderer* renderer, SDL_ScaleMode filter);

    [[nodiscard]] nlohmann::json json(std::string_view path) const;
    [[nodiscard]] vfs::Bytes bytes(std::string_view path) const { return vfs_.read(path); }
    const Texture& texture(std::string_view path);

private:
    const vfs::VirtualFileSystem& vfs_;
    SDL_Renderer* renderer_;
    SDL_ScaleMode filter_;
    std::unordered_map<std::string, Texture> textures_;
};

// "linear" | "nearest" | "pixelart"; unknown values fall back to linear.
[[nodiscard]] SDL_ScaleMode parseScaleMode(std::string_view name);

// Resolves a path relative to another asset's folder:
//   siblingPath("Fighters/Dee/atlas.json", "atlas.png") == "Fighters/Dee/atlas.png"
[[nodiscard]] std::string siblingPath(std::string_view assetPath, std::string_view relative);

} // namespace iron
