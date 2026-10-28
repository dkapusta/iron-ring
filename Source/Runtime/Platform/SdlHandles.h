#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace iron {

template <typename T, void (*Destroy)(T*)>
struct SdlDeleter {
    void operator()(T* p) const noexcept { if (p) Destroy(p); }
};

using WindowHandle   = std::unique_ptr<SDL_Window,   SdlDeleter<SDL_Window, SDL_DestroyWindow>>;
using RendererHandle = std::unique_ptr<SDL_Renderer, SdlDeleter<SDL_Renderer, SDL_DestroyRenderer>>;
using TextureHandle  = std::unique_ptr<SDL_Texture,  SdlDeleter<SDL_Texture, SDL_DestroyTexture>>;
using SurfaceHandle  = std::unique_ptr<SDL_Surface,  SdlDeleter<SDL_Surface, SDL_DestroySurface>>;

// Throws with SDL's error string when an SDL call reports failure.
inline void sdlCheck(bool ok, const char* what) {
    if (!ok) throw std::runtime_error(std::string(what) + ": " + SDL_GetError());
}

} // namespace iron
