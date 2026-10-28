add_executable(AssetPacker
    main.cpp
    Pipeline.h
    Pipeline.cpp)
target_link_libraries(AssetPacker PRIVATE IronRingVfs)
ir_configure_target(AssetPacker)
set_target_properties(AssetPacker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Tools/$<CONFIG>")
