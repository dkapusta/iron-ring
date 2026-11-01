#include "Game/OnlineSession.h"

#include "Net/Net.h"

#include <SDL3/SDL.h>

namespace iron {

OnlineSession::OnlineSession(std::string gameVersion, std::uint64_t dataHash)
    : gameVersion_(std::move(gameVersion)), dataHash_(dataHash) {}

bool OnlineSession::connect(const net::Invite& invite, const std::string& name) {
    disconnect();
    invite_ = invite;
    name_ = name;
    pendingCode_ = invite.code;
    message_.clear();
    refused_ = false;
    std::string error;
    if (!client_.connect(invite.host, invite.port, &error)) {
        message_ = error;
        return false;
    }
    status_ = Status::Connecting;
    SDL_Log("Online: connecting to %s", net::formatInvite(invite).c_str());
    return true;
}

void OnlineSession::disconnect() {
    if (status_ != Status::Offline) SDL_Log("Online: disconnected");
    client_.disconnect();
    status_ = Status::Offline;
    room_.reset();
    list_.reset();
    start_.reset();
    end_.reset();
    inputs_.clear();
}

void OnlineSession::leaveRoom() {
    if (room_) send(net::LeaveRoom{});
    room_.reset();
    start_.reset();
    end_.reset();
    inputs_.clear();
    wantSelect = false;
    pickSent = false;
}

std::optional<net::MatchStart> OnlineSession::takeMatchStart() {
    auto s = std::move(start_);
    start_.reset();
    return s;
}

void OnlineSession::simulate(int latencyMs, int lossPercent) {
    simLatency_ = latencyMs;
    simLoss_ = lossPercent;
    if (latencyMs > 0 || lossPercent > 0)
        SDL_Log("Online: simulating %d ms more latency and %d%% input loss", latencyMs, lossPercent);
}

std::vector<net::Inputs> OnlineSession::takeInputs() {
    const auto now = SDL_GetTicks();
    for (auto it = held_.begin(); it != held_.end();) {
        if (it->due > now) break; // in arrival order
        inputs_.push_back(std::move(it->inputs));
        it = held_.erase(it);
    }
    std::vector<net::Inputs> out;
    out.swap(inputs_);
    return out;
}

std::string OnlineSession::takeMessage() {
    std::string m;
    m.swap(message_);
    return m;
}

std::string OnlineSession::invite() const {
    if (!room_) return {};
    net::Invite i = invite_;
    i.code = room_->code;
    return net::formatInvite(i);
}

void OnlineSession::poll() {
    if (status_ == Status::Offline) return;
    for (auto& e : client_.poll(0)) {
        switch (e.kind) {
        case net::ClientEvent::Kind::Connected:
            SDL_Log("Online: connected, saying hello as %s", name_.c_str());
            client_.send(net::Hello{net::kProtocolVersion, gameVersion_, dataHash_, name_});
            break;
        case net::ClientEvent::Kind::Disconnected:
            if (message_.empty() && !refused_)
                message_ = status_ == Status::Connecting ? "NO ANSWER FROM SERVER" : "CONNECTION LOST";
            SDL_Log("Online: connection closed (%s)", message_.c_str());
            status_ = Status::Offline;
            room_.reset();
            start_.reset();
            inputs_.clear();
            held_.clear();
            break;
        case net::ClientEvent::Kind::Received: onMessage(std::move(e.message)); break;
        }
    }
}

void OnlineSession::onMessage(net::Message&& message) {
    std::visit(
        [&](auto& m) {
            using T = std::decay_t<decltype(m)>;
            if constexpr (std::is_same_v<T, net::Welcome>) {
                welcome_ = m;
                status_ = Status::Online;
                SDL_Log("Online: welcome to %s (%d players, %d rooms)", m.serverName.c_str(), m.players, m.rooms);
                if (!pendingCode_.empty()) client_.send(net::JoinRoom{pendingCode_});
                pendingCode_.clear();
            } else if constexpr (std::is_same_v<T, net::Refused>) {
                refused_ = true;
                message_ = net::describe(m.reason, m.detail);
                SDL_Log("Online: refused: %s", message_.c_str());
            } else if constexpr (std::is_same_v<T, net::Failed>) {
                message_ = net::describe(m.what);
                if (m.what == net::Problem::RoomClosed) room_.reset();
                SDL_Log("Online: %s", message_.c_str());
            } else if constexpr (std::is_same_v<T, net::RoomState>) {
                if (!room_ || room_->code != m.code) SDL_Log("Online: in room %s", m.code.c_str());
                room_ = std::move(m);
            } else if constexpr (std::is_same_v<T, net::RoomList>) {
                list_ = std::move(m);
            } else if constexpr (std::is_same_v<T, net::MatchStart>) {
                SDL_Log("Online: match %u, %s vs %s at %s, playing side %d", m.match, m.fighters[0].c_str(),
                        m.fighters[1].c_str(), m.stage.c_str(), m.side + 1);
                end_.reset();
                match_ = m.match;
                inputs_.clear();
                held_.clear();
                start_ = std::move(m);
            } else if constexpr (std::is_same_v<T, net::MatchEnd>) {
                SDL_Log("Online: match ended (reason %d, winner %d)", static_cast<int>(m.reason),
                        static_cast<int>(m.winner));
                end_ = m;
            } else if constexpr (std::is_same_v<T, net::Inputs>) {
                if (m.match != match_) return; // still in flight from the last match
                if (simLatency_ == 0 && simLoss_ == 0) {
                    inputs_.push_back(std::move(m));
                    return;
                }
                simRandom_ = simRandom_ * 1664525u + 1013904223u;
                if (static_cast<int>((simRandom_ >> 16) % 100) < simLoss_) return;
                held_.push_back({SDL_GetTicks() + static_cast<std::uint64_t>(simLatency_), std::move(m)});
            }
        },
        message);
}

} // namespace iron
