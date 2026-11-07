cmake_minimum_required(VERSION 3.25)

project(IronRing
    VERSION 0.4.2
    DESCRIPTION "Early-90s style arcade fighting game on C++20 / SDL3"
    HOMEPAGE_URL "https://github.com/dkapusta/iron-ring"
    LANGUAGES C CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use: cmake --preset <name>")
endif()

# ---------------------------------------------------------------------------
# Product identity. These names shape the Unity-style player layout
# (see docs/adr/0003-unity-style-player-layout.md).
# ---------------------------------------------------------------------------
set(IR_PRODUCT_NAME         "IronRing"     CACHE STRING "File stem of the player executable and <name>_Data folder")
set(IR_PRODUCT_DISPLAY_NAME "Iron Ring"    CACHE STRING "Human-readable product name (window title, app.info)")
set(IR_COMPANY_NAME         "dkapusta"     CACHE STRING "Company name (app.info, user data folder)")
set(IR_PLAYER_LIBRARY_NAME  "UnityPlayer"  CACHE STRING "File stem of the engine shared library")
set(IR_APP_ID "io.github.${IR_COMPANY_NAME}.${IR_PRODUCT_NAME}" CACHE STRING
    "Reverse-DNS application id: Wayland app_id / X11 WM_CLASS and the Linux .desktop file name")

option(IR_BUILD_TESTS          "Build unit tests"                  ON)
option(IR_BUILD_PLAYER         "Build the SDL3 player (engine + launcher + data)" ON)
option(IR_BUILD_SERVER         "Build IronRingServer, the dedicated server (no SDL)" ON)
option(IR_WARNINGS_AS_ERRORS   "Treat project warnings as errors"  OFF)

if(WIN32)
    enable_language(RC) # version resources and the application icon
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Static third-party libraries are linked into a shared engine module.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CompilerOptions)
include(Dependencies)
include(PlayerLayout)

# SDL-free libraries: simulation, archive I/O, settings parsing.
add_subdirectory(Source/Core)
add_subdirectory(Source/Vfs)
add_subdirectory(Source/Simulation)
add_subdirectory(Source/Content)
add_subdirectory(Source/Net)

# Host tool that bakes Assets/ and ProjectSettings/ into <name>_Data archives.
add_subdirectory(Source/Tools/AssetPacker)

if(IR_BUILD_SERVER)
    add_subdirectory(Source/Server)
endif()

if(IR_BUILD_PLAYER)
    add_subdirectory(Source/Runtime)
    add_subdirectory(Source/Launcher)
    include(AssetPipeline)
    include(Install)
endif()

if(IR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

message(STATUS "Iron Ring ${PROJECT_VERSION}: player output -> ${PROJECT_BINARY_DIR}/Player/<Config>/")
