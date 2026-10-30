#pragma once

#include "Core/PlayerPrefs.h"
#include "Core/PlayerSettings.h"
#include "Game/SceneStack.h"
#include "Platform/Display.h"
#include "Vfs/VirtualFileSystem.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace iron {

class AssetLibrary;
class AudioSystem;
class Controls;
class GameData;
struct Services;

struct ProductInfo {
    std::string company;
    std::string product;
    std::filesystem::path userFolder; // Player.log and F12 screenshots
};

// Composition root: owns the subsystems and runs the fixed-step main loop.
class Application {
public:
    Application(const PlayerSettings& settings, PlayerPrefs prefs, const ProductInfo& info);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    void mountData();
    void setWindowIcon();
    void handleEvent(const SDL_Event& event);
    void openFirstScene();
    void resetRenderer();
    void playKeyScript(int frame);
    void render(const std::filesystem::path& capturePath);
    void drawOverlay();
    void saveScreenshot(const std::filesystem::path& path) const;

    PlayerSettings settings_;
    PlayerPrefs prefs_;
    std::filesystem::path screenshotFolder_;
    vfs::VirtualFileSystem vfs_;
    std::unique_ptr<Display> display_;
    std::unique_ptr<AudioSystem> audio_;
    std::unique_ptr<AssetLibrary> assets_;
    std::unique_ptr<Controls> controls_;
    std::unique_ptr<GameData> data_;
    SceneStack scenes_;
    std::unique_ptr<Services> services_;
    int logicalWidth_ = 640, logicalHeight_ = 360;
    struct ScriptedKey {
        int first, last; // held from frame `first` through `last`
        SDL_Scancode key;
    };
    std::vector<ScriptedKey> keyScript_; // -keys, for batch tests of the menus
    bool running_ = true;
    bool overlay_ = false;
    bool pendingCapture_ = false;
    double fps_ = 0;
};

} // namespace iron
