#pragma once

// Training-mode dummy: what the opponent does while the player practises.
// Pure functions of the round state, like cpuInput, so training stays
// deterministic and replayable.

#include "Simulation/Round.h"

#include <array>
#include <optional>
#include <string_view>

namespace iron::sim {

enum class Dummy : std::int32_t { Stand = 0, Crouch, Jump, Guard, GuardAll, Cpu };
inline constexpr std::array<Dummy, 6> kDummyModes{Dummy::Stand, Dummy::Crouch, Dummy::Jump,
                                                  Dummy::Guard, Dummy::GuardAll, Dummy::Cpu};

[[nodiscard]] std::string_view dummyName(Dummy mode);
[[nodiscard]] Dummy nextDummy(Dummy mode);
// "guard-all" / "GUARD ALL" style names, case-insensitive; nullopt if unknown.
[[nodiscard]] std::optional<Dummy> parseDummy(std::string_view name);

// Input for fighters[side]:
// - Stand / Crouch / Jump: hold neutral, down, or jump repeatedly;
// - Guard: standing guard (lows hit);
// - GuardAll: guard, crouching against lows (throws still work: guard doesn't stop them);
// - Cpu: the regular CPU opponent.
// Every mode techs throws half the time, like the CPU.
[[nodiscard]] Input dummyInput(const Round& round, int side, Dummy mode);

} // namespace iron::sim
