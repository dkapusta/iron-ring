#include "Content/Atlas.h"
#include "Content/Fighter.h"
#include "Content/Paths.h"
#include "Content/Validation.h"
#include "Simulation/Match.h"
#include "Vfs/DirectoryMount.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <memory>

using namespace iron;
using namespace iron::content;
using nlohmann::json;

namespace {

const std::filesystem::path kSourceDir{IR_SOURCE_DIR};

// Reads virtual paths the way the packed game sees them.
FileReader projectReader() {
    auto assets = std::make_shared<vfs::DirectoryMount>(kSourceDir / "Assets");
    auto root = std::make_shared<vfs::DirectoryMount>(kSourceDir);
    return [assets, root](std::string_view path) -> std::optional<vfs::Bytes> {
        if (path.rfind("ProjectSettings/", 0) == 0) return root->read(path);
        return assets->read(path);
    };
}

json readJson(const FileReader& read, std::string_view path) {
    const auto bytes = read(path);
    REQUIRE(bytes);
    return json::parse(reinterpret_cast<const char*>(bytes->data()),
                       reinterpret_cast<const char*>(bytes->data() + bytes->size()));
}

FighterDefinition shippedFighter(std::string_view path) {
    return parseFighter(readJson(projectReader(), path), path);
}

// [[name, ticks], ...] spelled out: nested braces would become JSON objects.
json frames(std::initializer_list<std::pair<const char*, int>> list) {
    json out = json::array();
    for (const auto& [name, ticks] : list) out.push_back(json::array({name, ticks}));
    return out;
}

json minimalFighter() {
    const json strike = {{"animation", "jab"}, {"startup", 2}, {"active", 1}, {"recovery", 3},
                         {"hitbox", {20, 80, 20, 10}}, {"damage", 5}, {"hitstun", 10}};
    return {
        {"id", "test"}, {"name", "Test"}, {"atlas", "test_atlas.json"}, {"portrait", "test.png"},
        {"physics", {{"walkForward", 400}}},
        {"body", {{"standHurt", {-10, 0, 20, 100}}, {"crouchHurt", {-10, 0, 20, 60}}, {"knockdownTicks", 6}}},
        {"moves", {{"lightPunch", strike}, {"heavyPunch", strike}, {"lightKick", strike}, {"heavyKick", strike},
                   {"special", {{"animation", "jab"}, {"input", "214K"}, {"startup", 2}, {"active", 1},
                                {"recovery", 3}, {"hitbox", {0, 0, 10, 10}}}}}},
        {"animations", {{"jab", {{"frames", frames({{"a", 2}, {"b", 1}, {"a", 3}})}}},
                        {"idle", {{"loop", true}, {"frames", frames({{"a", 4}, {"b", 4}})}}}}},
    };
}

} // namespace

TEST_CASE("motion notation") {
    sim::MoveSpec m;
    parseMotion("236P", m);
    CHECK(m.motionLength == 3);
    CHECK(m.motion[0] == 2);
    CHECK(m.motion[2] == 6);
    CHECK(m.motionButtons == (sim::LightPunch | sim::HeavyPunch));
    parseMotion("63214HK", m);
    CHECK(m.motionLength == 5);
    CHECK(m.motionButtons == sim::HeavyKick);
    CHECK_THROWS_AS(parseMotion("P", m), ContentError);
    CHECK_THROWS_AS(parseMotion("236X", m), ContentError);
    CHECK_THROWS_AS(parseMotion("2362362P", m), ContentError);
}

TEST_CASE("fighter parsing") {
    const auto d = parseFighter(minimalFighter(), "Fighters/Test/test.fighter.json");
    CHECK(d.atlas == "Fighters/Test/test_atlas.json");
    CHECK(d.spec.walkForward == 400);
    CHECK(d.spec.walkBack == 420); // default kept
    CHECK(d.spec.moves[sim::LightPunchSlot].hitbox.y == 80);
    CHECK_FALSE(d.spec.moves[sim::ThrowSlot].enabled);
    CHECK(d.spec.moves[sim::SpecialSlot].motionButtons == (sim::LightKick | sim::HeavyKick));
    CHECK(d.moves[sim::SpecialSlot].input == "214K");

    auto broken = minimalFighter();
    broken["moves"].erase("heavyKick");
    CHECK_THROWS_WITH_AS((void)parseFighter(broken, "x.json"), doctest::Contains("heavyKick"), ContentError);
    broken = minimalFighter();
    broken["moves"]["lightPunch"]["hitbox"] = {1, 2, 3};
    CHECK_THROWS_AS((void)parseFighter(broken, "x.json"), ContentError);
}

