#include "Net/Codes.h"

#include "Net/Net.h"

#include <algorithm>
#include <cctype>
#include <charconv>

namespace iron::net {
namespace {

std::string_view trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.remove_suffix(1);
    return s;
}

bool hostChar(char c) {
    const auto u = static_cast<unsigned char>(c);
    return std::isalnum(u) || c == '.' || c == '-';
}

} // namespace

std::string newRoomCode(std::random_device& source) {
    std::uniform_int_distribution<std::size_t> pick(0, kCodeAlphabet.size() - 1);
    std::string code;
    for (std::size_t i = 0; i < kCodeLength; ++i)
        code += kCodeAlphabet[pick(source)];
    return code;
}

std::optional<std::string> normalizeCode(std::string_view raw) {
    std::string code;
    for (const char c : trim(raw)) {
        if (c == '-' || c == ' ') continue;
        const char u = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (kCodeAlphabet.find(u) == std::string_view::npos) return std::nullopt;
        code += u;
    }
    if (code.size() != kCodeLength) return std::nullopt;
    return code;
}

std::optional<Invite> parseInvite(std::string_view text) {
    text = trim(text);
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::string_view s = lower;
    if (s.rfind("ironring://", 0) == 0) s.remove_prefix(11);
    while (!s.empty() && s.back() == '/')
        s.remove_suffix(1);

    Invite invite;
    if (const auto slash = s.find('/'); slash != std::string_view::npos) {
        const auto code = normalizeCode(s.substr(slash + 1));
        if (!code) return std::nullopt;
        invite.code = *code;
        s = s.substr(0, slash);
    }
    invite.port = kDefaultPort;
    if (const auto colon = s.find(':'); colon != std::string_view::npos) {
        const auto portText = s.substr(colon + 1);
        unsigned port = 0;
        const auto [end, ec] = std::from_chars(portText.data(), portText.data() + portText.size(), port);
        if (ec != std::errc{} || end != portText.data() + portText.size() || port == 0 || port > 65535)
            return std::nullopt;
        invite.port = static_cast<std::uint16_t>(port);
        s = s.substr(0, colon);
    }
    if (s.empty() || s.size() > 253 || s.front() == '.' || s.front() == '-') return std::nullopt;
    if (!std::all_of(s.begin(), s.end(), hostChar)) return std::nullopt;
    invite.host = std::string(s);
    return invite;
}

std::string formatInvite(const Invite& invite) {
    std::string out;
    for (const char c : invite.host)
        out += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    out += ':' + std::to_string(invite.port);
    if (!invite.code.empty()) out += '/' + invite.code;
    return out;
}

std::string sanitizeName(std::string_view raw) {
    std::string name;
    for (const char c : trim(raw)) {
        const char u = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        const bool ok = (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '.' || u == '-' || u == ' ';
        if (!ok || (u == ' ' && (name.empty() || name.back() == ' '))) continue;
        if (name.size() == kMaxNameLength) break;
        name += u;
    }
    while (!name.empty() && name.back() == ' ')
        name.pop_back();
    return name;
}

std::uint64_t gameDataHash(std::vector<std::pair<std::string, std::uint64_t>> fighters,
                           std::vector<std::pair<std::string, int>> stages) {
    std::sort(fighters.begin(), fighters.end());
    std::sort(stages.begin(), stages.end());
    std::uint64_t h = 14695981039346656037ULL;
    const auto byte = [&h](std::uint8_t b) {
        h ^= b;
        h *= 1099511628211ULL;
    };
    const auto text = [&](const std::string& s) {
        for (const char c : s)
            byte(static_cast<std::uint8_t>(c));
        byte(0);
    };
    const auto u64 = [&](std::uint64_t v) {
        for (int i = 0; i < 8; ++i)
            byte(static_cast<std::uint8_t>(v >> (8 * i)));
    };
    for (const auto& [id, hash] : fighters) {
        text(id);
        u64(hash);
    }
    byte(0xFF);
    for (const auto& [id, width] : stages) {
        text(id);
        u64(static_cast<std::uint64_t>(width));
    }
    return h;
}

} // namespace iron::net
