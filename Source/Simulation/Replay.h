#pragma once

// Replay files (ADR-0026): what a match needs to play again tick for tick.
// The simulation is deterministic, so the inputs are the whole recording; the
// final checksum lets playback prove it reproduced the match.
//
// Layout (little-endian): "IRRP", u16 format version, the header fields,
// then the inputs run-length encoded as (u16 count, u16 p1, u16 p2) runs, then
// the final Match checksum.

#include "Simulation/Round.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace iron::sim {

inline constexpr std::uint16_t kReplayVersion = 1;

struct Replay {
    std::string gameVersion;             // the build that recorded it (informational)
    std::uint64_t contentHash = 0;       // replayContentHash() of the specs and stage width
    std::array<std::string, 2> fighters; // roster ids
    std::string stage;                   // stage id
    std::int32_t stageWidth = kStageWidth;
    std::int64_t recordedAt = 0;              // seconds since the Unix epoch (informational)
    std::vector<std::array<Input, 2>> inputs; // one pair per Match::step, from the first
    std::uint64_t finalChecksum = 0;          // Match::checksum() after the last step
};

[[nodiscard]] std::vector<std::uint8_t> encodeReplay(const Replay& replay);
// Nullopt (and a reason in *error) for anything that isn't a valid replay.
[[nodiscard]] std::optional<Replay> decodeReplay(std::span<const std::uint8_t> bytes, std::string* error = nullptr);

// What makes two recordings of the same inputs agree: both fighters' gameplay
// data and the stage width.
[[nodiscard]] std::uint64_t replayContentHash(const FighterSpec& p1, const FighterSpec& p2, int stageWidth);

} // namespace iron::sim
