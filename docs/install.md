# Installing

The player folder (`Build/<preset>/Player/<Config>/`) is self-contained and relocatable: the launcher finds the engine next to itself, and the engine finds `IronRing_Data/` the same way. Running it from the build folder, or copying the folder anywhere, needs nothing else. `cmake --install` copies it and, on Linux, adds a desktop entry and an icon.

```sh
cmake --build --preset release
cmake --install Build/release --prefix ~/.local            # per user
sudo cmake --install Build/release --prefix /usr/local     # system-wide
DESTDIR=pkg cmake --install Build/release --prefix /usr    # staging for a package
```

| Platform | Installed files |
| --- | --- |
| Linux | `<prefix>/lib/IronRing/` (the player folder), `<prefix>/share/applications/io.github.dkapusta.IronRing.desktop`, `<prefix>/share/icons/hicolor/256x256/apps/io.github.dkapusta.IronRing.png` |
| Windows | `<prefix>/IronRing/` (the player folder; with MinGW, including any runtime DLLs) |
| macOS | `<prefix>/IronRing.app` (`cmake --install Build/macos --prefix /Applications`) |

- The desktop entry is written at install time, so its `Exec` and `Path` follow `--prefix`; with `DESTDIR` they point at the final prefix while the files land in the staging root.
- The app id (`IR_APP_ID`, default `io.github.<company>.<product>`) is also given to SDL as the app identifier, so the window's Wayland app id / X11 class match the desktop entry and desktops show the right icon.
- `IR_INSTALL_DIR` changes the folder's location under the prefix (default `lib/IronRing` on Linux, `IronRing` on Windows, the prefix itself on macOS).
- `~/.local/share/applications` may need `update-desktop-database ~/.local/share/applications` before menus see the entry. Some desktops also need `gtk-update-icon-cache` for the icon.

Release zips for every platform come from the build scripts ([building.md](building.md)). Every install also gets a `Licenses/` folder beside the player (on Linux and Windows) with Iron Ring's licence, the third-party notices and their full licence texts. Installers and store packages are not planned (ROADMAP).
