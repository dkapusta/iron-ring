#include "Simulation/Match.h"

namespace iron::sim {

Match::Match() : round() {}

Match::Match(const FighterSpec& p1, const FighterSpec& p2) : round(p1, p2) {}

void Match::reset() {
    round.reset();
    roundNumber = 1;
    wins = {};
    lastRound = Outcome::Running;
    winner = Outcome::Running;
    finished = false;
    enter(Phase::Intro);
}

void Match::enter(Phase next) {
    phase = next;
    phaseTicks = 0;
}

MatchStep Match::step(const std::array<Input, 2>& inputs) {
    MatchStep result;
    switch (phase) {
    case Phase::Intro:
        if (phaseTicks == 0) result.flow = FlowEvent::RoundCall;
        if (phaseTicks == kFightCallTick) result.flow = FlowEvent::Fight;
        if (++phaseTicks >= kIntroTicks) enter(Phase::Fight);
        break;

    case Phase::Fight:
        result.round = round.step(inputs);
        ++phaseTicks;
        if (round.outcome != Outcome::Running) {
            lastRound = round.outcome;
            if (lastRound == Outcome::Player1) ++wins[0];
            if (lastRound == Outcome::Player2) ++wins[1];
            result.flow = FlowEvent::RoundEnd;
            enter(Phase::RoundOver);
        }
        break;

    case Phase::RoundOver:
        if (++phaseTicks < kRoundOverTicks) break;
        if (wins[0] >= kRoundsToWin || wins[1] >= kRoundsToWin || roundNumber >= kMaxRounds) {
            winner = wins[0] == wins[1] ? Outcome::Draw : (wins[0] > wins[1] ? Outcome::Player1 : Outcome::Player2);
            result.flow = FlowEvent::MatchEnd;
            enter(Phase::MatchOver);
        } else {
            ++roundNumber;
            // Heat lasts the whole match: what a fighter saved carries into the next round.
            const std::array<int, 2> heat{round.fighters[0].meter, round.fighters[1].meter};
            round.reset();
            for (std::size_t i = 0; i < 2; ++i) round.fighters[i].meter = heat[i];
            enter(Phase::Intro);
        }
        break;

    case Phase::MatchOver:
        if (!finished && ++phaseTicks >= kMatchOverTicks) {
            finished = true;
            result.flow = FlowEvent::Finished;
        }
        break;
    }
    return result;
}

std::uint64_t Match::checksum() const {
    std::uint64_t h = round.checksum();
    const auto add = [&h](std::int32_t v) {
        const auto u = static_cast<std::uint32_t>(v);
        for (int j = 0; j < 4; ++j) {
            h ^= (u >> (8 * j)) & 255U;
            h *= 1099511628211ULL;
        }
    };
    add(roundNumber);
    add(wins[0]);
    add(wins[1]);
    add(static_cast<std::int32_t>(phase));
    add(phaseTicks);
    add(static_cast<std::int32_t>(lastRound));
    add(static_cast<std::int32_t>(winner));
    add(finished);
    return h;
}

} // namespace iron::sim
