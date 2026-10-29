# Third-party dependencies, fetched at configure time with FetchContent.
# Versions are pinned to release tags; bump them deliberately and record the
# change in docs/adr/0002-dependencies-via-fetchcontent.md.
#
# Offline / pre-downloaded builds: point FETCHCONTENT_SOURCE_DIR_<NAME> at a
# local checkout (e.g. -DFETCHCONTENT_SOURCE_DIR_SDL3=/src/SDL), or set
# FETCHCONTENT_FULLY_DISCONNECTED=ON after a first successful configure.

# Build trees configured by older presets cached a Build/_deps shared by every
# preset; FetchContent also keeps the dependencies' build trees there, so
# configurations overwrote each other's objects. Move them to this build tree.
if(FETCHCONTENT_BASE_DIR STREQUAL "${CMAKE_SOURCE_DIR}/Build/_deps")
    message(STATUS "Dependencies move from the shared Build/_deps to ${CMAKE_BINARY_DIR}/_deps")
    set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps" CACHE PATH "FetchContent base directory" FORCE)
endif()

include(FetchContent)

# Let option() in older dependency scripts honour the variables set below.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(IR_SDL3_TAG        "release-3.4.18" CACHE STRING "SDL3 git tag")
set(IR_SDL3_MIXER_TAG  "release-3.2.4"  CACHE STRING "SDL3_mixer git tag")
set(IR_MINIZ_TAG       "3.1.2"          CACHE STRING "miniz git tag")
set(IR_JSON_TAG        "v3.12.0"        CACHE STRING "nlohmann/json git tag")
set(IR_DOCTEST_TAG     "v2.5.3"         CACHE STRING "doctest git tag")

# Helper: dependencies are always built as static libraries with their own
# tests, examples and install rules disabled.
macro(ir_quiet_static)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_TESTING OFF)
endmacro()

# --- nlohmann/json (header-only): settings, pipeline manifests, content data
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        ${IR_JSON_TAG}
    GIT_SHALLOW    ON
    SYSTEM)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install    OFF CACHE INTERNAL "")

# --- miniz: ZIP reader/writer behind the .assets / .resource archives
FetchContent_Declare(miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG        ${IR_MINIZ_TAG}
    GIT_SHALLOW    ON
    SYSTEM)
set(BUILD_EXAMPLES   OFF CACHE INTERNAL "")
set(BUILD_TESTS      OFF CACHE INTERNAL "")
set(BUILD_FUZZERS    OFF CACHE INTERNAL "")
set(BUILD_NO_STDIO   OFF CACHE INTERNAL "")
set(INSTALL_PROJECT  OFF CACHE INTERNAL "")

ir_quiet_static()
FetchContent_MakeAvailable(nlohmann_json miniz)

if(IR_BUILD_PLAYER)
    # --- SDL3: window, renderer, input, audio device, PNG decoding
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        ${IR_SDL3_TAG}
        GIT_SHALLOW    ON
        SYSTEM)
    set(SDL_SHARED        OFF CACHE INTERNAL "")
    set(SDL_STATIC        ON  CACHE INTERNAL "")
    set(SDL_TEST_LIBRARY  OFF CACHE INTERNAL "")
    set(SDL_TESTS         OFF CACHE INTERNAL "")
    set(SDL_EXAMPLES      OFF CACHE INTERNAL "")
    set(SDL_INSTALL       OFF CACHE INTERNAL "")
    set(SDL_UNINSTALL     OFF CACHE INTERNAL "")

    # --- SDL3_mixer: only self-contained decoders (no system codec libraries)
    FetchContent_Declare(SDL3_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG        ${IR_SDL3_MIXER_TAG}
        GIT_SHALLOW    ON
        SYSTEM)
    set(SDLMIXER_VENDORED          OFF CACHE INTERNAL "")
    set(SDLMIXER_INSTALL           OFF CACHE INTERNAL "")
    set(SDLMIXER_TESTS             OFF CACHE INTERNAL "")
    set(SDLMIXER_EXAMPLES          OFF CACHE INTERNAL "")
    set(SDLMIXER_SAMPLES           OFF CACHE INTERNAL "")
    set(SDLMIXER_STRICT            OFF CACHE INTERNAL "")
    set(SDLMIXER_DEPS_SHARED       OFF CACHE INTERNAL "")
    set(SDLMIXER_WAVE              ON  CACHE INTERNAL "")
    set(SDLMIXER_VORBIS_STB        ON  CACHE INTERNAL "")
    set(SDLMIXER_FLAC              ON  CACHE INTERNAL "")
    set(SDLMIXER_FLAC_DRFLAC       ON  CACHE INTERNAL "")
    set(SDLMIXER_FLAC_LIBFLAC      OFF CACHE INTERNAL "")
    set(SDLMIXER_MP3               OFF CACHE INTERNAL "")
    set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE INTERNAL "")
    set(SDLMIXER_VORBIS_TREMOR     OFF CACHE INTERNAL "")
    set(SDLMIXER_OPUS              OFF CACHE INTERNAL "")
    set(SDLMIXER_GME               OFF CACHE INTERNAL "")
    set(SDLMIXER_MOD               OFF CACHE INTERNAL "")
    set(SDLMIXER_MIDI              OFF CACHE INTERNAL "")
    set(SDLMIXER_WAVPACK           OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(SDL3 SDL3_mixer)
endif()

if(IR_BUILD_TESTS)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        ${IR_DOCTEST_TAG}
        GIT_SHALLOW    ON
        SYSTEM)
    set(DOCTEST_NO_INSTALL ON CACHE INTERNAL "")
    FetchContent_MakeAvailable(doctest)
endif()
