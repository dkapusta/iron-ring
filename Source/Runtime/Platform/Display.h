#pragma once

#include "Core/PlayerSettings.h"
#include "Platform/SdlHandles.h"

#include <string>

namespace iron {

struct HdrState {
    bool linearOutput = false; // renderer created with SDL_COLORSPACE_SRGB_LINEAR (scRGB)
    bool enabled = false;      // the display is currently presenting HDR
    float sdrWhite = 1.0f;     // SDR white in linear units (applied by SDL automatically)
    float headroom = 1.0f;     // brightest displayable value, in multiples of SDR white
};

// Owns the window and renderer. Fullscreen uses the desktop mode of the
// primary display (native resolution, borderless). HDR output is enabled when
// the display reports HDR and the renderer supports a linear colorspace
// (docs/adr/0005-display-hdr-and-presentation.md).
class Display {
public:
    Display(const PlayerSettings& settings, const char* title, int logicalWidth, int logicalHeight);

    [[nodiscard]] SDL_Window* window() const { return window_.get(); }
    [[nodiscard]] SDL_Renderer* renderer() const { return renderer_.get(); }
    [[nodiscard]] const HdrState& hdr() const { return hdr_; }

    // Multiplier for highlight draws (impact flashes, sparks): 1.0 on SDR,
    // up to min(headroom, hdr-max-boost) on HDR displays.
    [[nodiscard]] float highlightBoost() const;

    void onHdrStateChanged();
    void toggleFullscreen();
    [[nodiscard]] std::string describe() const;

private:
    void createRenderer(const PlayerSettings& settings);
    void readHdrState();

    WindowHandle window_;
    RendererHandle renderer_;
    HdrState hdr_;
    float maxBoost_ = 1.0f;
};

} // namespace iron
