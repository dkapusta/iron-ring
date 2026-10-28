#pragma once

#include "Core/PlayerSettings.h"
#include "Platform/Display.h"
#include "Vfs/VirtualFileSystem.h"

#include <filesystem>
#include <memory>
#include <string>

namespace iron {

class AssetLibrary;
class AudioSystem;
class Scene;

struct ProductInfo {
    std::string company;
    std::string product;
    std::filesystem::path userFolder; // Player.log and F12 screenshots
};

// Composition root: owns the subsystems and runs the fixed-step main loop.
class Application {
public:
    Application(const PlayerSettings& settings, const ProductInfo& info);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    void mountData();
    void handleEvent(const SDL_Event& event);
    void render(const std::filesystem::path& capturePath);
    void drawOverlay();
    void saveScreenshot(const std::filesystem::path& path) const;

    PlayerSettings settings_;
    std::filesystem::path screenshotFolder_;
    vfs::VirtualFileSystem vfs_;
    std::unique_ptr<Display> display_;
    std::unique_ptr<AudioSystem> audio_;
    std::unique_ptr<AssetLibrary> assets_;
    std::unique_ptr<Scene> scene_;
    bool running_ = true;
    bool overlay_ = false;
    bool pendingCapture_ = false;
    double fps_ = 0;
};

} // namespace iron
