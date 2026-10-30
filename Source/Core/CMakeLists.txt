add_library(IronRingCore STATIC
    Menu.h          Menu.cpp
    PlayerPrefs.h   PlayerPrefs.cpp
    PlayerSettings.h
    PlayerSettings.cpp)
target_include_directories(IronRingCore PUBLIC "${PROJECT_SOURCE_DIR}/Source")
ir_configure_target(IronRingCore)
