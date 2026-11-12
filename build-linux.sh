#!/usr/bin/env bash
# Builds the Iron Ring player for Linux (x86-64 or arm64) and packs a release zip
# (docs/building.md). Needs cmake, ninja, a C++20 compiler and SDL's build packages.
#
#   ./build-linux.sh              build, then Dist/IronRing-<version>-linux-<arch>.zip
#   ./build-linux.sh --test       also run the tests first
#   ./build-linux.sh --run [...]  build, then start the game (extra arguments go to it)
#
# Uses the release preset (Build/release), so re-runs only rebuild what changed.
set -euo pipefail
cd "$(dirname "$0")"
source Tools/Build/common.sh

TEST=0 RUN=0
while [ $# -gt 0 ]; do
    case "$1" in
        --test) TEST=1; shift ;;
        --run) RUN=1; shift; break ;;
        -h|--help) sed -n '2,9p' "$0"; exit 0 ;;
        *) echo "Unknown option $1 (see --help)" >&2; exit 2 ;;
    esac
done

if [ "$(uname -s)" != Linux ]; then
    echo "build-linux.sh builds on Linux. On macOS use ./build-macos.sh; for Windows, ./build-windows.sh or build-windows.bat." >&2
    exit 2
fi
HINT="On Ubuntu/Debian: sudo apt install build-essential cmake ninja-build git libx11-dev libxext-dev \
libxcursor-dev libxi-dev libxrandr-dev libxss-dev libxfixes-dev libxtst-dev libwayland-dev libxkbcommon-dev \
libegl1-mesa-dev libgl1-mesa-dev libasound2-dev libpulse-dev libudev-dev libdbus-1-dev (docs/building.md)"
for tool in cmake ninja c++ git; do need "$tool" "$HINT"; done

BUILD=Build/release
JOBS=$(build_jobs)
echo "== Configuring (release preset) =="
cmake --preset release > /dev/null
echo "== Building at $(git rev-parse --short HEAD 2>/dev/null || echo '?') ($JOBS jobs) =="
cmake --build --preset release -j "$JOBS"
if [ "$TEST" = 1 ]; then
    echo "== Testing =="
    ctest --preset release
fi

PLAYER_DIR="$BUILD/Player/Release"
case "$(uname -m)" in
    x86_64|amd64) ARCH=x64; EXE=IronRing.x86_64 ;;
    aarch64|arm64) ARCH=arm64; EXE=IronRing.aarch64 ;;
    *) ARCH="$(uname -m)"; EXE="IronRing.$(uname -m)" ;;
esac

if [ "$RUN" = 1 ]; then
    exec "$PLAYER_DIR/$EXE" "$@"
fi

VERSION=$(project_version "$BUILD")
NAME="IronRing-$VERSION-linux-$ARCH"
STAGE="Dist/$NAME"
echo "== Packing $NAME =="
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -a "$PLAYER_DIR/$EXE" "$PLAYER_DIR/UnityPlayer.so" "$PLAYER_DIR/IronRing_Data" "$STAGE/"
release_notes "$STAGE" "$BUILD"
zip_folder "$STAGE" "Dist/$NAME.zip"
echo "== Ready: Dist/$NAME.zip =="
GLIBC=$(objdump -T "$STAGE/$EXE" "$STAGE/UnityPlayer.so" 2>/dev/null | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1 || true)
echo "   Unzip anywhere and run $NAME/$EXE.${GLIBC:+ Needs ${GLIBC/_/ } or newer (docs/building.md).}"
