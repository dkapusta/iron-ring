// Combat invariants ported from the Brass Current kit prototype, plus a golden
// checksum that pins simulation behaviour for the kit specs.
#include "Simulation/Match.h"
#include "Simulation/Round.h"

#include <doctest/doctest.h>

using namespace iron::sim;

namespace {

Round closeRound() {
    Round m;
    m.fighters[0].x = 280 * kFixed;
    m.fighters[1].x = 325 * kFixed;
    return m;
}

void idle(Round& m, int frames, Input p2 = 0) {
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
    auto m = closeRound();
    (void)m.step({LightPunch, 0});
    idle(m, 40);
    CHECK(m.fighters[1].hp == 94);
}

TEST_CASE("guard prevents normal hit damage and grants meter once") {
    auto m = closeRound();
    (void)m.step({LightPunch, Guard});
    idle(m, 40, Guard);
    CHECK(m.fighters[1].hp == 100);
    CHECK(m.fighters[1].meter == 5);
}

TEST_CASE("high/low defense") {
    SUBCASE("standing guard loses to a sweep") {
        auto m = closeRound();
        (void)m.step({HeavyKick, Guard});
        idle(m, 45, Guard);
        CHECK(m.fighters[1].hp == 86);
    }
    SUBCASE("crouch guard blocks a sweep") {
        auto m = closeRound();
        (void)m.step({HeavyKick, Guard | Down});
        idle(m, 45, Guard | Down);
        CHECK(m.fighters[1].hp == 100);
    }
}

TEST_CASE("equal-frame contacts trade symmetrically") {
    auto m = closeRound();
    (void)m.step({LightPunch, LightPunch});
    idle(m, 12);
    CHECK(m.fighters[0].hp == 94);
    CHECK(m.fighters[1].hp == 94);
}

TEST_CASE("round timer pauses during hitstop") {
    auto m = closeRound();
    m.hitstop = 3;
    const int clock = m.remaining;
    idle(m, 3);
    CHECK(m.remaining == clock);
}

TEST_CASE("fighters stay within stage bounds") {
    Round m;
    for (int i = 0; i < 2000; ++i) (void)m.step({Left, Right});
    CHECK(m.fighters[0].x == kStageMinX * kFixed);
    CHECK(m.fighters[1].x == kStageMaxX * kFixed);
}

TEST_CASE("Breakaway is a guard cancel: nothing happens outside blockstun") {
    auto m = closeRound();
    m.fighters[0].meter = kBreakCost;
    const auto events = m.step({Guard | HeavyPunch | HeavyKick, 0});
    CHECK(m.fighters[0].meter == kBreakCost);
    CHECK(events.count == 0);
}

TEST_CASE("Breakaway entered during hitstop executes from blockstun") {
    auto m = closeRound();
    m.fighters[0].meter = kBreakCost;
    (void)m.step({Guard, LightPunch});
    for (int i = 0; i < 12 && !m.fighters[0].blockstun; ++i) (void)m.step({Guard, 0});
    REQUIRE(m.fighters[0].blockstun);
    REQUIRE(m.hitstop > 0);

    (void)m.step({Guard | HeavyPunch | HeavyKick, 0}); // entered in hitstop, buffered
    for (int i = 0; i < 5 && m.effect != Effect::Breakaway; ++i) (void)m.step({Guard, 0});
    CHECK(m.fighters[0].meter == 5); // 50 + 5 for the block - 50
    CHECK_FALSE(m.fighters[0].blockstun);
    CHECK(m.fighters[0].stun == 0);
    CHECK(m.effect == Effect::Breakaway);
    // The attacker is staggered (no damage), not pushed out of reach.
    CHECK(m.fighters[1].stun > kBreakStagger - 6);
    CHECK(m.fighters[1].move < 0);
    CHECK(m.fighters[1].hp == 100);
}

TEST_CASE("identical input histories produce identical checksums every tick") {
    Round a, b;
    randomHistory(4000, [&](const std::array<Input, 2>& in) {
        (void)a.step(in);
        (void)b.step(in);
        REQUIRE(a.checksum() == b.checksum());
    });
}

// History of this constant (update it only for intended gameplay changes):
//   17886306297783925268  kit prototype, reproduced bit-for-bit by the port
//    8522570831175173500  knockdown (air hits, KO), throw/special state in the
//                         checksum, effects placed at the hitbox centre
//   15812912959688531244  counter hits, combo scaling (combo in the checksum),
//                         Breakaway as a guard cancel that staggers (docs/design/heat.md)
TEST_CASE("simulation behaviour is pinned (golden checksum)") {
    Round m;
    randomHistory(4000, [&](const std::array<Input, 2>& in) { (void)m.step(in); });
    CHECK(m.checksum() == 15812912959688531244ULL);
}

TEST_CASE("a copied match is a snapshot that reproduces subsequent state") {
    Round live;
    idle(live, 123);
    Round restored = live;
    for (int i = 0; i < 800; ++i) {
        const std::array<Input, 2> in{static_cast<Input>(i % 47 == 0 ? LightPunch : Right), cpuInput(live)};
        (void)live.step(in);
        (void)restored.step(in);
    }
    CHECK(live.checksum() == restored.checksum());
}

TEST_CASE("round end conditions") {
    SUBCASE("a tied timeout is a draw") {
        Round m;
        idle(m, kRoundSeconds * kTickRate);
        CHECK(m.outcome == Outcome::Draw);
        CHECK(m.remaining == 0);
    }
    SUBCASE("a knockout names the winner") {
        auto m = closeRound();
        m.fighters[1].hp = 6;
        (void)m.step({LightPunch, 0});
        idle(m, 10);
        CHECK(m.outcome == Outcome::Player1);
    }
}
