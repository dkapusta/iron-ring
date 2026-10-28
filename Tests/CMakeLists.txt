add_executable(IronRingTests
    main.cpp
    PlayerSettingsTests.cpp
    SimulationTests.cpp
    VfsTests.cpp)
target_link_libraries(IronRingTests PRIVATE
    IronRingCore IronRingSimulation IronRingVfs doctest::doctest)
ir_configure_target(IronRingTests)

add_test(NAME IronRingTests COMMAND IronRingTests)

# End-to-end: the packer must bake the real project content without errors.
add_test(NAME AssetPackerBakesProject
    COMMAND AssetPacker
            --project  "${PROJECT_SOURCE_DIR}"
            --pipeline "${PROJECT_SOURCE_DIR}/ProjectSettings/AssetPipeline.json"
            --output   "${CMAKE_CURRENT_BINARY_DIR}/PackerSmoke")
