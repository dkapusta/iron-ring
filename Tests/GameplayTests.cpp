// Throws, throw tech, specials, knockdown and best-of-3 flow.
#include "Simulation/Match.h"
#include "Simulation/Round.h"
#include "Simulation/Training.h"

#include <doctest/doctest.h>

#include <initializer_list>

using namespace iron::sim;

namespace {

constexpr Input kThrowChord = LightPunch | LightKick;

MoveSpec throwMove(int damage, int width, bool techable) {
    MoveSpec m;
    m.kind = MoveKind::Throw;
    m.startup = 3;
    m.active = 2;
    m.recovery = 20;
    m.hitbox = {0, 30, width, 60};
    m.damage = damage;
    m.throwDuration = 40;
    m.throwDistance = 50;
    m.techable = techable;
    m.hitstopOnHit = 8;
    return m;
}

void setMotion(MoveSpec& m, std::initializer_list<int> dirs) {
    m.motionLength = 0;
    for (int d : dirs) m.motion[static_cast<std::size_t>(m.motionLength++)] = static_cast<std::uint8_t>(d);
    m.motionButtons = LightPunch | HeavyPunch;
}

// Kit boxer plus a throw and a rushing uppercut on 236 + P.
FighterSpec rusher() {
    FighterSpec s = kitBoxer();
    s.moves[ThrowSlot] = throwMove(10, 36, true);
    MoveSpec& u = s.moves[SpecialSlot];
    u = {};
    u.startup = 6;
    u.active = 4;
    u.recovery = 20;
    u.hitbox = {10, 40, 44, 60};
    u.damage = 14;
    u.hitstun = 30;
    u.knockback = 12;
    u.knockdown = true;
    u.heavy = true;
    u.advance = 2 * kFixed;
    setMotion(u, {2, 3, 6});
    return s;
}

// Kit grappler plus a throw and an untechable command grab on 63214 + P.
FighterSpec grabber() {
    FighterSpec s = kitGrappler();
    s.moves[ThrowSlot] = throwMove(13, 36, true);
    s.moves[SpecialSlot] = throwMove(20, 56, false);
    setMotion(s.moves[SpecialSlot], {6, 3, 2, 1, 4});
    return s;
}

const FighterSpec& rusherSpec() { static const FighterSpec s = rusher(); return s; }
const FighterSpec& grabberSpec() { static const FighterSpec s = grabber(); return s; }

Round closeRound() {
    Round r(rusherSpec(), grabberSpec());
    r.fighters[0].x = 280 * kFixed;
    r.fighters[1].x = 316 * kFixed;
    return r;
}

void run(Round& r, int ticks, Input p1 = 0, Input p2 = 0) {
    for (int i = 0; i < ticks; ++i) (void)r.step({p1, p2});
}

// Types a motion for player 1 (facing right), one direction per tick, then the button.
void typeMotion(Round& r, std::initializer_list<int> dirs, Input button, int ticksPerDirection = 1) {
    for (int d : dirs)
        for (int t = 0; t < ticksPerDirection; ++t) (void)r.step({inputForDirection(d, r.fighters[0].facing), 0});
    (void)r.step({static_cast<Input>(inputForDirection(*(dirs.end() - 1), 1) | button), 0});
}

} // namespace

TEST_CASE("numpad notation is relative to facing") {
    CHECK(numpadDirection(Down | Right, 1) == 3);
    CHECK(numpadDirection(Down | Right, -1) == 1);
    CHECK(numpadDirection(0, 1) == 5);
    CHECK(numpadDirection(Up | Left, 1) == 7);
    for (int d = 1; d <= 9; ++d) {
        CHECK(numpadDirection(inputForDirection(d, 1), 1) == d);
        CHECK(numpadDirection(inputForDirection(d, -1), -1) == d);
    }
}

