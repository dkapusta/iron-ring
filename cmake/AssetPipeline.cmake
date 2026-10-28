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

add_custom_command(
    OUTPUT  ${_outputs}
    COMMAND AssetPacker
            --project  "${PROJECT_SOURCE_DIR}"
            --pipeline "${_pipeline}"
            --output   "${IR_DATA_OUTPUT_DIR}"
    DEPENDS AssetPacker ${_asset_sources}
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
