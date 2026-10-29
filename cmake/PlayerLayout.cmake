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
# The archive list is defined by ProjectSettings/AssetPipeline.json.

set(IR_PLAYER_OUTPUT_DIR "${PROJECT_BINARY_DIR}/Player/$<CONFIG>")
set(IR_DATA_DIR_NAME     "${IR_PRODUCT_NAME}_Data")
set(IR_DATA_OUTPUT_DIR   "${IR_PLAYER_OUTPUT_DIR}/${IR_DATA_DIR_NAME}")

if(WIN32)
    set(IR_PLAYER_EXE_SUFFIX ".exe")
elseif(APPLE)
    # macOS .app bundle layout is planned (ROADMAP M1); flat layout for now.
    set(IR_PLAYER_EXE_SUFFIX "")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(IR_PLAYER_EXE_SUFFIX ".x86_64")
else()
    set(IR_PLAYER_EXE_SUFFIX ".${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Places a target's binaries in the player folder.
function(ir_place_in_player target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${IR_PLAYER_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${IR_PLAYER_OUTPUT_DIR}"
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
