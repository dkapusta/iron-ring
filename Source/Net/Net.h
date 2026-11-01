#pragma once

// Online play: constants shared by the player and IronRingServer (ADR-0026,
// docs/design/online.md). Everything in IronRingNet is SDL-free.

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace iron::net {

inline constexpr std::uint16_t kDefaultPort = 7727;
// Bump when any message changes; clients and servers of other versions refuse each other.
inline constexpr std::uint16_t kProtocolVersion = 1;
inline constexpr std::size_t kMaxNameLength = 12;
inline constexpr std::size_t kMaxPacketBytes = 4096;

// ENet channels.
inline constexpr std::uint8_t kChannelControl = 0; // reliable, ordered: lobby, rooms, results
inline constexpr std::uint8_t kChannelInputs =
    1; // unreliable: fight inputs (each packet repeats the unacknowledged ones)
inline constexpr std::size_t kChannelCount = 2;

// A player name as the game can draw it: the font's capitals, digits, '.'
// and '-', at most kMaxNameLength, trimmed. Lower case is raised; anything
// else is dropped. Empty when nothing usable is left.
[[nodiscard]] std::string sanitizeName(std::string_view raw);

// The simulation data two peers must share to fight: every fighter's
// gameplay hash and every stage's world width, by id (order-independent).
[[nodiscard]] std::uint64_t gameDataHash(std::vector<std::pair<std::string, std::uint64_t>> fighters,
                                         std::vector<std::pair<std::string, int>> stages);

} // namespace iron::net
