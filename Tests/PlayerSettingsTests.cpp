#include "Core/PlayerSettings.h"

#include <doctest/doctest.h>

using namespace iron;

TEST_CASE("boot.config parsing") {
    const auto values = parseKeyValueText("# comment\n"
                                          "window-mode = windowed\r\n"
                                          "hdr=off   # trailing comment\n"
                                          "no-equals-sign\n"
                                          "hdr-max-boost=3.5\n"
                                          "bogus=1\n");
    CHECK(values.at("window-mode") == "windowed");
    CHECK(values.at("hdr") == "off");
    CHECK(values.count("no-equals-sign") == 0);

    PlayerSettings s;
    const auto warnings = applyBootConfig(s, values);
    CHECK(s.windowMode == WindowMode::Windowed);
    CHECK(s.hdr == HdrMode::Off);
    CHECK(s.hdrMaxBoost == doctest::Approx(3.5f));
    REQUIRE(warnings.size() == 1);
    CHECK(warnings[0].find("bogus") != std::string::npos);
}

TEST_CASE("command line uses Unity-compatible names and overrides boot.config") {
    PlayerSettings s;
    (void)applyBootConfig(s, parseKeyValueText("window-mode=fullscreen\nvsync=1\n"));

    const std::vector<std::string> args{"-screen-fullscreen", "0", "-screen-width", "1600",
                                        "-screen-height", "900", "-vsync", "0", "-hdr", "on",
                                        "-batchmode", "-frames", "30", "-logFile", "out.log",
                                        "-demo", "-training", "-stage", "the_crown",
                                        "-music-volume", "0.25", "-sfx-volume", "0", "-voice-volume", "0.5"};
    const auto warnings = applyCommandLine(s, args);
    CHECK(warnings.empty());
    CHECK(s.windowMode == WindowMode::Windowed);
    CHECK(s.windowWidth == 1600);
    CHECK(s.windowHeight == 900);
    CHECK_FALSE(s.vsync);
    CHECK(s.hdr == HdrMode::On);
    CHECK(s.batchMode);
    CHECK(s.batchFrames == 30);
    CHECK(s.logFile == "out.log");
    CHECK(s.demo);
    CHECK(s.training);
    CHECK(s.stage == "the_crown");
    CHECK(s.musicVolume == doctest::Approx(0.25f));
    CHECK(s.sfxVolume == 0.0f);
    CHECK(s.voiceVolume == doctest::Approx(0.5f));
}

TEST_CASE("malformed command-line values are reported, not applied") {
    PlayerSettings s;
    const std::vector<std::string> args{"-screen-width", "wide", "-hdr", "maybe", "-unknown", "-frames"};
    const auto warnings = applyCommandLine(s, args);
    CHECK(warnings.size() == 4);
    CHECK(applyCommandLine(s, std::vector<std::string>{"-stage"}).size() == 1);
    CHECK(applyCommandLine(s, std::vector<std::string>{"-music-volume", "2"}).size() == 1);
    CHECK(s.windowWidth == 1280);
    CHECK(s.hdr == HdrMode::Auto);
}

TEST_CASE("screen effects setting") {
    PlayerSettings s;
    CHECK_FALSE(s.reducedEffects);
    CHECK(applyCommandLine(s, std::vector<std::string>{"-screen-effects", "reduced"}).empty());
    CHECK(s.reducedEffects);
    CHECK(applyCommandLine(s, std::vector<std::string>{"-screen-effects", "wobbly"}).size() == 1);
}

TEST_CASE("front-end test flags") {
    PlayerSettings s;
    const std::vector<std::string> args{"-scene", "options", "-keys", "10:Return,30:Down"};
    CHECK(applyCommandLine(s, args).empty());
    CHECK(s.scene == "options");
    CHECK(s.keys == "10:Return,30:Down");
    CHECK(applyCommandLine(s, std::vector<std::string>{"-keys"}).size() == 1);
}
