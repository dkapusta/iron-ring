#!/usr/bin/env bash
# Cross-compiles the Iron Ring player for Windows x64 on Linux with MinGW-w64, and
# packs a release zip (docs/building.md). On Windows itself, use build-windows.bat.
#
#   ./build-windows.sh            build, then Dist/IronRing-<version>-windows-x64.zip
#
# Needs cmake, ninja and the posix-thread MinGW-w64 compiler:
#   sudo apt install build-essential cmake ninja-build git g++-mingw-w64-x86-64-posix
# The game data is packed by a native AssetPacker, built first in Build/host-tools.
set -euo pipefail
cd "$(dirname "$0")"
source Tools/Build/common.sh

case "${1:-}" in
    "") ;;
    -h|--help) sed -n '2,9p' "$0"; exit 0 ;;
    *) echo "Unknown option $1 (see --help)" >&2; exit 2 ;;
esac

HINT="On Ubuntu/Debian: sudo apt install build-essential cmake ninja-build git g++-mingw-w64-x86-64-posix"
for tool in cmake ninja c++ git x86_64-w64-mingw32-g++-posix; do need "$tool" "$HINT"; done

HOST=Build/host-tools
BUILD=Build/mingw-release
JOBS=$(build_jobs)

echo "== Building the native AssetPacker ($HOST) =="
cmake -S . -B "$HOST" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DIR_BUILD_PLAYER=OFF -DIR_BUILD_SERVER=OFF -DIR_BUILD_TESTS=OFF > /dev/null
cmake --build "$HOST" --target AssetPacker -j "$JOBS"
PACKER="$PWD/$HOST/Tools/Release/AssetPacker"

echo "== Building the Windows player at $(git rev-parse --short HEAD 2>/dev/null || echo '?') ($JOBS jobs) =="
cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
    -DIR_ASSET_PACKER="$PACKER" -DIR_BUILD_TESTS=OFF > /dev/null
cmake --build "$BUILD" -j "$JOBS"

PLAYER_DIR="$BUILD/Player/Release"
VERSION=$(project_version "$BUILD")
NAME="IronRing-$VERSION-windows-x64"
STAGE="Dist/$NAME"
echo "== Packing $NAME =="
rm -rf "$STAGE"
mkdir -p "$STAGE"
# The player folder: IronRing.exe, UnityPlayer.dll, IronRing_Data and, only if the
# toolchain could not link its runtime statically, its DLLs (docs/adr/0009-mingw-static-runtime.md).
cp -a "$PLAYER_DIR"/. "$STAGE/"
release_notes "$STAGE" "$BUILD"
zip_folder "$STAGE" "Dist/$NAME.zip"
echo "== Ready: Dist/$NAME.zip =="
echo "   Unzip anywhere on Windows 10 or 11 (x64) and run $NAME\\IronRing.exe."
