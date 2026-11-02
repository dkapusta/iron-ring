#include "Platform/Display.h"

#include <algorithm>
#include <cstdio>

namespace iron {
namespace {

bool displayReportsHdr(SDL_Window* window) {
    const SDL_PropertiesID props = SDL_GetWindowProperties(window);
    return SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);
}

RendererHandle tryCreateRenderer(SDL_Window* window, const PlayerSettings& settings, SDL_Colorspace colorspace) {
    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
    SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, settings.vsync ? 1 : 0);
    SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER, colorspace);
    // Batch mode draws offscreen, often on a machine without a GPU: SDL's software
    // renderer there, since a software OpenGL (Mesa llvmpipe) costs over 1 GB of memory.
    const std::string driver = !settings.renderDriver.empty() ? settings.renderDriver
                             : settings.batchMode             ? std::string("software")
                                                              : std::string();
    if (!driver.empty()) SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, driver.c_str());
    RendererHandle renderer(SDL_CreateRendererWithProperties(props));
    SDL_DestroyProperties(props);
    return renderer;
}

} // namespace

Display::Display(const PlayerSettings& settings, const char* title, int logicalWidth, int logicalHeight)
    : maxBoost_(settings.hdrMaxBoost), logicalWidth_(logicalWidth), logicalHeight_(logicalHeight) {
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    int width = settings.windowWidth;
    int height = settings.windowHeight;

    if (settings.windowMode == WindowMode::Fullscreen) {
        // Borderless fullscreen at the desktop mode: native resolution, no mode switch.
        const SDL_DisplayMode* desktop = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
        if (desktop) {
            width = desktop->w;
            height = desktop->h;
        }
        flags |= SDL_WINDOW_FULLSCREEN;
    } else {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window_.reset(SDL_CreateWindow(title, width, height, flags));
    sdlCheck(window_ != nullptr, "SDL_CreateWindow");
    createRenderer(settings);

    setPresentation(settings.presentation);
    SDL_HideCursor();
}

void Display::createRenderer(const PlayerSettings& settings) {
    const bool wantHdr = settings.hdr == HdrMode::On || (settings.hdr == HdrMode::Auto && displayReportsHdr(window_.get()));
    if (wantHdr) {
        // SDL documents scRGB output for the direct3d11, direct3d12 and metal renderers.
        renderer_ = tryCreateRenderer(window_.get(), settings, SDL_COLORSPACE_SRGB_LINEAR);
        if (renderer_) hdr_.linearOutput = true;
        else SDL_Log("HDR: linear output unavailable (%s); falling back to SDR", SDL_GetError());
    }
    if (!renderer_) renderer_ = tryCreateRenderer(window_.get(), settings, SDL_COLORSPACE_SRGB);
    sdlCheck(renderer_ != nullptr, "SDL_CreateRenderer");
    readHdrState();
}

void Display::readHdrState() {
    const SDL_PropertiesID props = SDL_GetRendererProperties(renderer_.get());
    hdr_.enabled = SDL_GetBooleanProperty(props, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN, false);
    hdr_.sdrWhite = SDL_GetFloatProperty(props, SDL_PROP_RENDERER_SDR_WHITE_POINT_FLOAT, 1.0f);
    hdr_.headroom = SDL_GetFloatProperty(props, SDL_PROP_RENDERER_HDR_HEADROOM_FLOAT, 1.0f);
}

float Display::highlightBoost() const {
    if (!hdr_.linearOutput || !hdr_.enabled) return 1.0f;
    return std::clamp(hdr_.headroom, 1.0f, maxBoost_);
}

bool Display::onHdrStateChanged(HdrMode mode) {
    const bool wasEnabled = hdr_.enabled;
    readHdrState();
    if (wasEnabled != hdr_.enabled)
        SDL_Log("HDR: now %s (headroom %.2f)", hdr_.enabled ? "on" : "off", static_cast<double>(hdr_.headroom));
    // Turning HDR off needs nothing: SDL maps scRGB output to SDR by itself.
    return mode == HdrMode::Auto && !hdr_.linearOutput && displayReportsHdr(window_.get());
}

void Display::recreateRenderer(const PlayerSettings& settings) {
    renderer_.reset();
    hdr_ = {};
    createRenderer(settings);
    setPresentation(settings.presentation);
    SDL_Log("Display: new renderer, %s", describe().c_str());
}

void Display::toggleFullscreen() { setFullscreen(!fullscreen()); }

bool Display::fullscreen() const { return (SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_FULLSCREEN) != 0; }

void Display::setFullscreen(bool fullscreen) {
    if (!SDL_SetWindowFullscreen(window_.get(), fullscreen)) SDL_Log("Display: fullscreen change failed (%s)", SDL_GetError());
    // A window that started fullscreen has no windowed size of its own yet.
    if (!fullscreen) SDL_SetWindowResizable(window_.get(), true);
}

void Display::setPresentation(Presentation presentation) {
    const auto mode = presentation == Presentation::IntegerScale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
                                                                  : SDL_LOGICAL_PRESENTATION_LETTERBOX;
    sdlCheck(SDL_SetRenderLogicalPresentation(renderer_.get(), logicalWidth_, logicalHeight_, mode),
             "SDL_SetRenderLogicalPresentation");
}

void Display::setVsync(bool vsync) {
    if (!SDL_SetRenderVSync(renderer_.get(), vsync ? 1 : 0)) SDL_Log("Display: vsync change failed (%s)", SDL_GetError());
}

std::string Display::describe() const {
    int w = 0, h = 0;
    SDL_GetRenderOutputSize(renderer_.get(), &w, &h);
    char text[256];
    std::snprintf(text, sizeof text, "%dx%d %s | HDR %s (headroom %.1f, boost %.1f)", w, h,
                  SDL_GetRendererName(renderer_.get()),
                  hdr_.enabled ? "on" : (hdr_.linearOutput ? "ready" : "off"),
                  static_cast<double>(hdr_.headroom), static_cast<double>(highlightBoost()));
    return text;
}

} // namespace iron
