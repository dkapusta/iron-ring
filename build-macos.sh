#!/usr/bin/env bash
# Builds Iron Ring for macOS as IronRing.app (Apple silicon and Intel in one
# binary) and packs a release zip (docs/building.md).
#
#   ./build-macos.sh              build, then Dist/IronRing-<version>-macos.zip
#   ./build-macos.sh --test       also run the tests first
#   ./build-macos.sh --run [...]  build, then start the game (extra arguments go to it)
#
# Needs the Xcode command line tools (xcode-select --install), cmake and ninja
# (brew install cmake ninja). Uses the macos preset (Build/macos).
#
# Signing (optional; without it the app is signed ad hoc, and macOS asks once
# before the first start, see docs/building.md):
#   IR_CODESIGN_IDENTITY="Developer ID Application: Name (TEAMID)"   sign for distribution
#   IR_NOTARY_PROFILE=<profile>   also notarize and staple, with credentials saved by
#                                 xcrun notarytool store-credentials <profile>
# ARCHS="arm64" (or "x86_64") builds for one architecture only, which is faster.
set -euo pipefail
cd "$(dirname "$0")"
source Tools/Build/common.sh

TEST=0 RUN=0
while [ $# -gt 0 ]; do
    case "$1" in
        --test) TEST=1; shift ;;
        --run) RUN=1; shift; break ;;
        -h|--help) sed -n '2,18p' "$0"; exit 0 ;;
        *) echo "Unknown option $1 (see --help)" >&2; exit 2 ;;
    esac
done

if [ "$(uname -s)" != Darwin ]; then
    echo "build-macos.sh builds on a Mac. On Linux use ./build-linux.sh (or ./build-windows.sh for Windows)." >&2
    exit 2
fi
need xcrun "Install the Xcode command line tools: xcode-select --install"
for tool in cmake ninja git; do need "$tool" "With Homebrew: brew install cmake ninja git"; done

BUILD=Build/macos
JOBS=$(build_jobs)
ARCHS="${ARCHS:-arm64;x86_64}"
echo "== Configuring (macos preset, $ARCHS) =="
cmake --preset macos -DCMAKE_OSX_ARCHITECTURES="$ARCHS" > /dev/null
echo "== Building at $(git rev-parse --short HEAD 2>/dev/null || echo '?') ($JOBS jobs) =="
cmake --build --preset macos -j "$JOBS"
if [ "$TEST" = 1 ]; then
    echo "== Testing =="
    ctest --preset macos
fi

APP="$BUILD/Player/Release/IronRing.app"
if [ "$RUN" = 1 ]; then
    exec "$APP/Contents/MacOS/IronRing" "$@"
fi

VERSION=$(release_version "$(project_version "$BUILD")")
NAME="IronRing-$VERSION-macos"
STAGE="Dist/$NAME"
echo "== Packing $NAME =="
rm -rf "$STAGE"
mkdir -p "$STAGE"
ditto "$APP" "$STAGE/IronRing.app"
release_notes "$STAGE" macos IronRing.app

# Sign the bundle as a whole: the engine in Frameworks first, then the app.
IDENTITY="${IR_CODESIGN_IDENTITY:--}"
SIGN=(codesign --force --sign "$IDENTITY")
if [ "$IDENTITY" != "-" ]; then
    SIGN+=(--options runtime --timestamp) # what notarization requires
fi
"${SIGN[@]}" "$STAGE/IronRing.app/Contents/Frameworks/UnityPlayer.dylib"
"${SIGN[@]}" "$STAGE/IronRing.app"
codesign --verify --strict --verbose=1 "$STAGE/IronRing.app"

zip_folder "$STAGE" "Dist/$NAME.zip"
if [ -n "${IR_NOTARY_PROFILE:-}" ]; then
    if [ "$IDENTITY" = "-" ]; then
        echo "Notarizing needs IR_CODESIGN_IDENTITY (a Developer ID Application certificate)." >&2
        exit 1
    fi
    echo "== Notarizing (a few minutes) =="
    xcrun notarytool submit "Dist/$NAME.zip" --keychain-profile "$IR_NOTARY_PROFILE" --wait
    xcrun stapler staple "$STAGE/IronRing.app"
    zip_folder "$STAGE" "Dist/$NAME.zip" # again, with the ticket stapled
fi

echo "== Ready: Dist/$NAME.zip ($(lipo -archs "$STAGE/IronRing.app/Contents/MacOS/IronRing")) =="
echo "   Unzip and open $NAME/IronRing.app (macOS 11 or later)."
