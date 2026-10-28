add_library(IronRingSimulation STATIC
    Combat.h
    Combat.cpp)
target_include_directories(IronRingSimulation PUBLIC "${PROJECT_SOURCE_DIR}/Source")
ir_configure_target(IronRingSimulation)
