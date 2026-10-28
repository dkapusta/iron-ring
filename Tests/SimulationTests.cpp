// Combat invariants ported from the Brass Current kit prototype, plus a golden
// checksum that pins the port to the original behaviour bit-for-bit.
#include "Simulation/Combat.h"

#include <doctest/doctest.h>

using namespace iron::sim;

namespace {

Match closeMatch() {
    Match m;
    m.fighters[0].x = 280 * kFixed;
    m.fighters[1].x = 325 * kFixed;
    return m;
}

void idle(Match& m, int frames, Input p2 = 0) {
    for (int i = 0; i < frames; ++i) (void)m.step({0, p2});
}

constexpr Input operator|(Button a, Button b) { return static_cast<Input>(static_cast<Input>(a) | b); }
constexpr Input operator|(Input a, Button b) { return static_cast<Input>(a | static_cast<Input>(b)); }

// Same LCG stream as the kit's test, so the final checksum must match.
template <typename F>
void randomHistory(int ticks, F&& onTick) {
    std::uint32_t seed = 73;
    for (int i = 0; i < ticks; ++i) {
        seed = seed * 1664525U + 1013904223U;
        onTick(std::array<Input, 2>{static_cast<Input>((seed >> 7) & kInputMask),
                                    static_cast<Input>((seed >> 18) & kInputMask)});
    }
}

} // namespace

TEST_CASE("a jab deals damage once, not once per active frame") {
    auto m = closeMatch();
    (void)m.step({LightPunch, 0});
    idle(m, 40);
    CHECK(m.fighters[1].hp == 94);
}

TEST_CASE("guard prevents normal hit damage and grants meter once") {
    auto m = closeMatch();
    (void)m.step({LightPunch, Guard});
    idle(m, 40, Guard);
    CHECK(m.fighters[1].hp == 100);
    CHECK(m.fighters[1].meter == 8);
}

TEST_CASE("high/low defense") {
    SUBCASE("standing guard loses to a sweep") {
        auto m = closeMatch();
        (void)m.step({HeavyKick, Guard});
        idle(m, 45, Guard);
        CHECK(m.fighters[1].hp == 86);
    }
    SUBCASE("crouch guard blocks a sweep") {
        auto m = closeMatch();
        (void)m.step({HeavyKick, Guard | Down});
        idle(m, 45, Guard | Down);
        CHECK(m.fighters[1].hp == 100);
    }
}

TEST_CASE("equal-frame contacts trade symmetrically") {
    auto m = closeMatch();
    (void)m.step({LightPunch, LightPunch});
    idle(m, 12);
    CHECK(m.fighters[0].hp == 94);
    CHECK(m.fighters[1].hp == 94);
}

TEST_CASE("round timer pauses during hitstop") {
    auto m = closeMatch();
    m.hitstop = 3;
    const int clock = m.remaining;
    idle(m, 3);
    CHECK(m.remaining == clock);
}

TEST_CASE("fighters stay within stage bounds") {
    Match m;
    for (int i = 0; i < 2000; ++i) (void)m.step({Left, Right});
    CHECK(m.fighters[0].x == kStageMinX * kFixed);
    CHECK(m.fighters[1].x == kStageMaxX * kFixed);
}

TEST_CASE("Current Break spends meter and pushes without damage") {
    auto m = closeMatch();
    m.fighters[0].meter = kBreakCost;
    const auto events = m.step({Guard | HeavyPunch | HeavyKick, 0});
    CHECK(m.fighters[0].meter == 0);
    CHECK(m.fighters[1].hp == 100);
    CHECK(m.effect == Effect::CurrentBreak);
    REQUIRE(events.count == 1);
    CHECK(events.items[0].effect == Effect::CurrentBreak);
}

TEST_CASE("Current Break entered during hitstop executes from blockstun") {
    auto m = closeMatch();
    m.fighters[0].meter = kBreakCost;
    (void)m.step({Guard, LightPunch});
    for (int i = 0; i < 12 && !m.fighters[0].blockstun; ++i) (void)m.step({Guard, 0});
    REQUIRE(m.fighters[0].blockstun);
    REQUIRE(m.hitstop > 0);

    (void)m.step({Guard | HeavyPunch | HeavyKick, 0});
    for (int i = 0; i < 5; ++i) (void)m.step({Guard, 0});
    CHECK(m.fighters[0].meter == 8);
    CHECK_FALSE(m.fighters[0].blockstun);
    CHECK(m.effect == Effect::CurrentBreak);
}

TEST_CASE("identical input histories produce identical checksums every tick") {
    Match a, b;
    randomHistory(4000, [&](const std::array<Input, 2>& in) {
        (void)a.step(in);
        (void)b.step(in);
        REQUIRE(a.checksum() == b.checksum());
    });
}

TEST_CASE("port matches the kit prototype bit-for-bit (golden checksum)") {
    Match m;
    randomHistory(4000, [&](const std::array<Input, 2>& in) { (void)m.step(in); });
    CHECK(m.checksum() == 17886306297783925268ULL);
}

TEST_CASE("a copied match is a snapshot that reproduces subsequent state") {
    Match live;
    idle(live, 123);
    Match restored = live;
    for (int i = 0; i < 800; ++i) {
        const std::array<Input, 2> in{static_cast<Input>(i % 47 == 0 ? LightPunch : Right), cpuInput(live)};
        (void)live.step(in);
        (void)restored.step(in);
    }
    CHECK(live.checksum() == restored.checksum());
}

TEST_CASE("round end conditions") {
    SUBCASE("a tied timeout is a draw") {
        Match m;
        idle(m, kRoundSeconds * kTickRate);
        CHECK(m.outcome == Outcome::Draw);
        CHECK(m.remaining == 0);
    }
    SUBCASE("a knockout names the winner") {
        auto m = closeMatch();
        m.fighters[1].hp = 6;
        (void)m.step({LightPunch, 0});
        idle(m, 10);
        CHECK(m.outcome == Outcome::Player1);
    }
}
