#!/usr/bin/env bash
# Pushes the builds in Dist/ to itch.io with butler (docs/building.md#itchio):
#   Dist/IronRing-<version>-windows-x64  -> channel windows
#   Dist/IronRing-<version>-linux-x64    -> channel linux
#   Dist/IronRing-<version>-macos        -> channel mac
# Each folder is made by its build script. Missing ones are skipped.
#
#   Tools/Build/itch-push.sh <user>/<game> [version]     version: default the checkout's
#
# Needs butler (https://itch.io/docs/butler/) logged in once with `butler login`,
# or BUTLER_API_KEY set. DRY_RUN=1 shows what would be pushed.
set -euo pipefail
cd "$(dirname "$0")/../.."
source Tools/Build/common.sh

TARGET="${1:-}"
if [ -z "$TARGET" ] || [ "$TARGET" = -h ] || [ "$TARGET" = --help ]; then
    sed -n '2,12p' "$0"
    exit 2
fi
VERSION="${2:-$(release_version "$(source_version)")}"
[ "${DRY_RUN:-0}" = 1 ] || need butler "Install it from https://itch.io/docs/butler/installing.html"

pushed=0
for pair in windows-x64:windows linux-x64:linux macos:mac; do
    folder="Dist/IronRing-$VERSION-${pair%%:*}"
    channel="${pair##*:}"
    if [ ! -d "$folder" ]; then
        echo "-- $channel: no $folder, skipped"
        continue
    fi
    echo "== $folder -> $TARGET:$channel ($VERSION) =="
    if [ "${DRY_RUN:-0}" = 1 ]; then
        echo "   butler push $folder $TARGET:$channel --userversion $VERSION"
    else
        butler push "$folder" "$TARGET:$channel" --userversion "$VERSION"
    fi
    pushed=$((pushed + 1))
done
if [ "$pushed" = 0 ]; then
    echo "Nothing to push: run the build scripts first (docs/building.md)." >&2
    exit 1
fi
