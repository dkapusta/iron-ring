# ADR-0009: Link the MinGW runtime statically

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The owner develops in CLion on Windows with its MinGW-w64 toolchain. Binaries built by MinGW GCC depend by default on three runtime DLLs — `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` — which are not on `PATH` outside the toolchain's shell. Running the player, the tests or AssetPacker from the build folder therefore failed until the DLLs were copied by hand. The same problem would hit anyone receiving a MinGW-built player folder.

## Decision

When `MINGW` is true and `IR_MINGW_STATIC_RUNTIME` is `ON` (default), `cmake/CompilerOptions.cmake` adds

```cmake
add_link_options(-static -static-libgcc -static-libstdc++)
```

before dependencies are declared, so every executable and DLL in the tree — launcher, `UnityPlayer.dll`, `AssetPacker.exe`, `IronRingTests.exe`, and SDL's own link steps — embeds libgcc, libstdc++ and winpthread. `-static` is what pulls in `libwinpthread.a`; the two explicit flags document intent and cover toolchains where `-static` alone behaves differently.

Because the launcher and `UnityPlayer.dll` now each carry a private copy of the C++ runtime, the boundary between them stays **C-only**: `PlayerMain`/`UnityMain` take and return plain C types, and the engine catches every exception before returning (`guardedRun` in `PlayerMain.cpp`). The launcher contains no C++ library code. This rule already existed for the Unity-style layout; it is now load-bearing.

Verified by cross-compiling with MinGW-w64 GCC 13 (posix threads) and inspecting imports: the player, engine, tests and packer import only Windows system DLLs (`KERNEL32`, `USER32`, `GDI32`, `msvcrt`/UCRT, …). CI builds and runs the MinGW configuration on a Windows runner.

## Consequences

- Build-folder binaries run directly from CLion, Explorer or a plain `cmd.exe`; distribution needs no extra DLLs.
- Binaries grow by roughly 1–2 MB each. Irrelevant for this project.
- Two copies of libstdc++ (exe and DLL) mean no `std::` objects, allocations or exceptions may cross the boundary. Enforced by the narrow `Player.h` API; called out in AGENTS.md.
- Licensing: static libgcc/libstdc++ is permitted for any program by the GCC Runtime Library Exception. winpthreads is MIT-style licensed, so its copyright notice must accompany MinGW-built releases — add it to the third-party notices with SDL's (M5).
- MSVC is unaffected: it uses the system UCRT and the VC++ redistributable `vcruntime140.dll`, which ships with Windows 10/11 in practice; a static-CRT option for MSVC can be added later if needed.
- Opt out with `-DIR_MINGW_STATIC_RUNTIME=OFF` (e.g. to use a sanitizer runtime).

## Alternatives considered

- **Copy the three DLLs next to the binaries after build** (`$<TARGET_RUNTIME_DLLS>` cannot see them; a custom copy step from the compiler's `bin/`): works, but spreads toolchain files through the player folder and breaks if the compiler path changes.
- **Add the MinGW `bin` folder to PATH in CLion run configurations**: per-machine setup, and doesn't help testers.
- **Static libstdc++ only, dynamic libgcc**: still needs `libgcc_s_seh-1.dll` and `libwinpthread-1.dll`.
