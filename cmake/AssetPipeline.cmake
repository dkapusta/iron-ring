# Bakes Assets/ and ProjectSettings/ into the player's <name>_Data folder.
# The archive list comes from ProjectSettings/AssetPipeline.json so CMake and
# the AssetPacker tool share one source of truth (docs/adr/0004-asset-archives.md).

set(_pipeline "${PROJECT_SOURCE_DIR}/ProjectSettings/AssetPipeline.json")
file(READ "${_pipeline}" _pipeline_json)
string(JSON _archive_count LENGTH "${_pipeline_json}" archives)
math(EXPR _last "${_archive_count} - 1")

set(_outputs "")
foreach(_i RANGE ${_last})
    string(JSON _name GET "${_pipeline_json}" archives ${_i} output)
    list(APPEND _outputs "${IR_DATA_OUTPUT_DIR}/${_name}")
endforeach()

# Every source file can influence an archive; CONFIGURE_DEPENDS re-globs on build.
file(GLOB_RECURSE _asset_sources CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/Assets/*"
    "${PROJECT_SOURCE_DIR}/ProjectSettings/*")

# The packer runs on the build machine. When cross-compiling (e.g. MinGW on
# Linux) the freshly built AssetPacker can't run, so point IR_ASSET_PACKER at
# a host build of it, or set CMAKE_CROSSCOMPILING_EMULATOR (e.g. wine).
set(IR_ASSET_PACKER "" CACHE FILEPATH "Host-runnable AssetPacker (cross-compiling only)")
if(IR_ASSET_PACKER)
    set(_packer "${IR_ASSET_PACKER}")
    set(_packer_dependency "")
elseif(CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
    message(FATAL_ERROR "Cross-compiling: set IR_ASSET_PACKER to a host AssetPacker executable")
else()
    set(_packer AssetPacker)
    set(_packer_dependency AssetPacker)
endif()

add_custom_command(
    OUTPUT  ${_outputs}
    COMMAND ${_packer}
            --project  "${PROJECT_SOURCE_DIR}"
            --pipeline "${_pipeline}"
            --output   "${IR_DATA_OUTPUT_DIR}"
    DEPENDS ${_packer_dependency} ${_asset_sources}
    COMMENT "Packing assets into ${IR_DATA_DIR_NAME}"
    VERBATIM)

# Plain-text descriptors, generated per configuration.
set(_generated "${PROJECT_BINARY_DIR}/generated")
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/app.info.in"    "${_generated}/app.info"    @ONLY)
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/boot.config.in" "${_generated}/boot.config" @ONLY)
file(GENERATE OUTPUT "${IR_DATA_OUTPUT_DIR}/app.info"    INPUT "${_generated}/app.info")
file(GENERATE OUTPUT "${IR_DATA_OUTPUT_DIR}/boot.config" INPUT "${_generated}/boot.config")

add_custom_target(PlayerData ALL DEPENDS ${_outputs})
add_dependencies(IronRingPlayer PlayerData)
