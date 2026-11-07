#pragma once

// The Iron Ring Dedicated Server core (ADR-0026, docs/design/online.md §3):
// lobby, rooms with codes, the input relay and the referee. A plain class
// with poll(), so the tests run it in-process and HOST ON THIS PC (M4.5) can
// embed it; IronRingServer (main.cpp) wraps it in a console program.

#include "Net/Protocol.h"
#include "Simulation/Match.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

struct _ENetHost;
struct _ENetPeer;

namespace iron::server {

struct Config {
    std::uint16_t port = 7727; // 0: any free port (tests)
    std::string bind;          // empty: every interface
    std::string name = "IRON RING SERVER";
    std::string motd;
    int maxPlayers = 256;
    int maxRooms = 128;
    int roomsPerAddress = 3;
    int idleMinutes = 30;
    bool verify = true;            // referee fights with the game's simulation
    int helloSeconds = 10;         // a connection that doesn't introduce itself is dropped
    int joinAttemptsPerMinute = 5; // wrong room codes per address
    // A game sends about 130 messages a second in a fight; a connection that
    // sends far more is broken or hostile and is dropped. 0: no limit (tests).
    int maxMessagesPerSecond = 400;
};

// What the server knows of the game: what the referee needs.
struct GameRules {
    std::string version;        // the game version it was built from (shown on refusals)
    std::uint64_t dataHash = 0; // net::gameDataHash(); 0 = unknown (accept any, pair equals)
    std::map<std::string, sim::FighterSpec> fighters;
    std::map<std::string, int> stages; // id -> world width
};

class Server {
  public:
    using Log = std::function<void(const std::string&)>;
    using Clock = std::chrono::steady_clock;

    Server(Config config, GameRules rules, Log log);
    ~Server();
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool start(std::string* error = nullptr);
    // Waits up to timeoutMs for traffic, handles it, then housekeeping.
    void poll(int timeoutMs);
    // Tells every player the server is going away, then closes.
    void shutdown();

    [[nodiscard]] std::uint16_t port() const;
    [[nodiscard]] std::size_t players() const;
    [[nodiscard]] std::size_t rooms() const { return rooms_.size(); }
    [[nodiscard]] const Config& config() const { return config_; }

  private:
    struct Client {
        std::uint32_t id = 0;
        std::string address;
        std::string name;
        std::uint64_t dataHash = 0;
        bool welcomed = false;
        Clock::time_point connected;
        std::string room;         // code, or empty
        Clock::time_point window; // maxMessagesPerSecond: the current one-second window
        int messages = 0;
        bool dropping = false; // disconnecting: the rest of its traffic is ignored
    };
    struct Fight {
        std::uint32_t id = 0;
        std::array<std::string, 2> fighters;
        std::string stage;
        int width = 0;
        std::unique_ptr<sim::Match> referee; // verify=1
        int refereeTick = 0;
        std::array<std::vector<sim::Input>, 2> inputs;
        std::map<int, std::uint64_t> sums; // the referee's, by tick
        std::array<std::map<int, std::uint64_t>, 2> reported;
        std::array<std::optional<sim::Outcome>, 2> results; // verify=0
    };
    struct Room {
        std::string code;
        bool open = false;
        std::vector<_ENetPeer*> members; // [0] is the host and plays side 0
        net::RoomPhase phase = net::RoomPhase::Waiting;
        std::array<std::string, 2> picks;
        std::array<std::string, 2> hover; // Hover: the select cursors, shown to the opponent
        std::array<bool, 2> locked{};
        std::string stagePick;
        std::array<bool, 2> rematch{};
        std::uint32_t matches = 0;
        std::optional<Fight> fight;
        // The match the referee just finished: its last inputs are still relayed, because a client
        // confirms the result only once it has the other side's inputs up to the end, and the
        // referee can get there first.
        std::uint32_t finishedMatch = 0;
        Clock::time_point created, activity;
    };

    void onConnect(_ENetPeer* peer);
    void onDisconnect(_ENetPeer* peer);
    void onMessage(_ENetPeer* peer, net::Message&& message);
    void handle(_ENetPeer* peer, Client& c, const net::Hello& m);
    void handle(_ENetPeer* peer, Client& c, const net::CreateRoom& m);
    void handle(_ENetPeer* peer, Client& c, const net::JoinRoom& m);
    void handle(_ENetPeer* peer, Client& c, const net::ListRooms& m);
    void handle(_ENetPeer* peer, Client& c, const net::SetOpen& m);
    void handle(_ENetPeer* peer, Client& c, const net::Pick& m);
    void handle(_ENetPeer* peer, Client& c, const net::Rematch& m);
    void handle(_ENetPeer* peer, Client& c, const net::Hover& m);
    void handle(_ENetPeer* peer, Client& c, const net::Inputs& m);
    void handle(_ENetPeer* peer, Client& c, const net::Checksum& m);
    void handle(_ENetPeer* peer, Client& c, const net::Result& m);

    void leaveRoom(_ENetPeer* peer, Client& c, net::EndReason why);
    void startFight(Room& room);
    void endFight(Room& room, net::EndReason why, sim::Outcome winner = sim::Outcome::Running,
                  std::uint8_t culprit = 0xFF);
    void referee(Room& room);
    void broadcastState(Room& room);
    void housekeeping();
    [[nodiscard]] Room* roomOf(const Client& c);
    [[nodiscard]] int sideOf(const Room& room, _ENetPeer* peer) const;
    [[nodiscard]] bool tooManyAttempts(const std::string& address);
    void log(const std::string& line) const;
    [[nodiscard]] std::string who(const Client& c) const;

    Config config_;
    GameRules rules_;
    Log log_;
    std::unique_ptr<struct NetLibraryHolder> library_;
    _ENetHost* host_ = nullptr;
    std::unordered_map<_ENetPeer*, Client> clients_;
    std::map<std::string, Room> rooms_;
    std::unordered_map<std::string, std::deque<Clock::time_point>> failures_;
    std::random_device random_;
    std::uint32_t nextId_ = 1;
    Clock::time_point lastHousekeeping_;
};

} // namespace iron::server
