# Project-wide compiler settings, applied per target through ir_configure_target().
# Third-party code fetched by Dependencies.cmake never receives these flags.

if(MSVC)
    add_compile_options(/utf-8)
    # Static CRT is not used: SDL and the engine share the default dynamic CRT,
    # which ships with Windows (UCRT) — nothing extra to copy next to the exe.
endif()

# MinGW-w64 runtime: static where the toolchain allows it, otherwise the
# needed runtime DLLs are copied next to executables. Must run before
# Dependencies.cmake so add_link_options() also reaches SDL and friends.
include(MinGWRuntime)

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
    get_target_property(_type ${target} TYPE)
    if(_type STREQUAL "EXECUTABLE")
        ir_copy_mingw_runtime(${target})
    endif()
endfunction()
