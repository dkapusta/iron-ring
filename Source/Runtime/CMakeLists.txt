# The engine module. Named UnityPlayer by default (IR_PLAYER_LIBRARY_NAME),
# shipped next to the launcher, with SDL3 and SDL3_mixer linked in statically.
add_library(IronRingEngine SHARED
    Public/IronRing/Player.h
    Assets/AssetLibrary.h       Assets/AssetLibrary.cpp
    Audio/AudioSystem.h         Audio/AudioSystem.cpp
    Game/ArcadeScene.h          Game/ArcadeScene.cpp
    Game/CharacterSelectScene.h Game/CharacterSelectScene.cpp
    Game/Controls.h             Game/Controls.cpp
    Game/ControlsScene.h        Game/ControlsScene.cpp
    Game/Effects.h              Game/Effects.cpp
    Game/FighterView.h          Game/FighterView.cpp
    Game/FightMenuScene.h       Game/FightMenuScene.cpp
    Game/FightScene.h           Game/FightScene.cpp
    Game/GameData.h             Game/GameData.cpp
    Game/Hud.h                  Game/Hud.cpp
    Game/MenuView.h             Game/MenuView.cpp
    Game/MoveListScene.h        Game/MoveListScene.cpp
    Game/OnlineScenes.h         Game/OnlineScenes.cpp
    Game/OnlineSession.h        Game/OnlineSession.cpp
    Game/OptionsScene.h         Game/OptionsScene.cpp
    Game/Scene.h
    Game/SceneStack.h           Game/SceneStack.cpp
    Game/Services.h
    Game/StageSelectScene.h     Game/StageSelectScene.cpp
    Game/StageView.h            Game/StageView.cpp
    Game/TitleScene.h           Game/TitleScene.cpp
    Game/TrainingOverlay.h      Game/TrainingOverlay.cpp
    Game/InputDisplay.h         Game/InputDisplay.cpp
    Platform/Display.h          Platform/Display.cpp
    Platform/PlayerLog.h        Platform/PlayerLog.cpp
    Platform/SdlHandles.h
    Player/Application.h        Player/Application.cpp
    Player/PlayerMain.cpp
    Render/BitmapFont.h         Render/BitmapFont.cpp
    Render/SpriteSheet.h        Render/SpriteSheet.cpp
    Render/Ui.h                 Render/Ui.cpp)

target_include_directories(IronRingEngine
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/Public"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(IronRingEngine PRIVATE
    IronRingContent IronRingCore IronRingSimulation IronRingVfs IronRingNet
    SDL3::SDL3-static SDL3_mixer::SDL3_mixer-static)
target_compile_definitions(IronRingEngine PRIVATE
    IR_ENGINE_BUILD
    IR_VERSION_STRING="${PROJECT_VERSION}"
    IR_COMPANY_NAME="${IR_COMPANY_NAME}"
    IR_PRODUCT_DISPLAY_NAME="${IR_PRODUCT_DISPLAY_NAME}"
    IR_DATA_DIR_NAME="${IR_DATA_DIR_NAME}"
    IR_PLAYER_LIBRARY_NAME="${IR_PLAYER_LIBRARY_NAME}"
    IR_APP_ID="${IR_APP_ID}")
if(WIN32)
    target_link_libraries(IronRingEngine PRIVATE shell32)
endif()

ir_configure_target(IronRingEngine)
ir_place_in_player(IronRingEngine)
set_target_properties(IronRingEngine PROPERTIES
    OUTPUT_NAME "${IR_PLAYER_LIBRARY_NAME}"
    PREFIX "")                      # UnityPlayer.so, not libUnityPlayer.so
ir_add_version_resource(IronRingEngine "${IR_PRODUCT_DISPLAY_NAME} player engine" "${IR_PLAYER_LIBRARY_NAME}.dll"
                        VFT_DLL "")