TEST_CASE("per-phase hurtboxes") {
    auto j = minimalFighter();
    j["moves"]["heavyKick"]["hurt"] = json::parse(R"({"startup": [[-18, 0, 40, 90]],
                                                      "active": [[-18, 0, 38, 80], [20, 0, 60, 14]]})");
    const auto d = parseFighter(j, "x.json");
    const auto& hurt = d.spec.moves[sim::HeavyKickSlot].hurt;
    CHECK(hurt[0][0].h == 90);
    CHECK(hurt[0][1].empty());
    CHECK(hurt[1][1].w == 60);
    CHECK(hurt[2][0].empty()); // recovery falls back to the stand/crouch box

    j["moves"]["heavyKick"]["hurt"] = json::parse(R"({"windup": [[0, 0, 1, 1]]})");
    CHECK_THROWS_WITH_AS((void)parseFighter(j, "x.json"), doctest::Contains("windup"), ContentError);
    j["moves"]["heavyKick"]["hurt"] = json::parse(R"({"active": [[0, 0, 1, 1], [0, 0, 1, 1], [0, 0, 1, 1]]})");
    CHECK_THROWS_AS((void)parseFighter(j, "x.json"), ContentError);
}

TEST_CASE("animation timing") {
    const auto d = parseFighter(minimalFighter(), "t.json");
    const auto& jab = d.animation("jab");
    CHECK(jab.totalTicks() == 6);
    CHECK(jab.at(0).frame == "a");
    CHECK(jab.at(2).frame == "b");
    CHECK(jab.at(5).frame == "a");
    CHECK(jab.at(99).frame == "a"); // holds the last frame
    const auto& idle = d.animation("idle");
    CHECK(idle.at(9).frame == "a"); // loops
}

TEST_CASE("fighter validation catches timing and frame mistakes") {
    auto j = minimalFighter();
    j["animations"]["jab"]["frames"] = frames({{"a", 2}, {"missing", 2}});
    const auto d = parseFighter(j, "t.json");
    AtlasDefinition atlas;
    atlas.frames["a"] = {0, 0, 10, 10, 5, 10, {}};
    atlas.frames["held_0"] = atlas.frames["held_1"] = atlas.frames["a"];
    const auto errors = validateFighter(d, atlas, {64, 64});
    const auto has = [&](std::string_view text) {
        return std::any_of(errors.begin(), errors.end(), [&](const auto& e) { return e.find(text) != e.npos; });
    };
    CHECK(has("unknown frame 'missing'"));
    CHECK(has("lasts 4 ticks, the move 6"));
    CHECK(has("missing animation 'walk'"));
}

TEST_CASE("PNG header reader") {
    const auto bytes = projectReader()("Stages/PierNine/pier_nine.png");
    REQUIRE(bytes);
    const auto size = pngSize(*bytes);
    REQUIRE(size);
    CHECK(size->first == 640);
    CHECK(size->second == 360);
    CHECK_FALSE(pngSize(vfs::Bytes(30)));
}

TEST_CASE("the shipped project validates") {
    const auto report = validateProject(projectReader());
    for (const auto& e : report.errors) MESSAGE(e);
    CHECK(report.ok());
}

TEST_CASE("shipped fighters: a close throw connects and can be teched") {
    const auto dee = shippedFighter("Fighters/Dee/dee.fighter.json");
    const auto bram = shippedFighter("Fighters/Bram/bram.fighter.json");
    sim::Round r(dee.spec, bram.spec);
    r.fighters[0].x = 300 * sim::kFixed;
    r.fighters[1].x = 340 * sim::kFixed;
    (void)r.step({static_cast<sim::Input>(sim::LightPunch | sim::LightKick), 0});
    for (int i = 0; i < 5; ++i) (void)r.step({0, 0});
    CHECK(r.fighters[0].throwTicks > 0);
}

TEST_CASE("shipped fighters play a deterministic match") {
    const auto dee = shippedFighter("Fighters/Dee/dee.fighter.json");
    const auto bram = shippedFighter("Fighters/Bram/bram.fighter.json");
    sim::Match a(dee.spec, bram.spec), b(dee.spec, bram.spec);
    int specials = 0, throws = 0;
    std::uint32_t seed = 99;
    for (int i = 0; i < 20000 && !a.finished; ++i) {
        seed = seed * 1664525U + 1013904223U;
        const auto p1 = static_cast<sim::Input>((seed >> 9) & sim::kInputMask);
        const std::array<sim::Input, 2> in{p1, sim::cpuInput(a.round)};
        (void)a.step(in);
        (void)b.step(in);
        REQUIRE(a.checksum() == b.checksum());
        specials += a.round.fighters[1].move == sim::SpecialSlot;
        throws += a.round.fighters[1].throwTicks > 0 || a.round.fighters[0].throwTicks > 0;
    }
    CHECK(a.finished);
    CHECK(specials > 0);
    CHECK(throws > 0);
}

