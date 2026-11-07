# Shared by the build-*.sh scripts at the top of the repository (docs/building.md).
# Sourced, not run.

# need <tool> <install hint>...: stops with the hint when a tool is missing.
need() {
    local tool="$1"
    shift
    if ! command -v "$tool" > /dev/null 2>&1; then
        echo "Missing $tool. $*" >&2
        exit 1
    fi
}

# build_jobs: parallel compile jobs, about 1 GB of memory (and swap) each.
build_jobs() {
    local cpus mem_mb
    if [ -n "${JOBS:-}" ]; then
        echo "$JOBS"
        return
    fi
    if [ "$(uname -s)" = Darwin ]; then
        cpus=$(sysctl -n hw.ncpu)
        mem_mb=$(($(sysctl -n hw.memsize) / 1048576))
    else
        cpus=$(nproc)
        mem_mb=$(awk '/MemTotal|SwapTotal/ {sum += $2} END {print int(sum / 1024)}' /proc/meminfo)
    fi
    local by_mem=$((mem_mb / 1024))
    [ "$by_mem" -lt 1 ] && by_mem=1
    [ "$by_mem" -lt "$cpus" ] && cpus=$by_mem
    echo "$cpus"
}

# project_version <build dir>: the version CMake configured (the top-level project()).
project_version() {
    sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$1/CMakeCache.txt"
}

# release_version <version>: the name archives use: major.minor, and the patch only when it
# isn't 0 (1.0.0 -> 1.0, 1.0.2 -> 1.0.2).
release_version() {
    case "$1" in
        *.*.0) echo "${1%.0}" ;;
        *) echo "$1" ;;
    esac
}

# source_version: the version in the top-level CMakeLists.txt, without configuring.
source_version() {
    sed -n 's/^    VERSION \([0-9][0-9.]*\).*/\1/p' CMakeLists.txt | head -1
}

# tar_folder <folder> <tar.gz>: one folder per archive, Unix permissions kept.
tar_folder() {
    local folder="$1" out
    mkdir -p "$(dirname "$2")"
    out="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"
    rm -f "$out"
    tar -czf "$out" -C "$(dirname "$folder")" "$(basename "$folder")"
}

# zip_folder <folder> <zip>: one folder per zip, Unix permissions kept.
zip_folder() {
    local folder="$1" out
    mkdir -p "$(dirname "$2")"
    out="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"
    rm -f "$out"
    if [ "$(uname -s)" = Darwin ]; then
        ditto -c -k --sequesterRsrc --keepParent "$folder" "$out" # keeps the bundle's signature intact
    elif command -v zip > /dev/null 2>&1; then
        (cd "$(dirname "$folder")" && zip -qry "$out" "$(basename "$folder")")
    else
        (cd "$(dirname "$folder")" && cmake -E tar cf "$out" --format=zip "$(basename "$folder")")
    fi
}

# release_notes <folder> <platform> <what to start>: what every release archive carries
# next to the game: the licence texts, a short README.txt and the itch.io app's launch
# manifest (.itch.toml, docs/building.md#itchio).
release_notes() {
    local folder="$1" platform="$2" start="$3"
    cp LICENSE "$folder/LICENSE.txt"
    cp THIRD_PARTY_NOTICES.md "$folder/THIRD_PARTY_NOTICES.md"
    sed -e "s|@VERSION@|$VERSION|g" -e "s|@START@|$start|g" \
        "Tools/Build/README-$platform.txt" "Tools/Build/README-common.txt" > "$folder/README.txt"
    sed -e "s|@START@|$start|g" Tools/Build/itch.toml.in > "$folder/.itch.toml"
}
