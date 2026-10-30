#pragma once

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Core/PlayerPrefs.h"
#include "Core/PlayerSettings.h"
#include "Game/Controls.h"
#include "Game/GameData.h"
#include "Game/SceneStack.h"
#include "Platform/Display.h"

#include <functional>

namespace iron {

// What every scene can reach. Owned by the Application; scenes keep a reference.
struct Services {
    AssetLibrary& assets;
    AudioSystem& audio;
    Display& display;
    Controls& controls;
    PlayerSettings& settings; // live values; the Options screen edits them
    PlayerPrefs& prefs;
    GameData& data;
    SceneStack& scenes;
    float width = 640, height = 360; // logical resolution
    // A new renderer for the current settings (the HDR option); textures reload.
    std::function<void()> resetRenderer;
};

} // namespace iron
