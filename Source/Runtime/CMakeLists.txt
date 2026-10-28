# The engine module. Named UnityPlayer by default (IR_PLAYER_LIBRARY_NAME),
# shipped next to the launcher, with SDL3 and SDL3_mixer linked in statically.
add_library(IronRingEngine SHARED
    Public/IronRing/Player.h
    Assets/AssetLibrary.h       Assets/AssetLibrary.cpp
    Audio/AudioSystem.h         Audio/AudioSystem.cpp
    Game/FightScene.h           Game/FightScene.cpp
    Game/InputMapper.h          Game/InputMapper.cpp
    Game/Scene.h
    Game/SpriteAtlas.h          Game/SpriteAtlas.cpp
    Platform/Display.h          Platform/Display.cpp
    Platform/PlayerLog.h        Platform/PlayerLog.cpp
    Platform/SdlHandles.h
    Player/Application.h        Player/Application.cpp
    Player/PlayerMain.cpp)

target_include_directories(IronRingEngine
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/Public"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(IronRingEngine PRIVATE
    IronRingCore IronRingSimulation IronRingVfs
    SDL3::SDL3-static SDL3_mixer::SDL3_mixer-static)
target_compile_definitions(IronRingEngine PRIVATE
    IR_ENGINE_BUILD
    IR_VERSION_STRING="${PROJECT_VERSION}"
    IR_COMPANY_NAME="${IR_COMPANY_NAME}"
    IR_PRODUCT_DISPLAY_NAME="${IR_PRODUCT_DISPLAY_NAME}"
    IR_DATA_DIR_NAME="${IR_DATA_DIR_NAME}"
    IR_PLAYER_LIBRARY_NAME="${IR_PLAYER_LIBRARY_NAME}")
if(WIN32)
    target_link_libraries(IronRingEngine PRIVATE shell32)
endif()

ir_configure_target(IronRingEngine)
ir_place_in_player(IronRingEngine)
set_target_properties(IronRingEngine PROPERTIES
    OUTPUT_NAME "${IR_PLAYER_LIBRARY_NAME}"
    PREFIX "")                      # UnityPlayer.so, not libUnityPlayer.so
