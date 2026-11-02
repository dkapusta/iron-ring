#include "Server/Server.h"

#include "Net/Codes.h"
#include "Net/Net.h"
#include "Net/Transport.h"
#include "Simulation/Rollback.h"

#include <enet/enet.h>

#include <algorithm>
#include <cstdio>

namespace iron::server {

struct NetLibraryHolder {
    net::Library library;
};

namespace {

constexpr std::size_t kSparePeers = 8; // over maxPlayers: room to refuse with a reason

constexpr int kMaxFightTicks = 60 * 60 * 30; // half an hour of inputs per side, far beyond any match

std::string addressOf(const ENetPeer* peer) {
    char text[64] = "?";
    enet_address_get_host_ip(&peer->address, text, sizeof text);
    return text;
}

} // namespace

Server::Server(Config config, GameRules rules, Log log)
    : config_(std::move(config)), rules_(std::move(rules)), log_(std::move(log)),
      library_(std::make_unique<NetLibraryHolder>()) {
    if (rules_.fighters.empty() || rules_.stages.empty()) config_.verify = false; // nothing to referee with
}

Server::~Server() {
    if (host_) enet_host_destroy(host_);
}

bool Server::start(std::string* error) {
    if (!library_->library.ok()) {
        if (error) *error = "cannot initialise networking";
        return false;
    }
    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    if (!config_.bind.empty() && enet_address_set_host(&address, config_.bind.c_str()) != 0) {
        if (error) *error = "cannot resolve bind address " + config_.bind;
        return false;
    }
    address.port = config_.port;
    // A few peers over maxPlayers, so the server can tell the extra ones it is full.
    host_ = enet_host_create(&address, static_cast<size_t>(std::max(1, config_.maxPlayers) + kSparePeers),
                             net::kChannelCount, 0, 0);
    if (!host_) {
        if (error) *error = "cannot open UDP port " + std::to_string(config_.port) + " (in use, or not allowed)";
        return false;
    }
    host_->maximumPacketSize = net::kMaxPacketBytes;
    host_->maximumWaitingData = 64 * 1024;
    lastHousekeeping_ = Clock::now();
    log("listening on UDP " + std::to_string(port()) + (config_.verify ? ", refereeing fights" : ", relay only") +
        " (" + std::to_string(rules_.fighters.size()) + " fighters, " + std::to_string(rules_.stages.size()) +
        " stages)");
    return true;
}

std::uint16_t Server::port() const {
    if (!host_) return 0;
    ENetAddress a{};
    if (enet_socket_get_address(host_->socket, &a) != 0) return config_.port;
    return a.port;
}

std::size_t Server::players() const {
    return static_cast<std::size_t>(
        std::count_if(clients_.begin(), clients_.end(), [](const auto& c) { return c.second.welcomed; }));
}

void Server::log(const std::string& line) const {
    if (log_) log_(line);
}

std::string Server::who(const Client& c) const {
    return "#" + std::to_string(c.id) + (c.name.empty() ? "" : " " + c.name);
}

void Server::poll(int timeoutMs) {
    if (!host_) return;
    ENetEvent e;
    auto wait = static_cast<enet_uint32>(std::max(0, timeoutMs));
    while (enet_host_service(host_, &e, wait) > 0) {
        wait = 0;
        switch (e.type) {
        case ENET_EVENT_TYPE_CONNECT: onConnect(e.peer); break;
        case ENET_EVENT_TYPE_DISCONNECT: onDisconnect(e.peer); break;
        case ENET_EVENT_TYPE_RECEIVE: {
            auto message = net::decodeMessage({e.packet->data, e.packet->dataLength});
            enet_packet_destroy(e.packet);
            if (!message) {
                // Not our protocol: drop the connection rather than guess.
                if (const auto it = clients_.find(e.peer); it != clients_.end())
                    log(who(it->second) + ": malformed packet, disconnecting");
                enet_peer_disconnect(e.peer, 0);
                break;
            }
            onMessage(e.peer, std::move(*message));
            break;
        }
        default: break;
        }
    }
    if (Clock::now() - lastHousekeeping_ >= std::chrono::seconds(1)) housekeeping();
    enet_host_flush(host_);
}

void Server::shutdown() {
    if (!host_) return;
    for (auto& [peer, c] : clients_) {
        net::sendTo(peer, net::Refused{net::Refusal::ShuttingDown, {}});
        enet_peer_disconnect_later(peer, 0);
    }
    // Give the goodbyes a moment to leave.
    const auto until = Clock::now() + std::chrono::milliseconds(500);
    ENetEvent e;
    while (!clients_.empty() && Clock::now() < until && enet_host_service(host_, &e, 50) >= 0) {
        if (e.type == ENET_EVENT_TYPE_DISCONNECT) clients_.erase(e.peer);
        if (e.type == ENET_EVENT_TYPE_RECEIVE) enet_packet_destroy(e.packet);
    }
    log("shut down");
    enet_host_destroy(host_);
    host_ = nullptr;
    clients_.clear();
    rooms_.clear();
}

void Server::onConnect(ENetPeer* peer) {
    Client c;
    c.id = nextId_++;
    c.address = addressOf(peer);
    c.connected = Clock::now();
    enet_peer_timeout(peer, 0, 0, 10000);
    log("connect " + c.address + " #" + std::to_string(c.id));
    const bool full = static_cast<int>(clients_.size()) >= config_.maxPlayers;
    auto& added = clients_.emplace(peer, std::move(c)).first->second;
    if (full) {
        log(who(added) + ": server full (" + std::to_string(config_.maxPlayers) + " players)");
        net::sendTo(peer, net::Refused{net::Refusal::Full, {}});
        added.dropping = true;
        enet_peer_disconnect_later(peer, 0);
    }
}

void Server::onDisconnect(ENetPeer* peer) {
    const auto it = clients_.find(peer);
    if (it == clients_.end()) return;
    leaveRoom(peer, it->second, net::EndReason::Disconnected);
    log("disconnect " + who(it->second));
    clients_.erase(it);
}

void Server::onMessage(ENetPeer* peer, net::Message&& message) {
    const auto it = clients_.find(peer);
    if (it == clients_.end()) return;
    Client& c = it->second;
    if (c.dropping) return;
    if (config_.maxMessagesPerSecond > 0) {
        const auto now = Clock::now();
        if (now - c.window >= std::chrono::seconds(1)) {
            c.window = now;
            c.messages = 0;
        }
        if (++c.messages > config_.maxMessagesPerSecond) {
            log(who(c) + ": more than " + std::to_string(config_.maxMessagesPerSecond) +
                " messages a second, disconnecting");
            c.dropping = true;
            enet_peer_disconnect_later(peer, 0);
            return;
        }
    }
    if (!c.welcomed && !std::holds_alternative<net::Hello>(message)) return; // introduce yourself first
    std::visit(
        [&](const auto& m) {
            using T = std::decay_t<decltype(m)>;
            if constexpr (std::is_same_v<T, net::LeaveRoom>)
                leaveRoom(peer, c, net::EndReason::Left);
            else if constexpr (std::is_same_v<T, net::Hello> || std::is_same_v<T, net::CreateRoom> ||
                               std::is_same_v<T, net::JoinRoom> || std::is_same_v<T, net::ListRooms> ||
                               std::is_same_v<T, net::SetOpen> || std::is_same_v<T, net::Pick> ||
                               std::is_same_v<T, net::Rematch> || std::is_same_v<T, net::Hover> ||
                               std::is_same_v<T, net::Inputs> ||
                               std::is_same_v<T, net::Checksum> || std::is_same_v<T, net::Result>)
                handle(peer, c, m);
            // Server-to-client messages arriving here are ignored.
        },
        message);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Hello& m) {
    if (c.welcomed) return;
    const auto refuse = [&](net::Refusal why, std::string detail = {}) {
        log("refused " + c.address + " #" + std::to_string(c.id) + ": " + net::describe(why, detail));
        net::sendTo(peer, net::Refused{why, std::move(detail)});
        enet_peer_disconnect_later(peer, 0);
    };
    if (m.protocol != net::kProtocolVersion) return refuse(net::Refusal::Version, rules_.version);
    if (rules_.dataHash != 0 && m.dataHash != rules_.dataHash) return refuse(net::Refusal::GameData, rules_.version);
    const auto name = net::sanitizeName(m.name);
    if (name.empty()) return refuse(net::Refusal::BadName);
    c.name = name;
    c.dataHash = m.dataHash;
    c.welcomed = true;
    log("hello " + who(c) + " (game " + m.gameVersion.substr(0, 16) + ")");
    net::sendTo(peer, net::Welcome{config_.name, config_.motd, static_cast<std::uint16_t>(players()),
                                   static_cast<std::uint16_t>(rooms_.size())});
}

void Server::handle(ENetPeer* peer, Client& c, const net::CreateRoom& m) {
    leaveRoom(peer, c, net::EndReason::Left);
    if (static_cast<int>(rooms_.size()) >= config_.maxRooms)
        return net::sendTo(peer, net::Failed{net::Problem::ServerFull});
    const auto mine = std::count_if(rooms_.begin(), rooms_.end(), [&](const auto& r) {
        return !r.second.members.empty() && clients_.at(r.second.members[0]).address == c.address;
    });
    if (mine >= config_.roomsPerAddress) return net::sendTo(peer, net::Failed{net::Problem::TooManyRooms});
    std::string code;
    do
        code = net::newRoomCode(random_);
    while (rooms_.count(code));
    Room room;
    room.code = code;
    room.open = m.open;
    room.members.push_back(peer);
    room.created = room.activity = Clock::now();
    c.room = code;
    log("room " + code + " created by " + who(c) + (m.open ? " (open)" : " (private)"));
    broadcastState(rooms_.emplace(code, std::move(room)).first->second);
}

bool Server::tooManyAttempts(const std::string& address) {
    auto& q = failures_[address];
    const auto minuteAgo = Clock::now() - std::chrono::minutes(1);
    while (!q.empty() && q.front() < minuteAgo)
        q.pop_front();
    return static_cast<int>(q.size()) >= config_.joinAttemptsPerMinute;
}

void Server::handle(ENetPeer* peer, Client& c, const net::JoinRoom& m) {
    if (tooManyAttempts(c.address)) return net::sendTo(peer, net::Failed{net::Problem::TooManyAttempts});
    const auto code = net::normalizeCode(m.code);
    const auto it = code ? rooms_.find(*code) : rooms_.end();
    // A room on other game data counts as not found: nothing to fight with.
    const bool found =
        it != rooms_.end() && !it->second.members.empty() && clients_.at(it->second.members[0]).dataHash == c.dataHash;
    if (!found) {
        failures_[c.address].push_back(Clock::now());
        return net::sendTo(peer, net::Failed{net::Problem::RoomNotFound});
    }
    Room& room = it->second;
    if (room.code == c.room) return broadcastState(room); // already here
    if (room.members.size() >= 2) return net::sendTo(peer, net::Failed{net::Problem::RoomFull});
    leaveRoom(peer, c, net::EndReason::Left);
    room.members.push_back(peer);
    room.phase = net::RoomPhase::Selecting;
    room.picks = {};
    room.hover = {};
    room.locked = {};
    room.stagePick.clear();
    room.activity = Clock::now();
    c.room = room.code;
    log("room " + room.code + ": " + who(c) + " joined");
    broadcastState(room);
}

void Server::handle(ENetPeer* peer, Client&, const net::ListRooms&) {
    net::RoomList list;
    const auto now = Clock::now();
    std::vector<const Room*> waiting;
    for (const auto& [code, room] : rooms_)
        if (room.open && room.members.size() == 1 && room.phase == net::RoomPhase::Waiting) waiting.push_back(&room);
    std::sort(waiting.begin(), waiting.end(), [](const Room* a, const Room* b) { return a->created < b->created; });
    for (const Room* room : waiting) {
        if (list.rooms.size() == net::kMaxListedRooms) break;
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - room->created).count();
        list.rooms.push_back({room->code, clients_.at(room->members[0]).name, static_cast<std::uint32_t>(seconds)});
    }
    net::sendTo(peer, list);
}

