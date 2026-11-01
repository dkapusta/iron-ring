#pragma once

// Rollback netplay session for one peer (ADR-0026, docs/design/online.md §4.3).
//
// The peer controls one side. Its own inputs are scheduled `delay` ticks
// ahead; the other side's are predicted (the last one received repeats) until
// they arrive. When an input arrives that differs from the prediction, the
// session restores the snapshot before that tick and re-simulates up to the
// present. It never predicts more than `maxRollback` ticks: past that,
// advance() stalls until the peer catches up.
//
// No sockets, no clock: the caller moves inputs between peers (M4.4 over the
// server relay; the tests over a simulated network). Each frame:
//
//     if (session.canAdvance()) { session.addLocalInput(sample()); session.advance(); }
//
// so the local schedule stays exactly `delay` ticks ahead, also while stalled.

#include "Simulation/Match.h"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace iron::sim {

inline constexpr int kMaxRollback = 8;
inline constexpr int kChecksumInterval = 30; // confirmed checksums every half second, for desync detection

class Rollback {
  public:
    // `start` is the match at tick 0 (its specs must outlive the session).
    Rollback(const Match& start, int side, int delay, int maxRollback = kMaxRollback);

    // This peer's input for its next scheduled tick, localTicks().
    void addLocalInput(Input input);
    // The other side's input for a tick. Duplicates and repeats are ignored.
    void addRemoteInput(int tick, Input input);

    // Whether this frame may run a tick (false: too far ahead of the peer, so wait).
    [[nodiscard]] bool canAdvance() const;

    struct TickStep {
        int tick = 0;
        MatchStep step;
        bool resimulated = false; // a re-run of a tick already shown with predicted input
    };
    struct Advance {
        bool advanced = false;
        int rolledBack = 0;          // ticks re-simulated this call
        std::vector<TickStep> steps; // every tick simulated this call, in order (re-runs first)
    };
    // Re-simulates any mispredicted ticks, then simulates the next one (which
    // needs its local input: add it first).
    Advance advance();
    // Re-simulates any mispredicted ticks without running a new one: after the
    // match ends, while the last remote inputs come in to confirm the result.
    Advance settle();

    [[nodiscard]] const Match& state() const { return match_; }
    [[nodiscard]] int side() const { return side_; }
    [[nodiscard]] int delay() const { return delay_; }
    [[nodiscard]] int tick() const { return tick_; } // ticks simulated (the next one to run)
    [[nodiscard]] int localTicks() const { return static_cast<int>(local_.size()); } // local inputs scheduled
    [[nodiscard]] int remoteTicks() const { return remoteNext_; } // remote inputs received, contiguously
    [[nodiscard]] int confirmedTicks() const { return std::min(tick_, std::min(localTicks(), remoteNext_)); }
    // Ticks simulated on predicted input: how far this peer runs ahead of what it knows.
    [[nodiscard]] int predicted() const { return tick_ - std::min(tick_, remoteNext_); }

    // Local inputs for ticks [from, localTicks()), to send (with redundancy) to the peer.
    [[nodiscard]] std::vector<Input> localInputsFrom(int from) const;
    [[nodiscard]] Input localInput(int tick) const { return local_[static_cast<std::size_t>(tick)]; }
    [[nodiscard]] std::optional<Input> remoteInput(int tick) const;

    // Match::checksum() after tick t, once every tick up to t is confirmed;
    // recorded where (t + 1) is a multiple of kChecksumInterval.
    [[nodiscard]] std::optional<std::uint64_t> checksumAfter(int t) const;
    [[nodiscard]] const std::vector<std::pair<int, std::uint64_t>>& checksums() const { return checksums_; }

    // Totals, for the network HUD and tests.
    [[nodiscard]] int rollbacks() const { return rollbacks_; }
    [[nodiscard]] int deepestRollback() const { return deepest_; }

  private:
    [[nodiscard]] Input remoteFor(int t) const; // the known input, or the prediction
    void correct(Advance& result);              // re-runs from the earliest mispredicted tick
    MatchStep simulate(int t);                  // one tick from the current match_ (which is the state before t)
    void recordConfirmed();
    Match& snapshot(int t) { return snapshots_[static_cast<std::size_t>(t) % snapshots_.size()]; }

    Match match_;
    int side_, delay_, maxRollback_;
    int tick_ = 0;
    std::vector<Input> local_;
    std::vector<Input> remote_;
    std::vector<bool> remoteKnown_;
    int remoteNext_ = 0;            // first tick whose remote input is unknown
    std::vector<Input> usedRemote_; // the remote input each simulated tick used
    int rollbackFrom_ = -1;         // earliest mispredicted tick, or -1
    std::vector<Match> snapshots_;  // ring: the state before tick t at t % size
    std::vector<std::pair<int, std::uint64_t>> checksums_;
    int recorded_ = 0; // ticks below this have had their checksum considered
    int rollbacks_ = 0, deepest_ = 0;
};

} // namespace iron::sim
