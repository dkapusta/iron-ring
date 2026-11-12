# Third-party notices

Iron Ring itself is licensed under the [zlib License](LICENSE). It builds on the following third-party software, fetched at configure time (`cmake/Dependencies.cmake`) or supplied by the toolchain. Their licences apply to the respective components; binary releases must include this file.

| Component | Version | Licence | Used in | Source |
| --- | --- | --- | --- | --- |
| SDL | 3.4.18 | zlib | player (statically linked into `UnityPlayer`) | https://github.com/libsdl-org/SDL |
| SDL_mixer | 3.2.4 | zlib | player (statically linked into `UnityPlayer`) | https://github.com/libsdl-org/SDL_mixer |
| stb_vorbis (via SDL_mixer) | bundled | MIT or public domain (Unlicense) | player | https://github.com/nothings/stb |
| dr_flac (via SDL_mixer) | bundled | MIT-0 or public domain (Unlicense) | player | https://github.com/mackron/dr_libs |
| miniz | 3.1.2 | MIT | player, AssetPacker | https://github.com/richgel999/miniz |
| nlohmann/json | 3.12.0 | MIT | player, AssetPacker | https://github.com/nlohmann/json |
| ENet | 1.3.18 | MIT | IronRingServer, online play | https://github.com/lsalzman/enet |
| doctest | 2.5.3 | MIT | unit tests only (not shipped) | https://github.com/doctest/doctest |

**Tools whose output ships** (not linked or distributed with the game):

| Component | Version | Licence | Used for |
| --- | --- | --- | --- |
| eSpeak NG (via the `espeakng-loader` wheel) | 1.52 / wheel 0.2.4 | GPL-3.0-or-later | synthesising the announcer lines in `Tools/Audio/chiptune/voice.py`; only the rendered WAVs (`Assets/Resources/Audio/Voice/`) ship. Program output is not covered by the GPL ([ADR-0014](docs/adr/0014-venue-themes-and-announcer.md)). |

**Toolchain runtimes** (only in Windows builds made with MinGW-w64, see [ADR-0009](docs/adr/0009-mingw-static-runtime.md)):

| Component | Licence | Notes |
| --- | --- | --- |
| libgcc, libstdc++ | GPL-3.0 with the GCC Runtime Library Exception | The exception permits static or dynamic linking into programs under any licence |
| winpthreads | MIT, with some files under BSD-3-Clause | Statically linked, or shipped as `libwinpthread-1.dll` |
| mingw-w64 runtime (CRT startup, `libmingwex`) | Zope Public License 2.1 plus component notices | Statically linked into every MinGW binary; ship the toolchain's `COPYING.MinGW-w64-runtime.txt` with MinGW-built releases |

MSVC builds use the Microsoft C/C++ runtime that ships with Windows (UCRT) and the Visual C++ Redistributable (`vcruntime140.dll`).

The full licence texts are distributed with each component's sources (under `Build/_deps/<name>-src/` after configuring) and must be bundled with binary releases. The configure step copies them into `Build/<preset>/Licenses/` (`cmake/Licenses.cmake`, which stops the configure if one is missing), and every release zip carries that folder next to this file and Iron Ring's `LICENSE.txt`; `cmake --install` puts it beside the player. The MinGW-w64 runtime and winpthreads texts are kept in `cmake/licenses/`, because toolchains store them in different places. stb_vorbis and dr_flac are used under their public-domain (Unlicense) option.
