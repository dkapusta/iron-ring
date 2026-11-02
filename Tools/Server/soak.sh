#!/usr/bin/env bash
# Online soak test (M4.5): a real IronRingServer and two headless game
# clients meet in an open room and fight match after match through the
# relay, with simulated latency and loss. Fails unless at least MIN_MATCHES
# finish, the referee never sees a desync, and every saved online replay
# plays back identically.
#
#   Tools/Server/soak.sh [build dir]          default Build/release
#
# Environment: FRAMES (per client, at 60 a second; default 9000 = 2.5 min),
# NETSIM (default "40,10": 40 ms more latency, 10 % input loss), MIN_MATCHES
# (default 2), PORT (default 7737), SOAK_DIR (a folder for the logs and
# replays, kept; default a temporary one, removed unless KEEP=1 or it failed).
set -euo pipefail

BUILD="${1:-Build/release}"
FRAMES="${FRAMES:-9000}"
NETSIM="${NETSIM:-40,10}"
MIN_MATCHES="${MIN_MATCHES:-2}"
PORT="${PORT:-7737}"

PLAYER_DIR="$BUILD/Player/Release"
PLAYER="$(ls "$PLAYER_DIR"/IronRing.x86_64 "$PLAYER_DIR"/IronRing 2>/dev/null | head -1 || true)"
SERVER="$BUILD/Server/Release/IronRingServer"
DATA="$PLAYER_DIR/IronRing_Data"
for f in "$PLAYER" "$SERVER" "$DATA"; do
    if [ -z "$f" ] || [ ! -e "$f" ]; then
        echo "soak: missing $f (build the release preset first)" >&2
        exit 2
    fi
done
PLAYER="$(cd "$(dirname "$PLAYER")" && pwd)/$(basename "$PLAYER")"
SERVER="$(cd "$(dirname "$SERVER")" && pwd)/$(basename "$SERVER")"
DATA="$(cd "$DATA" && pwd)"

if [ -n "${SOAK_DIR:-}" ]; then
    WORK="$SOAK_DIR"
    KEEP=1
    rm -rf "$WORK" && mkdir -p "$WORK"
else
    WORK="$(mktemp -d)"
fi
cleanup() {
    kill "${PIDS[@]}" 2>/dev/null || true
    if [ "${KEEP:-0}" = 1 ]; then echo "soak: logs and replays in $WORK"; else rm -rf "$WORK"; fi
}
PIDS=()
trap cleanup EXIT

"$SERVER" --port "$PORT" --data "$DATA" --name "SOAK" > "$WORK/server.log" 2>&1 &
SERVER_PID=$!
PIDS+=("$SERVER_PID")
sleep 1

client() { # name: each client keeps its replays in a folder of its own
    mkdir -p "$WORK/$1"
    XDG_DATA_HOME="$WORK/$1" "$PLAYER" -batchmode -screen-fullscreen 0 -screen-width 640 -screen-height 360 \
        -frames "$FRAMES" -connect "127.0.0.1:$PORT" -online-name "$1" -autoplay -netsim "$NETSIM" \
        -logFile "$WORK/$1.log" > /dev/null 2>&1
}
echo "soak: two clients, $FRAMES frames each, netsim $NETSIM"
client BOT1 &
A=$!
PIDS+=("$A")
sleep 2
client BOT2 &
B=$!
PIDS+=("$B")
wait "$A" "$B"
kill -TERM "$SERVER_PID"
wait "$SERVER_PID" || true

fail=0
finished=$(grep -c "ended, finished" "$WORK/server.log" || true)
echo "soak: $finished matches finished"
grep "ended, " "$WORK/server.log" | sed 's/^/  /' || true
if grep -q "OUT OF SYNC" "$WORK/server.log"; then
    echo "soak: FAIL: the referee saw a desync" >&2
    fail=1
fi
if [ "$finished" -lt "$MIN_MATCHES" ]; then
    echo "soak: FAIL: fewer than $MIN_MATCHES matches finished" >&2
    fail=1
fi
for log in "$WORK"/BOT1.log "$WORK"/BOT2.log; do
    if grep -q "Online fight stopped" "$log"; then
        # Fine only when the other client ran out of frames first (the very end of the run).
        grep "Online fight stopped" "$log" | sed "s|^|  $(basename "$log"): |"
    fi
done

# Every finished online match was saved; each must play back to the same result.
replays=0
while IFS= read -r -d '' replay; do
    replays=$((replays + 1))
    if "$PLAYER" -batchmode -screen-fullscreen 0 -screen-width 640 -screen-height 360 -frames 20000 \
        -replay "$replay" -logFile "$WORK/replay.log" > /dev/null 2>&1 && grep -q "Replay verified" "$WORK/replay.log"; then
        :
    else
        echo "soak: FAIL: $(basename "$replay") did not play back the same" >&2
        grep -i "replay" "$WORK/replay.log" | sed 's/^/  /' >&2 || true
        fail=1
    fi
done < <(find "$WORK" -name '*.irr' -print0)
echo "soak: $replays online replays checked"
if [ "$replays" -lt "$MIN_MATCHES" ]; then
    echo "soak: FAIL: expected a replay for every finished match" >&2
    fail=1
fi

if [ "$fail" = 0 ]; then echo "soak: OK"; else KEEP=1; fi
exit "$fail"
