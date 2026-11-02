#pragma once

// Messages between the game and IronRingServer (docs/design/online.md §4).
// Each is a tagged struct encoded field by field with sim::ByteWriter;
// decodeMessage() rejects anything malformed, so packets from the network
// can't crash either side.

#include "Simulation/Round.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace iron::net {

inline constexpr std::size_t kMaxInputsPerPacket = 128;
inline constexpr std::size_t kMaxListedRooms = 32;

// Why the server refused a connection (Refused::reason).
enum class Refusal : std::uint8_t { Version = 1, GameData = 2, Full = 3, BadName = 4, ShuttingDown = 5 };
// Why a room request failed (Problem::what).
enum class Problem : std::uint8_t {
    RoomNotFound = 1,
    RoomFull = 2,
    TooManyRooms = 3,
    TooManyAttempts = 4,
    NotInRoom = 5,
    ServerFull = 6,
    BadPick = 7,
    RoomClosed = 8,
};
enum class RoomPhase : std::uint8_t { Waiting = 0, Selecting = 1, Fighting = 2, AfterMatch = 3 };
enum class EndReason : std::uint8_t { Finished = 1, Desync = 2, Disconnected = 3, Left = 4 };

// ---- client -> server ----
struct Hello {
    std::uint16_t protocol = 0;
    std::string gameVersion;
    std::uint64_t dataHash = 0; // gameDataHash() of the client's simulation data
    std::string name;
};
struct CreateRoom {
    bool open = false;
};
struct JoinRoom {
    std::string code;
};
struct ListRooms {};
struct LeaveRoom {};
struct SetOpen {
    bool open = false;
};
struct Pick { // the fighter for this player; the host's stage counts ("" = random)
    std::string fighter;
    std::string stage;
};
struct Rematch {}; // after a match: both send it to fight again with the same picks
struct Hover {     // on the select: the fighter under this player's cursor, for the opponent to see
    std::string fighter;
    bool locked = false; // the host has picked and is choosing the venue
};
struct Checksum {  // Match::checksum() after a confirmed tick (sim::Rollback::checksums())
    std::int32_t tick = 0;
    std::uint64_t sum = 0;
    std::uint32_t match = 0; // MatchStart::match: a late message from the last match is ignored
};
struct Result { // the match's outcome as this client saw it; decides only when the server doesn't referee
    sim::Outcome winner = sim::Outcome::Draw;
    std::uint32_t match = 0;
};

// ---- both ways (the server relays it to the other fighter) ----
struct Inputs {
    std::int32_t from = 0;          // tick of inputs[0]
    std::vector<sim::Input> inputs; // the sender's inputs for from, from + 1, ...
    std::int32_t ack = 0;           // ticks of the other side's input the sender has received
    std::uint32_t match = 0;        // MatchStart::match: inputs still in flight from the last match are dropped
};

// ---- server -> client ----
struct Welcome {
    std::string serverName;
    std::string motd;
    std::uint16_t players = 0;
    std::uint16_t rooms = 0;
};
struct Refused {
    Refusal reason = Refusal::Version;
    std::string detail; // e.g. the server's version
};
struct Failed {
    Problem what = Problem::RoomNotFound;
};
struct RoomMember {
    std::string name;
    bool picked = false;
    std::string fighter;    // the pick, once picked (the select shows the opponent's choice)
    std::uint16_t ping = 0; // ms, server <-> that player
    std::string hover;      // Hover: the fighter under their cursor ("" until they move it)
    bool locked = false;    // Hover::locked
};
struct RoomState {
    std::string code;
    bool open = false;
    RoomPhase phase = RoomPhase::Waiting;
    std::uint8_t you = 0;            // your index in members (0 = host, plays side 0)
    std::vector<RoomMember> members; // 1 or 2
};
struct RoomListEntry {
    std::string code;
    std::string host;
    std::uint32_t waitingSeconds = 0;
};
struct RoomList {
    std::vector<RoomListEntry> rooms;
};
struct MatchStart {
    std::uint32_t match = 0; // numbers the room's matches
    std::array<std::string, 2> fighters;
    std::string stage;
    std::int32_t stageWidth = 0;
    std::uint8_t side = 0; // which side you control
};
struct MatchEnd {
    EndReason reason = EndReason::Finished;
    sim::Outcome winner = sim::Outcome::Running; // Finished: the result the server confirmed
    std::uint8_t culprit = 0xFF;                 // Desync: the side that went out of sync, when known
};

using Message = std::variant<Hello, CreateRoom, JoinRoom, ListRooms, LeaveRoom, SetOpen, Pick, Rematch, Hover, Checksum,
                             Result, Inputs, Welcome, Refused, Failed, RoomState, RoomList, MatchStart, MatchEnd>;

[[nodiscard]] std::vector<std::uint8_t> encodeMessage(const Message& message);
[[nodiscard]] std::optional<Message> decodeMessage(std::span<const std::uint8_t> bytes);
// The channel a message travels on (inputs unreliable, everything else reliable).
[[nodiscard]] std::uint8_t channelFor(const Message& message);

// A one-line, player-facing explanation, in the font's capitals.
[[nodiscard]] std::string describe(Refusal reason, const std::string& detail);
[[nodiscard]] std::string describe(Problem what);

} // namespace iron::net