void Server::handle(ENetPeer* peer, Client& c, const net::SetOpen& m) {
    Room* room = roomOf(c);
    if (!room) return net::sendTo(peer, net::Failed{net::Problem::NotInRoom});
    if (room->members[0] != peer) return; // only the host
    room->open = m.open;
    broadcastState(*room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Pick& m) {
    Room* room = roomOf(c);
    if (!room) return net::sendTo(peer, net::Failed{net::Problem::NotInRoom});
    if (room->members.size() < 2 || room->phase == net::RoomPhase::Fighting || room->phase == net::RoomPhase::Waiting)
        return;
    const bool fighterOk = rules_.fighters.empty() ? !m.fighter.empty() : rules_.fighters.count(m.fighter) == 1;
    const bool stageOk = m.stage.empty() || rules_.stages.empty() || rules_.stages.count(m.stage) == 1;
    if (!fighterOk || !stageOk) return net::sendTo(peer, net::Failed{net::Problem::BadPick});
    const int side = sideOf(*room, peer);
    if (room->phase == net::RoomPhase::AfterMatch) {
        // CHANGE FIGHTERS after a match: a fresh select for both, not a rematch with the old picks.
        room->picks = {};
        room->stagePick.clear();
        room->rematch = {};
        room->hover = {};
        room->locked = {};
    }
    room->phase = net::RoomPhase::Selecting;
    room->picks[static_cast<std::size_t>(side)] = m.fighter;
    if (side == 0) room->stagePick = m.stage;
    room->activity = Clock::now();
    if (!room->picks[0].empty() && !room->picks[1].empty())
        startFight(*room);
    else
        broadcastState(*room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Rematch&) {
    Room* room = roomOf(c);
    if (!room || room->phase != net::RoomPhase::AfterMatch || room->members.size() < 2) return;
    room->rematch[static_cast<std::size_t>(sideOf(*room, peer))] = true;
    room->activity = Clock::now();
    if (room->rematch[0] && room->rematch[1]) startFight(*room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Hover& m) {
    Room* room = roomOf(c);
    if (!room || room->members.size() < 2) return;
    if (room->phase != net::RoomPhase::Selecting && room->phase != net::RoomPhase::AfterMatch) return;
    if (!m.fighter.empty() && !rules_.fighters.empty() && rules_.fighters.count(m.fighter) == 0) return;
    const auto side = static_cast<std::size_t>(sideOf(*room, peer));
    if (room->hover[side] == m.fighter && room->locked[side] == m.locked) return;
    room->hover[side] = m.fighter;
    room->locked[side] = m.locked && side == 0; // only the host goes on to the venue
    room->activity = Clock::now();
    broadcastState(*room);
}

void Server::startFight(Room& room) {
    Fight fight;
    fight.id = ++room.matches;
    fight.fighters = room.picks;
    room.hover = {};
    room.locked = {};
    if (!room.stagePick.empty() || rules_.stages.empty()) {
        fight.stage = room.stagePick;
    } else {
        std::uniform_int_distribution<std::size_t> pick(0, rules_.stages.size() - 1);
        fight.stage = std::next(rules_.stages.begin(), static_cast<std::ptrdiff_t>(pick(random_)))->first;
    }
    const auto width = rules_.stages.find(fight.stage);
    fight.width = width != rules_.stages.end() ? width->second : sim::kStageWidth;
    if (config_.verify) {
        fight.referee =
            std::make_unique<sim::Match>(rules_.fighters.at(fight.fighters[0]), rules_.fighters.at(fight.fighters[1]));
        fight.referee->round.stageWidth = fight.width;
        fight.referee->round.reset();
    }
    room.rematch = {};
    room.phase = net::RoomPhase::Fighting;
    room.activity = Clock::now();
    log("room " + room.code + ": match " + std::to_string(fight.id) + " " + fight.fighters[0] + " vs " +
        fight.fighters[1] + " at " + (fight.stage.empty() ? "?" : fight.stage));
    for (std::size_t side = 0; side < room.members.size(); ++side)
        net::sendTo(room.members[side], net::MatchStart{fight.id, fight.fighters, fight.stage, fight.width,
                                                        static_cast<std::uint8_t>(side)});
    room.fight = std::move(fight);
    broadcastState(room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Inputs& m) {
    Room* room = roomOf(c);
    if (!room || !room->fight || room->members.size() < 2 || m.match != room->fight->id) return;
    const int side = sideOf(*room, peer);
    auto& mine = room->fight->inputs[static_cast<std::size_t>(side)];
    // Keep the contiguous stream; a packet with a gap is dropped (the sender repeats it).
    const auto have = static_cast<std::int64_t>(mine.size());
    const auto end = static_cast<std::int64_t>(m.from) + static_cast<std::int64_t>(m.inputs.size());
    if (m.from <= have && end > have && end <= kMaxFightTicks)
        mine.insert(mine.end(), m.inputs.begin() + (have - m.from), m.inputs.end());
    room->activity = Clock::now();
    net::sendTo(room->members[static_cast<std::size_t>(1 - side)], m); // the relay
    referee(*room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Checksum& m) {
    Room* room = roomOf(c);
    if (!room || !room->fight || m.match != room->fight->id || m.tick < 0 || m.tick >= kMaxFightTicks) return;
    room->fight->reported[static_cast<std::size_t>(sideOf(*room, peer))][m.tick] = m.sum;
    referee(*room);
}

void Server::handle(ENetPeer* peer, Client& c, const net::Result& m) {
    Room* room = roomOf(c);
    if (!room || !room->fight || m.match != room->fight->id || room->fight->referee)
        return; // the referee decides when there is one
    auto& results = room->fight->results;
    results[static_cast<std::size_t>(sideOf(*room, peer))] = m.winner;
    if (!results[0] || !results[1]) return;
    if (*results[0] == *results[1])
        endFight(*room, net::EndReason::Finished, *results[0]);
    else
        endFight(*room, net::EndReason::Desync);
}

void Server::referee(Room& room) {
    Fight& f = *room.fight;
    if (f.referee) {
        const int confirmed = static_cast<int>(std::min(f.inputs[0].size(), f.inputs[1].size()));
        while (f.refereeTick < confirmed && !f.referee->finished) {
            const auto t = static_cast<std::size_t>(f.refereeTick);
            (void)f.referee->step({f.inputs[0][t], f.inputs[1][t]});
            if ((f.refereeTick + 1) % sim::kChecksumInterval == 0) f.sums[f.refereeTick] = f.referee->checksum();
            ++f.refereeTick;
        }
        // Compare what the clients saw with the referee, and forget what was settled.
        for (std::uint8_t side = 0; side < 2; ++side) {
            auto& reported = f.reported[side];
            for (auto it = reported.begin(); it != reported.end() && it->first < f.refereeTick;) {
                const auto mine = f.sums.find(it->first);
                if (mine != f.sums.end() && mine->second != it->second) {
                    log("room " + room.code + ": side " + std::to_string(side + 1) + " out of sync at tick " +
                        std::to_string(it->first));
                    return endFight(room, net::EndReason::Desync, sim::Outcome::Running, side);
                }
                it = reported.erase(it);
            }
        }
        if (f.referee->finished) endFight(room, net::EndReason::Finished, f.referee->winner);
        return;
    }
    // Relay only: the two clients must agree with each other.
    for (auto it = f.reported[0].begin(); it != f.reported[0].end();) {
        const auto other = f.reported[1].find(it->first);
        if (other == f.reported[1].end()) {
            ++it;
            continue;
        }
        if (other->second != it->second) {
            log("room " + room.code + ": clients disagree at tick " + std::to_string(it->first));
            return endFight(room, net::EndReason::Desync);
        }
        f.reported[1].erase(other);
        it = f.reported[0].erase(it);
    }
}

void Server::endFight(Room& room, net::EndReason why, sim::Outcome winner, std::uint8_t culprit) {
    if (!room.fight) return;
    const char* reason = why == net::EndReason::Finished       ? "finished"
                         : why == net::EndReason::Desync       ? "OUT OF SYNC"
                         : why == net::EndReason::Disconnected ? "a player disconnected"
                                                               : "a player left";
    log("room " + room.code + ": match " + std::to_string(room.fight->id) + " ended, " + reason +
        (why == net::EndReason::Finished ? " (winner " + std::to_string(static_cast<int>(winner)) + ")" : ""));
    for (auto* member : room.members)
        net::sendTo(member, net::MatchEnd{why, winner, culprit});
    room.fight.reset();
    room.rematch = {};
    room.phase = room.members.size() == 2 ? net::RoomPhase::AfterMatch : net::RoomPhase::Waiting;
    broadcastState(room);
}

void Server::leaveRoom(ENetPeer* peer, Client& c, net::EndReason why) {
    Room* room = roomOf(c);
    c.room.clear();
    if (!room) return;
    if (room->fight) endFight(*room, why);
    room->members.erase(std::remove(room->members.begin(), room->members.end(), peer), room->members.end());
    if (room->members.empty()) {
        log("room " + room->code + " closed");
        rooms_.erase(room->code);
        return;
    }
    // The one left becomes the host and waits for a new challenger; the code stays.
    room->phase = net::RoomPhase::Waiting;
    room->picks = {};
    room->hover = {};
    room->locked = {};
    room->stagePick.clear();
    room->rematch = {};
    room->activity = Clock::now();
    broadcastState(*room);
}

void Server::broadcastState(Room& room) {
    net::RoomState state;
    state.code = room.code;
    state.open = room.open;
    state.phase = room.phase;
    // Picks count on the select only: after a match they are kept for a rematch,
    // but a player who goes to CHANGE FIGHTERS must not see the old ones as made.
    const bool selecting = room.phase == net::RoomPhase::Selecting;
    for (std::size_t i = 0; i < room.members.size(); ++i) {
        const auto& c = clients_.at(room.members[i]);
        const bool picked = selecting && !room.picks[i].empty();
        state.members.push_back(
            {c.name, picked, picked ? room.picks[i] : std::string(),
             static_cast<std::uint16_t>(std::min<enet_uint32>(room.members[i]->roundTripTime, 9999)), room.hover[i],
             room.locked[i]});
    }
    for (std::size_t i = 0; i < room.members.size(); ++i) {
        state.you = static_cast<std::uint8_t>(i);
        net::sendTo(room.members[i], state);
    }
}

void Server::housekeeping() {
    const auto now = Clock::now();
    lastHousekeeping_ = now;
    for (auto& [peer, c] : clients_)
        if (!c.welcomed && now - c.connected > std::chrono::seconds(config_.helloSeconds))
            enet_peer_disconnect(peer, 0);
    std::vector<std::string> idle;
    for (const auto& [code, room] : rooms_)
        if (now - room.activity > std::chrono::minutes(config_.idleMinutes)) idle.push_back(code);
    for (const auto& code : idle) {
        Room& room = rooms_.at(code);
        log("room " + code + " closed: idle");
        for (auto* member : room.members) {
            net::sendTo(member, net::Failed{net::Problem::RoomClosed});
            clients_.at(member).room.clear();
        }
        rooms_.erase(code);
    }
    for (auto it = failures_.begin(); it != failures_.end();) {
        if (!it->second.empty() && it->second.back() < now - std::chrono::minutes(1)) it->second.clear();
        it = it->second.empty() ? failures_.erase(it) : std::next(it);
    }
}

Server::Room* Server::roomOf(const Client& c) {
    if (c.room.empty()) return nullptr;
    const auto it = rooms_.find(c.room);
    return it == rooms_.end() ? nullptr : &it->second;
}

int Server::sideOf(const Room& room, ENetPeer* peer) const {
    return room.members.size() > 1 && room.members[1] == peer ? 1 : 0;
}

} // namespace iron::server
