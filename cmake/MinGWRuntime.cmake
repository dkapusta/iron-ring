# MinGW-w64 runtime linking (docs/adr/0009-mingw-static-runtime.md).
#
# Goal: binaries run straight from the build folder, with no MinGW bin folder
# on PATH. Preferred: link libgcc, libstdc++ and winpthread statically. Some
# toolchains ship a static libpthread.a that cannot be linked (CLion 2026.2's
# bundled GCC 15.2: its libpthread.a needs UCRT symbols while the toolchain
# links msvcrt), so AUTO probes what actually links and falls back to copying
# the remaining runtime DLLs next to every executable.
#
#   IR_MINGW_RUNTIME = AUTO | STATIC | STATIC_LIBSTDCXX | SHARED
#     STATIC            -static: no runtime DLLs at all
#     STATIC_LIBSTDCXX  libgcc + libstdc++ static; libwinpthread-1.dll copied
#     SHARED            all three runtime DLLs copied

if(NOT MINGW)
    function(ir_copy_mingw_runtime target)
    endfunction()
    return()
endif()

set(IR_MINGW_RUNTIME "AUTO" CACHE STRING "MinGW runtime linking: AUTO, STATIC, STATIC_LIBSTDCXX or SHARED")
set_property(CACHE IR_MINGW_RUNTIME PROPERTY STRINGS AUTO STATIC STATIC_LIBSTDCXX SHARED)

set(_ir_static_flags          -static -static-libgcc -static-libstdc++)
set(_ir_static_libstdcxx_flags -static-libgcc -static-libstdc++)

# Exercises exactly what failed in the wild: std::thread pulls winpthread's
# thread.o, snprintf pulls the CRT printf aliases.
set(_ir_mingw_probe [=[
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <thread>
int main() {
    std::mutex m;
    int value = 0;
    std::thread t([&] { std::lock_guard<std::mutex> lock(m); value = 42; });
    t.join();
    char text[16];
    std::snprintf(text, sizeof text, "%d", value);
    return std::filesystem::path(text).empty() ? 1 : 0;
}
]=])

include(CheckCXXSourceCompiles)
function(_ir_mingw_probe result)
    set(CMAKE_REQUIRED_LINK_OPTIONS ${ARGN})
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_source_compiles("${_ir_mingw_probe}" ${result})
endfunction()

set(_mode "${IR_MINGW_RUNTIME}")
if(_mode STREQUAL "AUTO")
    _ir_mingw_probe(IR_MINGW_LINKS_STATIC ${_ir_static_flags})
    if(IR_MINGW_LINKS_STATIC)
        set(_mode STATIC)
    else()
        _ir_mingw_probe(IR_MINGW_LINKS_STATIC_LIBSTDCXX ${_ir_static_libstdcxx_flags})
        if(IR_MINGW_LINKS_STATIC_LIBSTDCXX)
            set(_mode STATIC_LIBSTDCXX)
        else()
            set(_mode SHARED)
        endif()
        message(WARNING
            "MinGW: this toolchain cannot link winpthread statically (a known issue with some "
            "bundled toolchains, e.g. CLion 2026.2). Falling back to IR_MINGW_RUNTIME=${_mode}: the "
            "missing runtime DLLs are copied next to each executable, so builds still run from the "
            "build folder. A toolchain whose static libraries link cleanly (e.g. MSYS2 UCRT64) "
            "produces fully self-contained binaries.")
    endif()
endif()

if(_mode STREQUAL "STATIC")
    add_link_options(${_ir_static_flags})
    set(_dlls "")
elseif(_mode STREQUAL "STATIC_LIBSTDCXX")
    add_link_options(${_ir_static_libstdcxx_flags})
    set(_dlls libwinpthread-1.dll)
elseif(_mode STREQUAL "SHARED")
    set(_dlls libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
else()
    message(FATAL_ERROR "IR_MINGW_RUNTIME must be AUTO, STATIC, STATIC_LIBSTDCXX or SHARED (got '${IR_MINGW_RUNTIME}')")
endif()

# Locate DLLs: the compiler knows its own runtime directories; on Windows
# toolchains the DLLs usually sit next to the compiler instead.
get_filename_component(_compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(IR_MINGW_RUNTIME_DLLS "")
foreach(_dll IN LISTS _dlls)
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=${_dll}
                    OUTPUT_VARIABLE _path OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(NOT (IS_ABSOLUTE "${_path}" AND EXISTS "${_path}"))
        set(_path "${_compiler_dir}/${_dll}")
    endif()
    if(EXISTS "${_path}")
        list(APPEND IR_MINGW_RUNTIME_DLLS "${_path}")
    else()
        message(WARNING "MinGW: ${_dll} not found near ${CMAKE_CXX_COMPILER}; binaries will need it on PATH")
    endif()
endforeach()

message(STATUS "MinGW runtime: ${_mode}")
if(IR_MINGW_RUNTIME_DLLS)
    message(STATUS "MinGW runtime DLLs copied next to executables: ${IR_MINGW_RUNTIME_DLLS}")
endif()

# Copies the runtime DLLs this configuration needs next to an executable.
function(ir_copy_mingw_runtime target)
    if(NOT IR_MINGW_RUNTIME_DLLS)
        return()
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${IR_MINGW_RUNTIME_DLLS} "$<TARGET_FILE_DIR:${target}>"
        VERBATIM)
endfunction()