TEST_CASE("a throw holds, then damages and knocks down") {
    auto r = closeRound();
    (void)r.step({kThrowChord, 0});
    run(r, 5);
    REQUIRE(r.fighters[0].throwTicks > 0);
    CHECK(r.fighters[1].heldTicks == r.fighters[0].throwTicks);
    run(r, 45);
    CHECK(r.fighters[1].hp == 90);
    CHECK(r.fighters[1].knockdown > 0);
    CHECK(r.fighters[0].throwTicks == 0);
}

TEST_CASE("throw tech") {
    SUBCASE("LP + LK inside the window escapes without damage") {
        auto r = closeRound();
        (void)r.step({kThrowChord, 0});
        run(r, 5);
        REQUIRE(r.fighters[1].techTicks > 0);
        (void)r.step({0, kThrowChord});
        CHECK(r.fighters[1].heldTicks == 0);
        CHECK(r.effect == Effect::ThrowTech);
        run(r, 60);
        CHECK(r.fighters[1].hp == 100);
    }
    SUBCASE("too late, the throw lands") {
        auto r = closeRound();
        (void)r.step({kThrowChord, 0});
        run(r, 5 + kTechWindow);
        (void)r.step({0, kThrowChord});
        run(r, 60);
        CHECK(r.fighters[1].hp == 90);
    }
    SUBCASE("a command grab can't be teched") {
        auto r = closeRound();
        r.fighters[1].x = 330 * kFixed;
        r.fighters[0].facing = 1;
        // Player 2 types 63214 + P while facing left.
        for (int d : {6, 3, 2, 1, 4}) (void)r.step({0, inputForDirection(d, -1)});
        (void)r.step({0, static_cast<Input>(inputForDirection(4, -1) | LightPunch)});
        run(r, 4);
        REQUIRE(r.fighters[1].throwTicks > 0);
        CHECK(r.fighters[0].techTicks == 0);
        (void)r.step({kThrowChord, 0});
        run(r, 60);
        CHECK(r.fighters[0].hp == 80);
    }
}

TEST_CASE("throws can't grab blockstun, airborne or knocked-down opponents") {
    auto blocked = closeRound();
    blocked.fighters[1].stun = 20;
    blocked.fighters[1].blockstun = true;
    (void)blocked.step({kThrowChord, Guard});
    run(blocked, 6, 0, Guard);
    CHECK(blocked.fighters[0].throwTicks == 0);

    auto down = closeRound();
    down.fighters[1].knockdown = 30;
    (void)down.step({kThrowChord, 0});
    run(down, 6);
    CHECK(down.fighters[0].throwTicks == 0);
}

TEST_CASE("simultaneous throws cancel out") {
    auto r = closeRound();
    (void)r.step({kThrowChord, kThrowChord});
    run(r, 6);
    CHECK(r.fighters[0].throwTicks == 0);
    CHECK(r.fighters[1].throwTicks == 0);
    CHECK(r.fighters[0].hp == 100);
    CHECK(r.fighters[1].hp == 100);
}

TEST_CASE("special moves need the motion") {
    SUBCASE("236 + P performs the special") {
        auto r = closeRound();
        r.fighters[1].x = 340 * kFixed;
        typeMotion(r, {2, 3, 6}, HeavyPunch);
        CHECK(r.fighters[0].move == SpecialSlot);
        run(r, 12);
        CHECK(r.fighters[1].hp == 86);
        CHECK(r.fighters[1].knockdown > 0);
    }
    SUBCASE("a motion that is too slow gives a normal attack") {
        auto r = closeRound();
        typeMotion(r, {2, 3, 6}, HeavyPunch, kMotionStepWindow + 2);
        CHECK(r.fighters[0].move == HeavyPunchSlot);
    }
    SUBCASE("the special advances during startup") {
        auto r = closeRound();
        r.fighters[1].x = 400 * kFixed;
        const int start = r.fighters[0].x;
        typeMotion(r, {2, 3, 6}, LightPunch);
        run(r, 9);
        CHECK(r.fighters[0].x - start >= 8 * 2 * kFixed);
    }
}

