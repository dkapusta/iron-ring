#pragma once

// Room codes and invites (docs/design/online.md §1, §3.4).

#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <string_view>

namespace iron::net {

// 31 characters with nothing to confuse (no 0 O 1 I L); six of them make
// about 887 million codes.
inline constexpr std::string_view kCodeAlphabet = "23456789ABCDEFGHJKMNPQRSTUVWXYZ";
inline constexpr std::size_t kCodeLength = 6;

// A fresh code from a cryptographic source (std::random_device).
[[nodiscard]] std::string newRoomCode(std::random_device& source);
// "k7qx-2m", " K7QX2M " -> "K7QX2M"; nullopt for anything that can't be a code.
[[nodiscard]] std::optional<std::string> normalizeCode(std::string_view raw);

// "SERVER:PORT/CODE": what COPY INVITE puts on the clipboard. Accepted on
// paste: a host or IPv4 address, an optional :port (default 7727), an
// optional /code, an optional "ironring://" in front, any case, surrounding
// whitespace. IPv6 is not supported (the font has no brackets).
struct Invite {
    std::string host; // as typed, lower-cased
    std::uint16_t port = 0;
    std::string code; // normalized, or empty: just a server
};
[[nodiscard]] std::optional<Invite> parseInvite(std::string_view text);
[[nodiscard]] std::string formatInvite(const Invite& invite); // upper case, the port always shown

} // namespace iron::net
