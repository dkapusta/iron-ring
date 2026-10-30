#pragma once

// Best-of-N match flow around Round: intro, fight, round over, match over.
// Deterministic and checksummed like Round, so replays and (later) rollback
// cover round transitions too.

#include "Simulation/Round.h"

namespace iron::sim {

inline constexpr int kRoundsToWin = 2;
inline constexpr int kMaxRounds = 5;      // draws can't extend a match forever
inline constexpr int kIntroTicks = 120;   // "ROUND n" then "FIGHT!"
inline constexpr int kFightCallTick = 60; // when "FIGHT!" replaces "ROUND n"
inline constexpr int kRoundOverTicks = 180;
inline constexpr int kMatchOverTicks = 300;

enum class Phase : std::int32_t { Intro = 0, Fight = 1, RoundOver = 2, MatchOver = 3 };

enum class FlowEvent : std::int32_t { None = 0, RoundCall, Fight, RoundEnd, MatchEnd, Finished };

struct MatchStep {
    StepEvents round;
    FlowEvent flow = FlowEvent::None;
};

struct Match {
    Round round;
    int roundNumber = 1;
    std::array<int, 2> wins{};
    Phase phase = Phase::Intro;
    int phaseTicks = 0;
    Outcome lastRound = Outcome::Running; // result of the most recent round
    Outcome winner = Outcome::Running;    // set when the match is decided
    bool finished = false;                // MatchOver has run its course

    Match(); // kit specs
    Match(const FighterSpec& p1, const FighterSpec& p2);
    void reset();
    MatchStep step(const std::array<Input, 2>& inputs);
    [[nodiscard]] std::uint64_t checksum() const;

private:
    void enter(Phase next);
};

// True when this round decides the match whatever happens (both one win away,
// or the last round allowed). Presentation calls it "FINAL ROUND".
[[nodiscard]] inline bool isFinalRound(const Match& m) {
    return m.roundNumber == kMaxRounds || (m.wins[0] == kRoundsToWin - 1 && m.wins[1] == kRoundsToWin - 1);
}

} // namespace iron::sim