TEST_CASE("knockdown") {
    SUBCASE("an airborne fighter hit by anything is knocked down") {
        auto r = closeRound();
        r.fighters[1].y = 40 * kFixed;
        r.fighters[1].vy = 0;
        r.fighters[1].x = 300 * kFixed;
        (void)r.step({HeavyPunch, 0});
        run(r, 12);
        CHECK(r.fighters[1].knockdown > 0);
    }
    SUBCASE("a fighter on the floor is invulnerable and gets up") {
        auto r = closeRound();
        r.fighters[1].knockdown = 20;
        (void)r.step({LightPunch, 0});
        run(r, 10);
        CHECK(r.fighters[1].hp == 100);
        run(r, 20);
        CHECK(r.fighters[1].knockdown == 0);
    }
}

TEST_CASE("best of three") {
    Match m(rusherSpec(), grabberSpec());
    SUBCASE("intro: fighters don't move, then the fight starts") {
        for (int i = 0; i < kIntroTicks - 1; ++i) (void)m.step({Right, Left});
        CHECK(m.phase == Phase::Intro);
        CHECK(m.round.fighters[0].x == 230 * kFixed);
        (void)m.step({Right, Left});
        CHECK(m.phase == Phase::Fight);
    }
    SUBCASE("two round wins take the match") {
        int roundsSeen = 0;
        while (!m.finished) {
            if (m.phase == Phase::Fight) m.round.fighters[1].hp = 0; // P1 wins every round
            const auto s = m.step({0, 0});
            if (s.flow == FlowEvent::RoundEnd) ++roundsSeen;
        }
        CHECK(roundsSeen == 2);
        CHECK(m.wins[0] == 2);
        CHECK(m.winner == Outcome::Player1);
    }
    SUBCASE("draws score nothing and the match is capped") {
        while (!m.finished) {
            if (m.phase == Phase::Fight) m.round.remaining = 1; // every round times out level
            (void)m.step({0, 0});
        }
        CHECK(m.roundNumber == kMaxRounds);
        CHECK(m.wins[0] == 0);
        CHECK(m.winner == Outcome::Draw);
    }
}

TEST_CASE("matches with throws and specials stay deterministic") {
    Match a(rusherSpec(), grabberSpec()), b(rusherSpec(), grabberSpec());
    std::uint32_t seed = 1234;
    for (int i = 0; i < 6000; ++i) {
        seed = seed * 1664525U + 1013904223U;
        const std::array<Input, 2> in{static_cast<Input>((seed >> 7) & kInputMask),
                                      static_cast<Input>((seed >> 18) & kInputMask)};
        (void)a.step(in);
        (void)b.step(in);
        REQUIRE(a.checksum() == b.checksum());
    }
    Match snapshot = a;
    for (int i = 0; i < 600; ++i) {
        const std::array<Input, 2> in{static_cast<Input>(i % 5 == 0 ? kThrowChord : Input(Right)), cpuInput(a.round)};
        (void)a.step(in);
        (void)snapshot.step(in);
    }
    CHECK(a.checksum() == snapshot.checksum());
}

TEST_CASE("training rules") {
    SUBCASE("the clock stops and nobody is knocked out") {
        auto r = closeRound();
        r.training = true;
        r.fighters[1].hp = 3;
        const int clock = r.remaining;
        (void)r.step({HeavyPunch, 0});
        run(r, 30);
        CHECK(r.remaining == clock);
        CHECK(r.fighters[1].hp >= 1);
        CHECK(r.outcome == Outcome::Running);
    }
    SUBCASE("health and Heat refill after a second of recovery") {
        auto r = closeRound();
        r.training = true;
        r.fighters[1].hp = 40;
        r.fighters[0].meter = 10;
        run(r, kTrainingRefillTicks + 1);
        CHECK(r.fighters[1].hp == kMaxHealth);
        CHECK(r.fighters[0].meter == kMeterMax);
    }
    SUBCASE("reset keeps the rules; the checksum only changes under them") {
        Round a, b;
        b.training = true;
        b.reset();
        CHECK(b.training);
        CHECK(a.checksum() != b.checksum());
        b.training = false;
        CHECK(a.checksum() == b.checksum());
    }
}

