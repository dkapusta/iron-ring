add_executable(IronRingTests
    main.cpp
    ContentTests.cpp
    FrontEndTests.cpp
    GameplayTests.cpp
    PlayerSettingsTests.cpp
    SimulationTests.cpp
    VfsTests.cpp
    OnlineTests.cpp)
target_link_libraries(IronRingTests PRIVATE
    IronRingContent IronRingCore IronRingSimulation IronRingVfs IronRingNet doctest::doctest)
if(TARGET IronRingServerCore)
    target_sources(IronRingTests PRIVATE NetTests.cpp)
    target_link_libraries(IronRingTests PRIVATE IronRingServerCore)
endif()
target_compile_definitions(IronRingTests PRIVATE IR_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
ir_configure_target(IronRingTests)

add_test(NAME IronRingTests COMMAND IronRingTests)

# End-to-end: the packer must bake the real project content without errors.
add_test(NAME AssetPackerBakesProject
    COMMAND AssetPacker
            --project  "${PROJECT_SOURCE_DIR}"
            --pipeline "${PROJECT_SOURCE_DIR}/ProjectSettings/AssetPipeline.json"
            --output   "${CMAKE_CURRENT_BINARY_DIR}/PackerSmoke")

# The dedicated server binary opens a port and exits cleanly.
if(TARGET IronRingServer)
    add_test(NAME IronRingServerCheck COMMAND IronRingServer --check --verify 0 --port 0)
endif()
