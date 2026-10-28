add_library(IronRingCore STATIC
    PlayerSettings.h
    PlayerSettings.cpp)
target_include_directories(IronRingCore PUBLIC "${PROJECT_SOURCE_DIR}/Source")
ir_configure_target(IronRingCore)
