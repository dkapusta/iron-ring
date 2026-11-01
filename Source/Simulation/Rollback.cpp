#include "Simulation/Rollback.h"

#include <algorithm>

namespace iron::sim {

Rollback::Rollback(const Match& start, int side, int delay, int maxRollback)
    : match_(start), side_(side == 0 ? 0 : 1), delay_(std::max(0, delay)), maxRollback_(std::max(1, maxRollback)),
      snapshots_(static_cast<std::size_t>(maxRollback_ + 2), start) {
    // The first `delay` ticks have no local input yet: they are neutral, and
    // the peer learns that the same way as every other input.
    local_.assign(static_cast<std::size_t>(delay_), Input{0});
}

void Rollback::addLocalInput(Input input) {
    local_.push_back(static_cast<Input>(input & kInputMask));
}

void Rollback::addRemoteInput(int tick, Input input) {
    if (tick < 0) return;
    input = static_cast<Input>(input & kInputMask);
    const auto t = static_cast<std::size_t>(tick);
    if (t < remoteKnown_.size() && remoteKnown_[t]) return; // already known
    if (t >= remote_.size()) {
        remote_.resize(t + 1, 0);
        remoteKnown_.resize(t + 1, false);
    }
    remote_[t] = input;
    remoteKnown_[t] = true;
    while (static_cast<std::size_t>(remoteNext_) < remoteKnown_.size() &&
           remoteKnown_[static_cast<std::size_t>(remoteNext_)])
        ++remoteNext_;
    // Already simulated on a guess? Then from here on the past was wrong.
    if (tick < tick_ && usedRemote_[t] != input)
        rollbackFrom_ = rollbackFrom_ < 0 ? tick : std::min(rollbackFrom_, tick);
}

std::optional<Input> Rollback::remoteInput(int tick) const {
    const auto t = static_cast<std::size_t>(tick);
    if (tick < 0 || t >= remoteKnown_.size() || !remoteKnown_[t]) return std::nullopt;
    return remote_[t];
}

Input Rollback::remoteFor(int t) const {
    if (const auto known = remoteInput(t)) return *known;
    // Predict: the latest input received at or before t repeats (players hold buttons).
    for (int k = std::min(t, static_cast<int>(remoteKnown_.size()) - 1); k >= 0; --k)
        if (remoteKnown_[static_cast<std::size_t>(k)]) return remote_[static_cast<std::size_t>(k)];
    return 0;
}

bool Rollback::canAdvance() const {
    return tick_ - remoteNext_ < maxRollback_;
}

MatchStep Rollback::simulate(int t) {
    snapshot(t) = match_;
    const Input remote = remoteFor(t);
    const auto ut = static_cast<std::size_t>(t);
    if (usedRemote_.size() <= ut) usedRemote_.resize(ut + 1, 0);
    usedRemote_[ut] = remote;
    std::array<Input, 2> inputs{};
    inputs[static_cast<std::size_t>(side_)] = local_[ut];
    inputs[static_cast<std::size_t>(1 - side_)] = remote;
    return match_.step(inputs);
}

void Rollback::recordConfirmed() {
    // Every tick below confirmedTicks() ran on known inputs from a known state,
    // and any rollback has been done: its result is final. The state after t
    // is the snapshot before t + 1 (or the live match for the newest tick).
    const int confirmed = confirmedTicks();
    for (int t = recorded_; t < confirmed; ++t)
        if ((t + 1) % kChecksumInterval == 0)
            checksums_.emplace_back(t, t + 1 == tick_ ? match_.checksum() : snapshot(t + 1).checksum());
    recorded_ = std::max(recorded_, confirmed);
}

Rollback::Advance Rollback::advance() {
    Advance result;
    if (!canAdvance() || tick_ >= localTicks()) return result;
    if (rollbackFrom_ >= 0) {
        const int from = rollbackFrom_;
        rollbackFrom_ = -1;
        match_ = snapshot(from);
        for (int t = from; t < tick_; ++t)
            result.steps.push_back({t, simulate(t), true});
        result.rolledBack = tick_ - from;
        ++rollbacks_;
        deepest_ = std::max(deepest_, result.rolledBack);
    }
    result.steps.push_back({tick_, simulate(tick_), false});
    ++tick_;
    result.advanced = true;
    recordConfirmed();
    return result;
}

std::vector<Input> Rollback::localInputsFrom(int from) const {
    from = std::clamp(from, 0, localTicks());
    return {local_.begin() + from, local_.end()};
}

std::optional<std::uint64_t> Rollback::checksumAfter(int t) const {
    for (const auto& [tick, sum] : checksums_)
        if (tick == t) return sum;
    return std::nullopt;
}

} // namespace iron::sim
