# ADR-0027: macOS as an application bundle laid out like a Unity Mac player

- **Status:** Accepted
- **Date:** 2026-11-07

## Context

Windows and Linux ship a flat player folder (`IronRing.exe`, `UnityPlayer.dll`, `IronRing_Data/`, [ADR-0003](0003-unity-style-player-layout.md)). macOS was planned for M5. On a Mac, a game is expected to be one `.app` bundle: Finder shows it as a single icon, it carries its own name, version and icon in `Info.plist`, and only a bundle can be signed and notarized as a whole. The owner asked for macOS support and a build script.

## Decision

- **The bundle mirrors a Unity Mac player:**
  - `IronRing.app/Contents/MacOS/IronRing`: the launcher, unchanged;
  - `Contents/Frameworks/UnityPlayer.dylib`: the engine, found through the launcher's rpath `@executable_path/../Frameworks`;
  - `Contents/Resources/Data/`: the same files as `IronRing_Data/`;
  - `Contents/Resources/IronRing.icns`: made by `Tools/Art` from the same 32 × 32 icon as the Windows `.ico`;
  - `Contents/Info.plist`: from `cmake/templates/Info.plist.in`. Its bundle id is `IR_APP_ID`, and its versions are the project's.
- **Finding the data.** Inside a bundle, `SDL_GetBasePath()` is `Contents/Resources/`, so on macOS the data folder's name is `Data` (`IR_DATA_DIR_NAME`). The engine code doesn't change. The server keeps its flat layout and `IronRing_Data` on every platform.
- **One universal binary** for Apple silicon and Intel (`CMAKE_OSX_ARCHITECTURES=arm64;x86_64`, the `macos` preset), from macOS 11.
- **Signing:** ad hoc by default. With a Developer ID, `build-macos.sh` signs with the hardened runtime, then notarizes and staples ([building.md](../building.md#signing-and-notarization)). The project has no Developer ID yet.
- **CI:** a `macos-15` job builds with the script, runs the tests, checks the bundle, runs the game in batch mode from the zip, and runs the online soak.

## Consequences

- Online play is cross-platform: the server and the game data don't depend on the platform.
- Until the owner has a Developer ID, players on other Macs approve the app once in *Privacy & Security*.
- `IRONRING_USER_FOLDER` replaces the per-user folder. The soak test uses it on every platform, because macOS ignores `XDG_DATA_HOME`.
