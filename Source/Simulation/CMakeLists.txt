add_library(IronRingSimulation STATIC
    Match.h Match.cpp
    Cpu.h   Cpu.cpp
    Round.h Round.cpp
    Spec.h  Spec.cpp
    Training.h Training.cpp
    Serialize.h Serialize.cpp
    Replay.h Replay.cpp
    Rollback.h Rollback.cpp)
target_include_directories(IronRingSimulation PUBLIC "${PROJECT_SOURCE_DIR}/Source")
ir_configure_target(IronRingSimulation)
