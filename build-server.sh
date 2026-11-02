#!/usr/bin/env bash
# Builds the Iron Ring dedicated server and packs the game data next to it,
# from this checkout (docs/server.md). Linux; needs cmake, ninja and g++.
#
#   ./build-server.sh            build into Build/server/Server/Release/
#   ./build-server.sh --run      build, then run it (extra arguments go to the server)
#   ./build-server.sh --run --name "MY RING" --motd "FRIDAY NIGHT FIGHTS"
#
# Re-run after every `git pull`: only what changed is rebuilt. Players need a
# game built from the same commit.
set -euo pipefail

cd "$(dirname "$0")"
BUILD=Build/server
OUT="$BUILD/Server/Release"

RUN=0
if [ "${1:-}" = "--run" ]; then
    RUN=1
    shift
fi

for tool in cmake ninja c++; do
    if ! command -v "$tool" > /dev/null; then
        echo "Missing $tool. On Ubuntu: sudo apt install -y build-essential cmake ninja-build git" >&2
        exit 1
    fi
done

# The compiler needs about 1 GB per parallel job; small servers build one file at a time.
jobs=$(nproc)
mem_mb=$(awk '/MemTotal|SwapTotal/ {sum += $2} END {print int(sum / 1024)}' /proc/meminfo)
if [ "$mem_mb" -lt 1500 ]; then
    jobs=1
    echo "Only ${mem_mb} MB of memory and swap: building one file at a time." >&2
    echo "If the compiler gets killed, add swap (see docs/server.md)." >&2
elif [ $((mem_mb / 1024)) -lt "$jobs" ]; then
    jobs=$((mem_mb / 1024))
fi

echo "== Building the server at $(git rev-parse --short HEAD 2>/dev/null || echo '?') ($jobs jobs) =="
cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release -DIR_BUILD_PLAYER=OFF -DIR_BUILD_TESTS=OFF > /dev/null
cmake --build "$BUILD" --target IronRingServer AssetPacker -j "$jobs"

echo "== Packing the game data =="
rm -rf "$OUT/IronRing_Data"
"$BUILD/Tools/Release/AssetPacker" --project . --pipeline ProjectSettings/AssetPipeline.json \
    --output "$OUT/IronRing_Data" > /dev/null
[ -f "$OUT/server.cfg" ] || cp Source/Server/server.cfg "$OUT/server.cfg" # kept across rebuilds

echo "== Ready: $("$OUT/IronRingServer" --version) =="
echo "   $OUT/IronRingServer     settings: $OUT/server.cfg"

if [ "$RUN" = 1 ]; then
    exec "$OUT/IronRingServer" "$@"
fi
echo "   Run it with: $OUT/IronRingServer   (UDP 7727 must be open; Ctrl+C stops it)"