#include "Simulation/Cpu.h"

#include "Content/Arcade.h"

TEST_CASE("the shipped arcade ladder: no repeats, Greta at her Foundry, Bram at the Crown") {
    const auto ladder = iron::content::parseArcade(readJson(projectReader(), "Data/arcade.json"), "arcade.json");
    const std::vector<std::string> playable{"dee", "bram", "sable", "idris", "greta"};
    for (const auto& player : playable) {
        const auto run = iron::content::arcadeOpponents(ladder, player, playable);
        REQUIRE(run.size() == 4);
        for (std::size_t i = 0; i < run.size(); ++i) {
            CHECK(run[i] != player);
            for (std::size_t j = 0; j < i; ++j) CHECK(run[i] != run[j]);
        }
        if (player != "greta") CHECK(run[2] == "greta");
        if (player != "bram") CHECK(run[3] == "bram");
    }
}

TEST_CASE("the CPU lands every new fighter's special (236K, 214K, 623P)") {
    const auto dee = shippedFighter("Fighters/Dee/dee.fighter.json");
    for (const char* path : {"Fighters/Sable/sable.fighter.json", "Fighters/Idris/idris.fighter.json",
                             "Fighters/Greta/greta.fighter.json"}) {
        CAPTURE(path);
        const auto fighter = shippedFighter(path);
        int specials = 0;
        for (std::uint32_t seed = 1; seed <= 4; ++seed) { // a few matches: one can go by without one
            sim::Match m(fighter.spec, dee.spec);
            sim::CpuPlayer a(4, seed * 7), b(2, seed * 11);
            int lastMove = -1;
            for (int i = 0; i < 30000 && !m.finished; ++i) {
                (void)m.step({a.think(m.round, 0), b.think(m.round, 1)});
                const int move = m.round.fighters[0].move;
                specials += move == sim::SpecialSlot && lastMove != sim::SpecialSlot;
                lastMove = move;
            }
            CHECK(m.finished);
        }
        CHECK(specials >= 4);
    }
}

#include "Content/Stage.h"

TEST_CASE("layered stages: parsing and the layer width rule") {
    const auto yard = nlohmann::json::parse(R"({
        "id": "yard", "name": "Yard", "image": "Stages/Yard/yard.png", "width": 800,
        "layers": [
            {"image": "Stages/Yard/sky.png", "parallax": 0.25},
            {"image": "Stages/Yard/crowd.png", "parallax": 0.8, "y": 186, "frames": 8, "crowd": true},
            {"image": "Stages/Yard/front.png", "parallax": 1.25, "x": -10, "front": true}
        ]})");
    const auto stage = iron::content::parseStage(yard, "stages.json");
    REQUIRE(stage.layered());
    CHECK(stage.width == 800);
    CHECK(stage.requiredWidth(stage.layers[0]) == 680);
    CHECK(stage.requiredWidth(stage.layers[1]) == 768);
    CHECK(stage.requiredWidth(stage.layers[2]) == 850); // 640 + 200, plus the 10 px it starts off screen
    CHECK(stage.layers[1].crowd);
    CHECK(stage.layers[2].front);

    auto bad = yard;
    bad["layers"][1]["frames"] = 3; // a crowd needs rest and cheer halves
    CHECK_THROWS_AS((void)iron::content::parseStage(bad, "stages.json"), iron::content::ContentError);
    bad = yard;
    bad["width"] = 500;
    CHECK_THROWS_AS((void)iron::content::parseStage(bad, "stages.json"), iron::content::ContentError);
}

#include "Content/Arcade.h"