TEST_CASE("training dummy") {
    // A low sweep from player 1, against each guard mode.
    FighterSpec sweeper = rusher();
    sweeper.moves[HeavyKickSlot].low = true;
    const auto sweepHits = [&](Dummy mode) {
        Round r(sweeper, grabberSpec());
        r.fighters[0].x = 280 * kFixed;
        r.fighters[1].x = 316 * kFixed;
        r.training = true;
        bool attacked = false;
        for (int t = 0; t < 40; ++t) {
            const Input p1 = (!attacked && t == 10) ? Input(HeavyKick) : Input(0);
            attacked = attacked || t == 10;
            (void)r.step({p1, dummyInput(r, 1, mode)});
        }
        return r.fighters[1].hp < kMaxHealth;
    };
    CHECK(sweepHits(Dummy::Stand));
    CHECK(sweepHits(Dummy::Guard));       // standing guard loses to lows
    CHECK_FALSE(sweepHits(Dummy::GuardAll));

    CHECK(nextDummy(Dummy::Cpu) == Dummy::Stand);
    CHECK(dummyName(Dummy::GuardAll) == "GUARD ALL");
    CHECK(parseDummy("guard-all") == Dummy::GuardAll);
    CHECK(parseDummy("CPU") == Dummy::Cpu);
    CHECK_FALSE(parseDummy("sleep").has_value());
    Round r(rusherSpec(), grabberSpec());
    CHECK(dummyInput(r, 1, Dummy::Crouch) == Down);
    CHECK(dummyInput(r, 1, Dummy::Cpu) == cpuInput(r, 1));
}

TEST_CASE("per-phase hurtboxes") {
    // Player 2 starts a long-startup move that ducks low; player 1's jab is high.
    FighterSpec ducker = grabber();
    MoveSpec& duck = ducker.moves[HeavyKickSlot];
    duck.startup = 30;
    duck.hurt[static_cast<std::size_t>(MovePhase::Startup)] = {Box{-18, 0, 40, 40}, Box{}};
    const auto jabLands = [](const FighterSpec& p2spec, Input p2) {
        Round r(rusherSpec(), p2spec);
        r.fighters[0].x = 280 * kFixed;
        r.fighters[1].x = 316 * kFixed;
        (void)r.step({0, p2});                 // player 2 starts the move (or not)
        (void)r.step({LightPunch, 0});
        run(r, 10);
        return r.fighters[1].hp < kMaxHealth;
    };
    CHECK(jabLands(ducker, 0));                // standing: the jab hits
    CHECK_FALSE(jabLands(ducker, HeavyKick));  // ducking under it during startup

    // An extended limb can be hit where the body can't.
    FighterSpec reacher = grabber();
    MoveSpec& limb = reacher.moves[LightPunchSlot];
    limb.hurt[static_cast<std::size_t>(MovePhase::Active)] = {Box{-20, 0, 40, 120}, Box{20, 80, 40, 16}};
    Round r(rusherSpec(), reacher);
    Fighter& f = r.fighters[1];
    f.move = LightPunchSlot;
    f.age = limb.startup;  // active
    const WorldBox reach{f.x - 55 * kFixed, 85 * kFixed, f.x - 45 * kFixed, 90 * kFixed};  // in front of p2 (facing left)
    CHECK(r.touches(reach, f));
    f.age = 0;             // startup: default box only
    CHECK_FALSE(r.touches(reach, f));
}
