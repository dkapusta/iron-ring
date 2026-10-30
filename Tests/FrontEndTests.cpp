#include "Core/Menu.h"
#include "Core/PlayerPrefs.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

using namespace iron;

TEST_CASE("menu focus skips disabled items and wraps") {
    Menu menu;
    menu.add(MenuItem::action("arcade", "ARCADE", false));
    menu.add(MenuItem::action("versus", "VERSUS"));
    menu.add(MenuItem::action("training", "TRAINING"));
    menu.add(MenuItem::action("options", "OPTIONS"));
    CHECK(menu.focused().id == "versus"); // a disabled first item never holds the focus
    CHECK(menu.move(-1));
    CHECK(menu.focused().id == "options"); // wraps past the disabled item
    CHECK(menu.move(1));
    CHECK(menu.focused().id == "versus");
    menu.focusOn("arcade");
    CHECK(menu.focused().id == "versus"); // can't focus a disabled item
    menu.focusOn("training");
    CHECK(menu.focused().id == "training");
    CHECK(menu.find("nothing") == nullptr);
}

TEST_CASE("menu settings step through their choices") {
    Menu menu;
    menu.add(MenuItem::setting("vsync", "VSYNC", {"OFF", "ON"}, 1));
    menu.add(MenuItem::level("music", "MUSIC", 10, 10));
    CHECK(menu.adjust(1));
    CHECK(menu.focused().value() == "OFF"); // toggles wrap
    menu.move(1);
    CHECK_FALSE(menu.adjust(1));           // levels stop at the ends
    CHECK(menu.adjust(-3));
    CHECK(menu.focused().value() == "7");
    CHECK(levelValue(menu.focused().choice, 10) == doctest::Approx(0.7f));
    CHECK(levelIndex(0.25f, 10) == 3);     // rounds to the nearest step
    CHECK(levelIndex(1.5f, 10) == 10);
}

TEST_CASE("grid cursor wraps by row and column and skips missing cells") {
    // 3 columns, 5 cells:  0 1 2
    //                      3 4
    CHECK(gridMove(0, 1, 0, 3, 5) == 1);
    CHECK(gridMove(2, 1, 0, 3, 5) == 0);
    CHECK(gridMove(4, 1, 0, 3, 5) == 3);  // the short row wraps on itself
    CHECK(gridMove(1, 0, 1, 3, 5) == 4);
    CHECK(gridMove(4, 0, 1, 3, 5) == 1);
    CHECK(gridMove(2, 0, 1, 3, 5) == 2);  // no cell below: stays in its column
    CHECK(gridMove(0, -1, 0, 3, 5) == 2);
}

TEST_CASE("player prefs round-trip and sit between boot.config and the command line") {
    const auto folder = std::filesystem::temp_directory_path() / "IronRingPrefsTest";
    std::filesystem::remove_all(folder);
    PlayerPrefs prefs(folder / "prefs.cfg");
    CHECK(prefs.load().empty()); // missing file: empty store

    PlayerSettings chosen;
    chosen.musicVolume = 0.4f;
    chosen.vsync = false;
    chosen.windowMode = WindowMode::Windowed;
    prefs.storeSettings(chosen);
    prefs.set("last-stage", "the_crown");
    prefs.set("bad", "a#b\nc");
    REQUIRE(prefs.save());

    PlayerPrefs reloaded(folder / "prefs.cfg");
    CHECK(reloaded.load().empty());
    CHECK(reloaded.get("last-stage") == "the_crown");
    CHECK(reloaded.get("bad") == "abc"); // comments and newlines can't leak into the file
    CHECK(reloaded.get("music-volume") == "0.40");

    PlayerSettings s;
    (void)applyBootConfig(s, parseKeyValueText("music-volume=1\nvsync=1\nwindow-mode=fullscreen\n"));
    CHECK(reloaded.applyTo(s).empty());
    CHECK(s.musicVolume == doctest::Approx(0.4f));
    CHECK_FALSE(s.vsync);
    CHECK(s.windowMode == WindowMode::Windowed);
    (void)applyCommandLine(s, std::vector<std::string>{"-music-volume", "0.9"});
    CHECK(s.musicVolume == doctest::Approx(0.9f));

    reloaded.set("vsync", "sometimes");
    CHECK(reloaded.applyTo(s).size() == 1);
    reloaded.erase("vsync");
    CHECK_FALSE(reloaded.get("vsync").has_value());
    std::filesystem::remove_all(folder);
}
