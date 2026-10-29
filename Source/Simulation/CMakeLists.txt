add_library(IronRingSimulation STATIC
    Round.h Round.cpp
    Spec.h  Spec.cpp)
target_include_directories(IronRingSimulation PUBLIC "${PROJECT_SOURCE_DIR}/Source")
ir_configure_target(IronRingSimulation)
