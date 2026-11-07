add_executable(IronRingPlayer WIN32 main.cpp) # MACOSX_BUNDLE on macOS, below
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
    # IronRing.app (cmake/PlayerLayout.cmake): the launcher is Contents/MacOS/IronRing
    # and finds the engine in Contents/Frameworks.
    set(_plist "${PROJECT_BINARY_DIR}/generated/Info.plist")
    configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/Info.plist.in" "${_plist}" @ONLY)
    set(_icns "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${IR_PRODUCT_NAME}.icns")
    target_sources(IronRingPlayer PRIVATE "${_icns}")
    set_source_files_properties("${_icns}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(IronRingPlayer PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${_plist}"
        INSTALL_RPATH "@executable_path/../Frameworks")
endif()
ir_add_version_resource(IronRingPlayer "${IR_PRODUCT_DISPLAY_NAME}" "${IR_PRODUCT_NAME}.exe" VFT_APP
                        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/IronRing.ico")
