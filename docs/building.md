# Building release archives

One script per platform builds the game from a checkout and packs a release archive into `Dist/`. Each archive holds one folder, `IronRing-<version>-<platform>/`, with the player and its data, `README.txt` (how to start, controls, online play), `LICENSE.txt`, `THIRD_PARTY_NOTICES.md` and `.itch.toml` ([itch.io](#itchio)). The folder stays in `Dist/` next to the archive. `<version>` is `major.minor`, plus the patch when it isn't 0: 1.0.0 makes `IronRing-1.0-…`, 1.0.2 makes `IronRing-1.0.2-…`. The scripts use the normal presets, so a second run only rebuilds what changed. For day-to-day development, the presets in the [README](../README.md#building) are enough.

| Script | Runs on | Builds | Output |
| --- | --- | --- | --- |
| `./build-linux.sh` | Linux | the Linux player (x86-64, or arm64 on an arm64 machine) | `Dist/IronRing-<version>-linux-x64.tar.gz` |
| `./build-windows.sh` | Linux | the Windows x64 player, cross-compiled with MinGW-w64 | `Dist/IronRing-<version>-windows-x64.zip` |
| `build-windows.bat` (or `.\build-windows.ps1`) | Windows | the Windows x64 player, with Visual Studio (or MinGW: `-MinGW`) | `Dist\IronRing-<version>-windows-x64.zip` |
| `./build-macos.sh` | macOS | `IronRing.app`, one binary for Apple silicon and Intel | `Dist/IronRing-<version>-macos.zip` |
| `./build-server.sh` | Linux, macOS | the dedicated server and its data ([server.md](server.md)) | `Build/server/Server/Release/` |
| `Tools/Build/source-package.sh` | anywhere with git | the source package: the committed tree at HEAD | `Dist/IronRing-<version>-source.zip` |

Common options:
- `--test` (`-Test` on Windows) runs the tests before packing.
- `--run` (`-Run`) starts the game instead of packing. Extra arguments go to the game: `./build-linux.sh --run -screen-fullscreen 0`.
- `JOBS=n` sets the number of parallel compile jobs. By default it is one per CPU, capped at one per GB of memory and swap.

The CI workflow runs every script and keeps the archives as artifacts of the run, in the jobs *Release archives (Linux, Windows cross)*, *macOS* and *Windows (build-windows.ps1)*.

## Linux

```sh
sudo apt install build-essential cmake ninja-build git \
  libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxss-dev libxfixes-dev libxtst-dev \
  libwayland-dev libxkbcommon-dev libegl1-mesa-dev libgl1-mesa-dev libasound2-dev libpulse-dev libudev-dev libdbus-1-dev
./build-linux.sh
```

SDL is built from source and opens X11 or Wayland, and ALSA or PulseAudio, at run time. It needs only their headers to build. The archive is a `.tar.gz`, which keeps the executable bit.

**glibc.** A Linux build runs on the glibc it was built with, or a newer one. The script prints the version it needs. A build on Ubuntu 24.04 needs glibc 2.38 (Ubuntu 24.04, Debian 13, Fedora 39 or newer). For older distributions, build on the oldest one you want to support.

## Windows

**From Linux** (what the published zips use):

```sh
sudo apt install build-essential cmake ninja-build git g++-mingw-w64-x86-64-posix
./build-windows.sh
```

The script first builds a native `AssetPacker` in `Build/host-tools`, which packs the data, then the Windows player in `Build/mingw-release`. The MinGW runtime is linked statically, so the zip needs nothing but Windows ([ADR-0009](adr/0009-mingw-static-runtime.md)).

**On Windows:** install Visual Studio 2022 or 2026 with *Desktop development with C++* (it brings CMake), and Git. Then double-click `build-windows.bat`, or run it from a terminal:

```bat
build-windows.bat
build-windows.bat -Test
build-windows.bat -MinGW      :: MSYS2 UCRT64 or CLion's MinGW, with g++ and ninja on PATH
```

Windows SmartScreen warns about an unsigned download from the internet (*More info → Run anyway*). Signing the exe and dll with a code-signing certificate removes the warning, but the project has no certificate yet.

## macOS

```sh
xcode-select --install          # the compiler, once
brew install cmake ninja
./build-macos.sh
```

The result is `IronRing.app`, laid out like a Unity Mac player ([ADR-0027](adr/0027-macos-app-bundle.md)):

```text
IronRing.app/Contents/
├── Info.plist                  io.github.dkapusta.IronRing, macOS 11 or later
├── MacOS/IronRing              launcher
├── Frameworks/UnityPlayer.dylib engine
└── Resources/
    ├── IronRing.icns
    └── Data/                   what IronRing_Data holds on the other platforms
```

`ARCHS=arm64 ./build-macos.sh` builds only for Apple silicon, which takes half the time. Prefs, replays and screenshots go to `~/Library/Application Support/dkapusta/Iron Ring/`.

### Signing and notarization

Without a Developer ID, the script signs the app *ad hoc*. It runs on the Mac that built it. On another Mac, a downloaded copy is blocked the first time: open it, then allow it in *System Settings → Privacy & Security → Open Anyway* (on macOS 14 and earlier, Control-click the app and choose *Open*). Players who would rather not do that can remove the quarantine flag with `xattr -dr com.apple.quarantine IronRing.app`.

To publish a build that opens without warnings, you need an Apple Developer Program membership (99 USD a year):

1. In Xcode → Settings → Accounts, or on developer.apple.com, create a **Developer ID Application** certificate. It installs into the login keychain.
2. Save notarization credentials once, with an app-specific password from appleid.apple.com:
   ```sh
   xcrun notarytool store-credentials ironring --apple-id you@example.com --team-id TEAMID
   ```
3. Build, sign, notarize and staple:
   ```sh
   IR_CODESIGN_IDENTITY="Developer ID Application: Your Name (TEAMID)" IR_NOTARY_PROFILE=ironring ./build-macos.sh
   ```

The script signs with the hardened runtime and a timestamp, sends the zip to Apple and waits (usually a few minutes). It then staples the ticket to the app and packs the zip again. `spctl --assess -vv Dist/IronRing-*-macos/IronRing.app` should print *accepted, source=Notarized Developer ID*.

## itch.io

Every build folder carries an `.itch.toml`, so the itch.io app knows what to start (`IronRing.exe`, `IronRing.x86_64` or `IronRing.app`). Push the folders with [butler](https://itch.io/docs/butler/) rather than uploading archives: butler sends only what changed, and players of the itch.io app get updates as patches.

```sh
butler login                                    # once
./build-linux.sh && ./build-windows.sh          # on Linux; ./build-macos.sh on a Mac
Tools/Build/itch-push.sh <user>/<game>          # e.g. dkapusta/iron-ring
```

`itch-push.sh` pushes each folder in `Dist/` that exists for the checkout's version to its channel: `windows`, `linux` or `mac`. itch.io tags the platform from the channel name. Run it on the Mac too, to push the `mac` channel from there. `DRY_RUN=1` shows what it would push. `BUTLER_API_KEY` replaces `butler login`, for example in CI.

On the game's itch.io page:
- set *Kind of project* to *Downloadable*;
- set the pricing;
- after the first push, check that each upload shows its platform icon.

You can also upload the archives by hand for players who don't use the app. They can be the same files as the pushed folders.

## Versions

The version comes from `project(... VERSION)` in the top-level `CMakeLists.txt` (`0.<milestone>.<patch>` until 1.0, see the [ROADMAP](../ROADMAP.md) decisions log). Online play needs both players and the server on the same commit's game data, whatever the platform: a Mac and a Windows PC built from one commit can fight each other.