TEST_CASE("arcade ladder: candidate lists meet everyone once") {
    const auto ladder = iron::content::parseArcade(nlohmann::json::parse(R"({"bouts": [
        {"stage": "pier_nine", "opponent": ["sable", "dee"]},
        {"stage": "lantern_row", "opponent": ["dee", "sable"]},
        {"stage": "foundry", "opponent": "rival"},
        {"stage": "the_crown", "opponent": ["bram", "dee"], "final": true}]})"),
                                                  "arcade.json");
    using Ids = std::vector<std::string>;
    const Ids playable{"dee", "bram", "sable"};
    // Each fighter meets the other two before anyone twice, and Bram (the gatekeeper) waits at the Crown.
    CHECK(iron::content::arcadeOpponents(ladder, "dee", playable) == Ids{"sable", "bram", "sable", "bram"});
    CHECK(iron::content::arcadeOpponents(ladder, "sable", playable) == Ids{"dee", "bram", "dee", "bram"});
    CHECK(iron::content::arcadeOpponents(ladder, "bram", playable) == Ids{"sable", "dee", "sable", "dee"});
    // A listed fighter who isn't playable is skipped.
    CHECK(iron::content::arcadeOpponents(ladder, "dee", Ids{"dee", "bram"}) == Ids{"bram", "bram", "bram", "bram"});
    CHECK_THROWS_AS((void)iron::content::parseArcade(
                        nlohmann::json::parse(R"({"bouts": [{"stage": "x", "opponent": []}]})"), "arcade.json"),
                    iron::content::ContentError);
}

TEST_CASE("arcade ladder: opponents, levels and the run's steps") {
    const auto ladder = iron::content::parseArcade(nlohmann::json::parse(R"({"bouts": [
        {"stage": "pier_nine", "opponent": "rival", "level": 1},
        {"stage": "lantern_row", "opponent": "mirror", "level": 1},
        {"stage": "the_crown", "opponent": "bram", "level": 4, "final": true}]})"),
                                                  "arcade.json");
    REQUIRE(ladder.bouts.size() == 3);
    using Ids = std::vector<std::string>;
    const Ids playable{"dee", "bram"};
    CHECK(iron::content::arcadeOpponents(ladder, "dee", playable) == Ids{"bram", "dee", "bram"});
    CHECK(iron::content::arcadeOpponents(ladder, "bram", playable) == Ids{"dee", "bram", "dee"}); // can't fight self
    CHECK(iron::content::arcadeOpponents(ladder, "dee", Ids{"dee"}) == Ids{"dee", "dee", "dee"}); // a roster of one
    CHECK(iron::content::arcadeLevel(ladder.bouts[0], 0) == 0); // EASY: one lower
    CHECK(iron::content::arcadeLevel(ladder.bouts[0], 2) == 2); // HARD: one higher
    CHECK(iron::content::arcadeLevel(ladder.bouts[2], 2) == 4); // capped at CHAMPION

    using Step = iron::content::ArcadeRun::Step;
    iron::content::ArcadeRun run;
    run.startFight();
    run.finished(false, 3);
    CHECK(run.step == Step::Continue);
    run.continueRun();
    CHECK(run.step == Step::Versus);
    CHECK(run.bout == 0); // the same bout again
    CHECK(run.continuesUsed == 1);
    run.finished(true, 3);
    run.finished(true, 3);
    CHECK(run.bout == 2);
    run.finished(true, 3);
    CHECK(run.step == Step::Ending);
    run.giveUp();
    CHECK(run.step == Step::GameOver);

    CHECK_THROWS_AS((void)iron::content::parseArcade(nlohmann::json::parse(R"({"bouts": []})"), "a.json"),
                    iron::content::ContentError);
}

// Not a check: a CPU-vs-CPU win matrix at VETERAN for tuning frame data.
// Run with: IronRingTests -s -tc="balance*" --no-skip. Rows win out of 100 against columns.
TEST_CASE("balance: CPU win matrix" * doctest::skip()) {
    const char* ids[]{"dee", "bram", "sable", "idris", "greta"};
    const char* paths[]{"Fighters/Dee/dee.fighter.json", "Fighters/Bram/bram.fighter.json",
                        "Fighters/Sable/sable.fighter.json", "Fighters/Idris/idris.fighter.json",
                        "Fighters/Greta/greta.fighter.json"};
    std::vector<FighterDefinition> f;
    for (auto* p : paths) f.push_back(shippedFighter(p));
    MESSAGE("        dee bram sable idris greta");
    for (int i = 0; i < 5; ++i) {
        std::string line = ids[i];
        for (int j = 0; j < 5; ++j) {
            int wins = 0;
            for (std::uint32_t seed = 1; seed <= 100; ++seed) {
                const bool swap = seed % 2;
                sim::Match m(f[swap ? j : i].spec, f[swap ? i : j].spec);
                sim::CpuPlayer a(3, seed * 7), b(3, seed * 13 + 1);
                for (int k = 0; k < 40000 && !m.finished; ++k) (void)m.step({a.think(m.round, 0), b.think(m.round, 1)});
                const int mine = swap ? 1 : 0;
                wins += m.winner == (mine ? sim::Outcome::Player2 : sim::Outcome::Player1);
            }
            line += " " + std::to_string(wins);
        }
        MESSAGE(line);
    }
}
