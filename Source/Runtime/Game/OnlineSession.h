#pragma once

// The connection to an IronRingServer, for as long as the player is online
// (docs/design/online.md §2). Application polls it every frame, so traffic
// flows whichever scene is on top; the online scenes read its state and send
// through it. One per game, in Services.

#include "Net/Codes.h"
#include "Net/Protocol.h"
#include "Net/Transport.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace iron {

class OnlineSession {
  public:
    enum class Status { Offline, Connecting, Online };

    OnlineSession(std::string gameVersion, std::uint64_t dataHash);

    // Connects to the invite's server; once welcomed, joins its room if it names one.
    // False (with error()) when the address can't even be tried.
    bool connect(const net::Invite& invite, const std::string& name);
    void disconnect();
    void poll(); // once per frame

    void send(const net::Message& message) { client_.send(message); }
    void leaveRoom();                                              // also forgets the room here at once
    [[nodiscard]] std::optional<net::MatchStart> takeMatchStart(); // a fight to start, once
    [[nodiscard]] bool hasMatchStart() const { return start_.has_value(); }
    [[nodiscard]] std::vector<net::Inputs> takeInputs(); // the opponent's, since the last call

    [[nodiscard]] Status status() const { return status_; }
    [[nodiscard]] bool online() const { return status_ == Status::Online; }
    [[nodiscard]] const net::Invite& server() const { return invite_; }
    [[nodiscard]] const net::Welcome& welcome() const { return welcome_; }
    [[nodiscard]] const std::optional<net::RoomState>& room() const { return room_; }
    [[nodiscard]] bool inRoom() const { return room_.has_value(); }
    [[nodiscard]] int side() const { return room_ ? room_->you : 0; }
    [[nodiscard]] bool hosting() const { return room_ && room_->you == 0; }
    [[nodiscard]] bool hasOpponent() const { return room_ && room_->members.size() == 2; }
    [[nodiscard]] const std::optional<net::RoomList>& roomList() const { return list_; }
    void clearRoomList() { list_.reset(); }
    [[nodiscard]] const std::optional<net::MatchEnd>& matchEnd() const { return end_; }
    void clearMatchEnd() { end_.reset(); }
    [[nodiscard]] int pingMs() const { return client_.pingMs() + simLatency_; }
    // -netsim: the opponent's inputs arrive this much later, and this share of them never does.
    void simulate(int latencyMs, int lossPercent);
    // The invite for the current room ("SERVER:PORT/CODE"), as COPY INVITE puts it.
    [[nodiscard]] std::string invite() const;

    // The latest problem in player-facing words ("ROOM NOT FOUND"), once.
    [[nodiscard]] std::string takeMessage();
    // After a match: the player chose CHANGE FIGHTERS (the room opens the select).
    bool wantSelect = false;
    // This player's Pick went to the server (the select waits for the opponent).
    bool pickSent = false;

  private:
    void onMessage(net::Message&& message);

    std::string gameVersion_;
    std::uint64_t dataHash_;
    net::Client client_;
    Status status_ = Status::Offline;
    net::Invite invite_;
    std::string name_;
    std::string pendingCode_;
    net::Welcome welcome_;
    std::optional<net::RoomState> room_;
    std::optional<net::RoomList> list_;
    std::optional<net::MatchStart> start_;
    std::uint32_t match_ = 0; // the latest MatchStart's
    std::optional<net::MatchEnd> end_;
    std::vector<net::Inputs> inputs_;
    struct Held {
        std::uint64_t due;
        net::Inputs inputs;
    };
    std::vector<Held> held_; // -netsim
    int simLatency_ = 0, simLoss_ = 0;
    std::uint32_t simRandom_ = 0x2545F491u;
    std::string message_;
    bool refused_ = false;
};

} // namespace iron
