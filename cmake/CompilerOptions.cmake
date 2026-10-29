# Project-wide compiler settings, applied per target through ir_configure_target().
# Third-party code fetched by Dependencies.cmake never receives these flags.

if(MSVC)
    add_compile_options(/utf-8)
    # Static CRT is not used: SDL and the engine share the default dynamic CRT,
    # which ships with Windows (UCRT) — nothing extra to copy next to the exe.
endif()

# MinGW-w64: link libgcc, libstdc++ and winpthread statically into every
# executable and DLL, so binaries run straight from the build folder without
# libgcc_s_seh-1.dll / libstdc++-6.dll / libwinpthread-1.dll on PATH.
# Declared before Dependencies.cmake so SDL and friends inherit it too.
# See docs/adr/0009-mingw-static-runtime.md.
option(IR_MINGW_STATIC_RUNTIME "MinGW: link the GCC runtime statically" ON)
if(MINGW AND IR_MINGW_STATIC_RUNTIME)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

function(ir_configure_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
        if(IR_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion
            -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual)
        if(IR_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
    set_target_properties(${target} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
endfunction()
