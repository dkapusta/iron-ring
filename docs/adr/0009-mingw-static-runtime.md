# ADR-0009: MinGW runtime — static when possible, self-contained always

- **Status:** Accepted (revised 2026-10-29)
- **Date:** 2026-10-28

## Context

The owner develops in CLion on Windows with its bundled MinGW-w64 toolchain. MinGW GCC binaries depend by default on `libgcc_s_seh-1.dll`, `libstdc++-6.dll` and `libwinpthread-1.dll`, which are not on `PATH` outside the toolchain's shell, so the player, tests and AssetPacker failed to start from the build folder.

**First attempt** (this ADR's original version): link everything with `-static -static-libgcc -static-libstdc++`. It worked in CI (MSYS2 UCRT64, GCC 15) and in a Linux cross build, but **failed to link with CLion 2026.2.1's bundled GCC 15.2**:

```text
libpthread.a(libwinpthread_la-thread.o): undefined reference to `__intrinsic_setjmpex'
libmsvcrt.a(...snprintf_alias.o): undefined reference to `__ms_vsnprintf'
```

Investigation: that toolchain links against **msvcrt**, but its static `libpthread.a` references `__intrinsic_setjmpex`, which exists only in the **UCRT** import libraries. The static winpthread was evidently built for a different C runtime than the toolchain uses; the second error is a knock-on effect of pulling in that object. Public GCC 15.2/16.1 msvcrt builds from the same family (niXman mingw-builds `15.2.0-rt_v13-rev0/rev1`, `16.1.0-rt_v14-rev0`) don't have this reference, so it's specific to the bundled toolchain and can't be fixed from the project side. Its import library (dynamic winpthread) is fine.

## Decision

`cmake/MinGWRuntime.cmake` chooses how the MinGW runtime is linked, via the cache variable `IR_MINGW_RUNTIME`:

| Mode | Link flags | Runtime DLLs copied next to executables |
| --- | --- | --- |
| `STATIC` | `-static -static-libgcc -static-libstdc++` | none |
| `STATIC_LIBSTDCXX` | `-static-libgcc -static-libstdc++` | `libwinpthread-1.dll` |
| `SHARED` | — | `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` |
| **`AUTO`** (default) | first of the above that **links a probe program** at configure time | as chosen |

- The probe uses `std::thread`, `std::mutex`, `snprintf` and `std::filesystem` — exactly what broke in the wild.
- When `AUTO` falls back, CMake prints a warning naming the reason and the chosen mode.
- DLLs are located with `<compiler> -print-file-name=<dll>`, falling back to the compiler's directory (where Windows toolchains keep them), and copied by a `POST_BUILD` step on every executable (`ir_configure_target` → `ir_copy_mingw_runtime`). The engine DLL lives next to the launcher and uses the same copy.
- The link options are set with `add_link_options()` before dependencies are declared, so they apply to every target.

Because the launcher and `UnityPlayer.dll` may each carry a private copy of the C++ runtime (in `STATIC` mode), the boundary between them stays **C-only**: `PlayerMain`/`UnityMain` take and return plain C types, and the engine catches every exception before returning (`guardedRun` in `PlayerMain.cpp`).

## Verification

- Linux cross build (MinGW-w64 GCC 13) with a stub `libpthread.a` that mimics CLion's: forced `STATIC` fails to link; `AUTO` warns, picks `STATIC_LIBSTDCXX`, builds, and copies `libwinpthread-1.dll` into `Player/`, `Tests/` and `Tools/`. Under Wine the tests (21/21), AssetPacker and the batch-mode player run. Removing the copied DLL makes the test executable fail to load, which proves the copy is what makes it work.
- The same cross build with the normal toolchain: `AUTO` picks `STATIC`; the binaries import only Windows system DLLs.
- CI: the MSYS2 UCRT64 job requires `STATIC` and checks that no binary imports a MinGW runtime DLL. A second job uses a niXman GCC 15.2 msvcrt toolchain (the closest public relative of CLion's). Both run the tests and the player from PowerShell with no MinGW folder on `PATH`.

## Consequences

- Build-folder binaries run directly from CLion, Explorer or `cmd.exe` on every MinGW toolchain we know of. Toolchains with healthy static libraries (MSYS2 UCRT64, mingw-builds, Ubuntu's cross compiler) produce fully self-contained binaries.
- With CLion's bundled toolchain the player folder contains `libwinpthread-1.dll`. That's fine for development. For distribution, prefer an MSVC or MSYS2 UCRT64 build, or ship the DLL; it is MIT-licensed.
- `STATIC` binaries grow by roughly 1–2 MB each. That doesn't matter here.
- Licensing: static libgcc/libstdc++ is permitted by the GCC Runtime Library Exception. winpthreads' MIT-style notice is listed in `THIRD_PARTY_NOTICES.md`.
- MSVC is unaffected: it uses the system UCRT plus `vcruntime140.dll`.
- The copy runs when an executable is (re)linked; deleting a copied DLL by hand needs a relink (or a clean build) to restore it.

## Alternatives considered

- **Always static**: breaks on CLion's bundled toolchain (the original version of this ADR).
- **Link `-lucrt` to satisfy `__intrinsic_setjmpex`**: mixes two C runtimes in one process; undefined behaviour waiting to happen.
- **Add the MinGW `bin` folder to PATH in CLion run configurations**: per-machine setup that doesn't help double-clicking or testers.
- **Ask everyone to use MSYS2**: still documented as the best-quality option, but the default CLion setup must just work.
