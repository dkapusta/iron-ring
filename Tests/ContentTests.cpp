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
    CHECK_THROWS_WITH_AS(parseFighter(broken, "x.json"), doctest::Contains("heavyKick"), ContentError);
    broken = minimalFighter();
    broken["moves"]["lightPunch"]["hitbox"] = {1, 2, 3};
    CHECK_THROWS_AS(parseFighter(broken, "x.json"), ContentError);
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
