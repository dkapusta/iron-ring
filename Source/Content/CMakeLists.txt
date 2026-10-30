# Content formats and validation: no SDL, shared by the runtime, AssetPacker and tests.
add_library(IronRingContent STATIC
    Atlas.h      Atlas.cpp
    Fighter.h    Fighter.cpp
    Paths.h      Paths.cpp
    Stage.h      Stage.cpp
    Validation.h Validation.cpp)
target_include_directories(IronRingContent PUBLIC "${PROJECT_SOURCE_DIR}/Source")
target_link_libraries(IronRingContent PUBLIC IronRingSimulation nlohmann_json::nlohmann_json)
ir_configure_target(IronRingContent)
