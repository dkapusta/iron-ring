# Online play shared by the player and IronRingServer: protocol, codes,
# invites, the ENet transport (ADR-0026). No SDL.
add_library(IronRingNet STATIC
    Net.h
    Codes.h     Codes.cpp
    Protocol.h  Protocol.cpp
    Transport.h Transport.cpp)
target_include_directories(IronRingNet PUBLIC "${PROJECT_SOURCE_DIR}/Source")
target_link_libraries(IronRingNet PUBLIC IronRingSimulation PRIVATE enet)
ir_configure_target(IronRingNet)
