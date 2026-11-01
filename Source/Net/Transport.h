#pragma once

// The ENet side of IronRingNet: library lifetime, and the client end of a
// connection to IronRingServer. ENet types stay out of this header.

#include "Net/Protocol.h"

#include <cstdint>
#include <string>
#include <vector>

struct _ENetHost;
struct _ENetPeer;

namespace iron::net {

// enet_initialize / enet_deinitialize, reference-counted: hold one while
// any socket is open.
class Library {
  public:
    Library();
    ~Library();
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;
    [[nodiscard]] bool ok() const { return ok_; }

  private:
    bool ok_ = false;
};

struct ClientEvent {
    enum class Kind { Connected, Disconnected, Received };
    Kind kind = Kind::Received;
    Message message{ListRooms{}}; // for Received
};

// One connection to a server. connect() returns at once (a name lookup may
// block briefly); the outcome arrives from poll() as Connected or
// Disconnected (a timeout after about five seconds).
class Client {
  public:
    Client();
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    // False with *error for a name that doesn't resolve or a socket failure.
    bool connect(const std::string& host, std::uint16_t port, std::string* error = nullptr);
    void disconnect(); // polite: the server hears it; then the socket closes
    void send(const Message& message);
    std::vector<ClientEvent> poll(int timeoutMs = 0);

    [[nodiscard]] bool connected() const { return connected_; }
    [[nodiscard]] int pingMs() const;

  private:
    void close();
    Library library_;
    _ENetHost* host_ = nullptr;
    _ENetPeer* peer_ = nullptr;
    bool connected_ = false;
};

// Internal to IronRingNet and IronRingServer: send one message to a peer.
void sendTo(_ENetPeer* peer, const Message& message);

} // namespace iron::net
