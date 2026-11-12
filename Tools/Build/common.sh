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

# release_notes <folder> <build dir>: the licence texts every release zip carries:
# Iron Ring's own, the notices, and the full third-party texts that the configure
# step collected into <build dir>/Licenses (cmake/Licenses.cmake).
release_notes() {
    if [ ! -d "$2/Licenses" ]; then
        echo "Missing $2/Licenses: the configure step collects the third-party licence texts." >&2
        exit 1
    fi
    cp LICENSE "$1/LICENSE.txt"
    cp THIRD_PARTY_NOTICES.md "$1/THIRD_PARTY_NOTICES.md"
    rm -rf "$1/Licenses"
    cp -R "$2/Licenses" "$1/Licenses"
}
