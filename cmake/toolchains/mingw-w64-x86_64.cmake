# Cross-compile Windows x64 binaries with MinGW-w64 on Linux.
#   cmake -S . -B Build/mingw-cross -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
#     -DIR_ASSET_PACKER=$PWD/Build/release/Tools/Release/AssetPacker
# Debian/Ubuntu: apt install g++-mingw-w64-x86-64-posix
# The posix thread model is required for std::thread / std::mutex.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(_prefix x86_64-w64-mingw32)
find_program(CMAKE_C_COMPILER   NAMES ${_prefix}-gcc-posix ${_prefix}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${_prefix}-g++-posix ${_prefix}-g++ REQUIRED)
find_program(CMAKE_RC_COMPILER  NAMES ${_prefix}-windres REQUIRED)

set(CMAKE_FIND_ROOT_PATH /usr/${_prefix})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
