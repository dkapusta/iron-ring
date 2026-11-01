# The Iron Ring dedicated server (ADR-0026, docs/server.md). No SDL.
add_library(IronRingServerCore STATIC
    Server.h    Server.cpp
    GameRules.h GameRules.cpp
    Settings.h  Settings.cpp)
target_include_directories(IronRingServerCore PUBLIC "${PROJECT_SOURCE_DIR}/Source")
target_link_libraries(IronRingServerCore
    PUBLIC  IronRingNet IronRingSimulation IronRingContent IronRingVfs IronRingCore
    PRIVATE enet nlohmann_json::nlohmann_json)
ir_configure_target(IronRingServerCore)

add_executable(IronRingServer main.cpp)
target_link_libraries(IronRingServer PRIVATE IronRingServerCore)
target_compile_definitions(IronRingServer PRIVATE IR_VERSION_STRING="${PROJECT_VERSION}")
ir_configure_target(IronRingServer)
set_target_properties(IronRingServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Server/$<CONFIG>")
# The example configuration next to the binary (never overwriting an edited one).
add_custom_command(TARGET IronRingServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/server.cfg"
            "$<TARGET_FILE_DIR:IronRingServer>/server.cfg.example")

# cmake --install <build> --component Server --prefix <dir>: the server alone.
install(TARGETS IronRingServer RUNTIME DESTINATION . COMPONENT Server EXCLUDE_FROM_ALL)
install(FILES server.cfg DESTINATION . RENAME server.cfg.example COMPONENT Server EXCLUDE_FROM_ALL)
