# The full licence texts of the third-party code linked into the player
# (THIRD_PARTY_NOTICES.md), collected into ${PROJECT_BINARY_DIR}/Licenses/ at
# configure time. The build-*.sh scripts and build-windows.ps1 put that folder in
# every release zip; `cmake --install` puts it next to the player.
#
# stb_vorbis and dr_flac (inside SDL_mixer) are used under their public-domain
# (Unlicense) option, which asks for no notice.

set(IR_LICENSES_DIR "${PROJECT_BINARY_DIR}/Licenses")
file(REMOVE_RECURSE "${IR_LICENSES_DIR}")

# ir_collect_licence(<name> <source dir> <candidate file>...): the first candidate
# that exists becomes Licenses/<name>.txt. A missing one stops the configure, so a
# release can't silently ship without it.
function(ir_collect_licence name dir)
    foreach(candidate IN LISTS ARGN)
        if(EXISTS "${dir}/${candidate}")
            configure_file("${dir}/${candidate}" "${IR_LICENSES_DIR}/${name}.txt" COPYONLY)
            return()
        endif()
    endforeach()
    message(FATAL_ERROR "Licence text of ${name} not found in ${dir} (looked for: ${ARGN})")
endfunction()

ir_collect_licence(SDL           "${sdl3_SOURCE_DIR}"          LICENSE.txt LICENSE)
ir_collect_licence(SDL_mixer     "${sdl3_mixer_SOURCE_DIR}"    LICENSE.txt LICENSE)
ir_collect_licence(miniz         "${miniz_SOURCE_DIR}"         LICENSE LICENSE.txt)
ir_collect_licence(nlohmann-json "${nlohmann_json_SOURCE_DIR}" LICENSE.MIT LICENSE)
ir_collect_licence(ENet          "${enet_SOURCE_DIR}"          LICENSE LICENSE.txt)

if(MINGW)
    # The MinGW-w64 runtime (CRT startup, libmingwex) is statically linked into every
    # MinGW binary, and winpthreads into most; their texts aren't in a fixed place
    # across toolchains (MSYS2, Debian, CLion's bundle), so they are kept here.
    ir_collect_licence(MinGW-w64-runtime "${PROJECT_SOURCE_DIR}/cmake/licenses" MinGW-w64-runtime.txt)
    ir_collect_licence(winpthreads       "${PROJECT_SOURCE_DIR}/cmake/licenses" winpthreads.txt)
endif()
