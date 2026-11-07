# Unity-style player layout (docs/adr/0003-unity-style-player-layout.md).
#
#   <build>/Player/<Config>/
#     IronRing.exe | IronRing.x86_64          launcher (tiny: calls into the engine)
#     UnityPlayer.dll | UnityPlayer.so        engine module (SDL3, SDL3_mixer linked in)
#     IronRing_Data/
#       app.info  boot.config                 plain text, written at configure time
#       globalgamemanagers                    ProjectSettings/*  (ZIP container)
#       resources.assets                      Assets/Resources/** (ZIP container)
#       sharedassets0.assets                  gameplay content    (ZIP container)
#       sharedassets0.resource                audio streams       (ZIP container, stored)
#
# On macOS the player is an application bundle, laid out like a Unity Mac player:
#
#   <build>/Player/<Config>/IronRing.app/Contents/
#     Info.plist
#     MacOS/IronRing                          launcher
#     Frameworks/UnityPlayer.dylib            engine module
#     Resources/IronRing.icns                 Finder / Dock icon
#     Resources/Data/                         the same files as IronRing_Data
#
# SDL_GetBasePath() is Contents/Resources/ inside a bundle, so the engine finds
# Data/ the same way it finds IronRing_Data/ next to the executable elsewhere.
#
# The archive list is defined by ProjectSettings/AssetPipeline.json.

set(IR_PLAYER_OUTPUT_DIR "${PROJECT_BINARY_DIR}/Player/$<CONFIG>")
if(APPLE)
    set(IR_APP_BUNDLE_DIR  "${IR_PLAYER_OUTPUT_DIR}/${IR_PRODUCT_NAME}.app")
    set(IR_DATA_DIR_NAME   "Data")
    set(IR_DATA_OUTPUT_DIR "${IR_APP_BUNDLE_DIR}/Contents/Resources/${IR_DATA_DIR_NAME}")
else()
    set(IR_DATA_DIR_NAME   "${IR_PRODUCT_NAME}_Data")
    set(IR_DATA_OUTPUT_DIR "${IR_PLAYER_OUTPUT_DIR}/${IR_DATA_DIR_NAME}")
endif()

if(WIN32)
    set(IR_PLAYER_EXE_SUFFIX ".exe")
elseif(APPLE)
    set(IR_PLAYER_EXE_SUFFIX "")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(IR_PLAYER_EXE_SUFFIX ".x86_64")
else()
    set(IR_PLAYER_EXE_SUFFIX ".${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Places a target's binaries in the player folder.
# On macOS the launcher (a MACOSX_BUNDLE executable) creates the bundle in the
# player folder, and shared libraries go to its Contents/Frameworks.
function(ir_place_in_player target)
    if(APPLE)
        set(_library_dir "${IR_APP_BUNDLE_DIR}/Contents/Frameworks")
    else()
        set(_library_dir "${IR_PLAYER_OUTPUT_DIR}")
    endif()
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${IR_PLAYER_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${_library_dir}"
        # Import libraries / static archives stay out of the shipped folder.
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/$<CONFIG>"
        PDB_OUTPUT_DIRECTORY     "${PROJECT_BINARY_DIR}/symbols/$<CONFIG>")
endfunction()

# Windows VERSIONINFO (and optionally the icon) for a player binary. The
# strings name our product and company; nothing claims to be Unity (ADR-0003).
function(ir_add_version_resource target description filename type icon)
    if(NOT WIN32)
        return()
    endif()
    set(IR_RC_DESCRIPTION "${description}")
    set(IR_RC_FILENAME "${filename}")
    set(IR_RC_FILETYPE "${type}")
    if(icon)
        set(IR_RC_ICON_LINE "1 ICON \"${icon}\"")
    else()
        set(IR_RC_ICON_LINE "")
    endif()
    set(rc "${PROJECT_BINARY_DIR}/generated/${target}.rc")
    configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/version.rc.in" "${rc}" @ONLY)
    target_sources(${target} PRIVATE "${rc}")
endfunction()
