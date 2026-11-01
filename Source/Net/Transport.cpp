#include "Net/Transport.h"

#include "Net/Net.h"

#include <enet/enet.h>

#include <atomic>
#include <mutex>

namespace iron::net {
namespace {

std::mutex libraryMutex;
int libraryUsers = 0;
bool libraryOk = false;

} // namespace

Library::Library() {
    const std::lock_guard lock(libraryMutex);
    if (libraryUsers++ == 0) libraryOk = enet_initialize() == 0;
    ok_ = libraryOk;
}

Library::~Library() {
    const std::lock_guard lock(libraryMutex);
    if (--libraryUsers == 0 && libraryOk) {
        enet_deinitialize();
        libraryOk = false;
    }
}

void sendTo(ENetPeer* peer, const Message& message) {
    if (!peer) return;
    const auto bytes = encodeMessage(message);
    const auto channel = channelFor(message);
    const enet_uint32 flags = channel == kChannelInputs ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE;
    ENetPacket* packet = enet_packet_create(bytes.data(), bytes.size(), flags);
    if (enet_peer_send(peer, channel, packet) != 0) enet_packet_destroy(packet);
}

Client::Client() = default;

Client::~Client() {
    close();
}

bool Client::connect(const std::string& host, std::uint16_t port, std::string* error) {
    close();
    const auto fail = [&](const char* why) {
        if (error) *error = why;
        close();
        return false;
    };
    if (!library_.ok()) return fail("NETWORK UNAVAILABLE");
    host_ = enet_host_create(nullptr, 1, kChannelCount, 0, 0);
    if (!host_) return fail("NETWORK UNAVAILABLE");
    host_->maximumPacketSize = kMaxPacketBytes;
    ENetAddress address{};
    if (enet_address_set_host(&address, host.c_str()) != 0) return fail("SERVER NOT FOUND");
    address.port = port;
    peer_ = enet_host_connect(host_, &address, kChannelCount, 0);
    if (!peer_) return fail("NETWORK UNAVAILABLE");
    enet_peer_timeout(peer_, 0, 0, 5000);
    return true;
}

void Client::disconnect() {
    if (peer_ && connected_) {
        enet_peer_disconnect(peer_, 0);
        enet_host_flush(host_);
    }
    close();
}

void Client::close() {
    if (peer_ && !connected_) enet_peer_reset(peer_);
    peer_ = nullptr;
    if (host_) enet_host_destroy(host_);
    host_ = nullptr;
    connected_ = false;
}

void Client::send(const Message& message) {
    if (connected_) sendTo(peer_, message);
}

std::vector<ClientEvent> Client::poll(int timeoutMs) {
    std::vector<ClientEvent> events;
    if (!host_) return events;
    ENetEvent e;
    int wait = timeoutMs;
    while (host_ && enet_host_service(host_, &e, static_cast<enet_uint32>(wait)) > 0) {
        wait = 0;
        switch (e.type) {
        case ENET_EVENT_TYPE_CONNECT:
            connected_ = true;
            events.push_back({ClientEvent::Kind::Connected});
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            connected_ = false;
            peer_ = nullptr;
            events.push_back({ClientEvent::Kind::Disconnected});
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            if (auto m = decodeMessage({e.packet->data, e.packet->dataLength}))
                events.push_back({ClientEvent::Kind::Received, std::move(*m)});
            enet_packet_destroy(e.packet);
            break;
        default: break;
        }
    }
    if (host_) enet_host_flush(host_);
    return events;
}

int Client::pingMs() const {
    return peer_ ? static_cast<int>(peer_->roundTripTime) : 0;
}

} // namespace iron::net
