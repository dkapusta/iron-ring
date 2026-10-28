add_executable(IronRingPlayer WIN32 main.cpp)
target_link_libraries(IronRingPlayer PRIVATE IronRingEngine)
ir_configure_target(IronRingPlayer)
ir_place_in_player(IronRingPlayer)
set_target_properties(IronRingPlayer PROPERTIES
    OUTPUT_NAME "${IR_PRODUCT_NAME}"
    SUFFIX      "${IR_PLAYER_EXE_SUFFIX}"
    # Find UnityPlayer.so next to the executable, like a Unity Linux player.
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "$ORIGIN"
    VS_DEBUGGER_WORKING_DIRECTORY "${IR_PLAYER_OUTPUT_DIR}")
if(MINGW)
    target_link_options(IronRingPlayer PRIVATE -municode)
endif()
if(APPLE)
    set_target_properties(IronRingPlayer PROPERTIES
        INSTALL_RPATH "@executable_path")
endif()
