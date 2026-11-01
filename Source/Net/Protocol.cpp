#include "Net/Protocol.h"

#include "Net/Net.h"
#include "Simulation/Serialize.h"

#include <type_traits>

namespace iron::net {
namespace {

using sim::ByteReader;
using sim::ByteWriter;

// Wire tags: the index in Message, fixed by this table so reordering the
// variant can't silently change the protocol.
template <typename T> constexpr std::uint8_t kTag = 0;
template <> constexpr std::uint8_t kTag<Hello> = 1;
template <> constexpr std::uint8_t kTag<CreateRoom> = 2;
template <> constexpr std::uint8_t kTag<JoinRoom> = 3;
template <> constexpr std::uint8_t kTag<ListRooms> = 4;
template <> constexpr std::uint8_t kTag<LeaveRoom> = 5;
template <> constexpr std::uint8_t kTag<SetOpen> = 6;
template <> constexpr std::uint8_t kTag<Pick> = 7;
template <> constexpr std::uint8_t kTag<Rematch> = 8;
template <> constexpr std::uint8_t kTag<Checksum> = 9;
template <> constexpr std::uint8_t kTag<Inputs> = 10;
template <> constexpr std::uint8_t kTag<Result> = 11;
template <> constexpr std::uint8_t kTag<Welcome> = 20;
template <> constexpr std::uint8_t kTag<Refused> = 21;
template <> constexpr std::uint8_t kTag<Failed> = 22;
template <> constexpr std::uint8_t kTag<RoomState> = 23;
template <> constexpr std::uint8_t kTag<RoomList> = 24;
template <> constexpr std::uint8_t kTag<MatchStart> = 25;
template <> constexpr std::uint8_t kTag<MatchEnd> = 26;

constexpr std::size_t kMaxString = 64;

void put(ByteWriter& w, const Hello& m) {
    w.u16(m.protocol);
    w.str(m.gameVersion);
    w.u64(m.dataHash);
    w.str(m.name);
}
void put(ByteWriter& w, const CreateRoom& m) {
    w.boolean(m.open);
}
void put(ByteWriter& w, const JoinRoom& m) {
    w.str(m.code);
}
void put(ByteWriter&, const ListRooms&) {}
void put(ByteWriter&, const LeaveRoom&) {}
void put(ByteWriter& w, const SetOpen& m) {
    w.boolean(m.open);
}
void put(ByteWriter& w, const Pick& m) {
    w.str(m.fighter);
    w.str(m.stage);
}
void put(ByteWriter&, const Rematch&) {}
void put(ByteWriter& w, const Checksum& m) {
    w.i32(m.tick);
    w.u64(m.sum);
}
void put(ByteWriter& w, const Result& m) {
    w.i32(static_cast<std::int32_t>(m.winner));
}
void put(ByteWriter& w, const Inputs& m) {
    w.i32(m.from);
    w.u16(static_cast<std::uint16_t>(m.inputs.size()));
    for (const auto in : m.inputs)
        w.u16(in);
    w.i32(m.ack);
}
void put(ByteWriter& w, const Welcome& m) {
    w.str(m.serverName);
    w.str(m.motd);
    w.u16(m.players);
    w.u16(m.rooms);
}
void put(ByteWriter& w, const Refused& m) {
    w.u8(static_cast<std::uint8_t>(m.reason));
    w.str(m.detail);
}
void put(ByteWriter& w, const Failed& m) {
    w.u8(static_cast<std::uint8_t>(m.what));
}
void put(ByteWriter& w, const RoomState& m) {
    w.str(m.code);
    w.boolean(m.open);
    w.u8(static_cast<std::uint8_t>(m.phase));
    w.u8(m.you);
    w.u8(static_cast<std::uint8_t>(m.members.size()));
    for (const auto& p : m.members) {
        w.str(p.name);
        w.boolean(p.picked);
        w.u16(p.ping);
    }
}
void put(ByteWriter& w, const RoomList& m) {
    w.u8(static_cast<std::uint8_t>(m.rooms.size()));
    for (const auto& r : m.rooms) {
        w.str(r.code);
        w.str(r.host);
        w.u32(r.waitingSeconds);
    }
}
void put(ByteWriter& w, const MatchStart& m) {
    w.u32(m.match);
    w.str(m.fighters[0]);
    w.str(m.fighters[1]);
    w.str(m.stage);
    w.i32(m.stageWidth);
    w.u8(m.side);
}
void put(ByteWriter& w, const MatchEnd& m) {
    w.u8(static_cast<std::uint8_t>(m.reason));
    w.i32(static_cast<std::int32_t>(m.winner));
    w.u8(m.culprit);
}

std::string text(ByteReader& r) {
    auto s = r.str();
    if (s.size() > kMaxString) r.fail();
    return s;
}

template <typename E> E enumIn(ByteReader& r, int lo, int hi) {
    const int v = r.u8();
    if (v < lo || v > hi) r.fail();
    return static_cast<E>(v);
}

std::optional<Message> get(std::uint8_t tag, ByteReader& r) {
    switch (tag) {
    case kTag<Hello>: {
        Hello m;
        m.protocol = r.u16();
        m.gameVersion = text(r);
        m.dataHash = r.u64();
        m.name = text(r);
        return m;
    }
    case kTag<CreateRoom>: return CreateRoom{r.boolean()};
    case kTag<JoinRoom>: return JoinRoom{text(r)};
    case kTag<ListRooms>: return ListRooms{};
    case kTag<LeaveRoom>: return LeaveRoom{};
    case kTag<SetOpen>: return SetOpen{r.boolean()};
    case kTag<Pick>: {
        Pick m;
        m.fighter = text(r);
        m.stage = text(r);
        return m;
    }
    case kTag<Rematch>: return Rematch{};
    case kTag<Checksum>: {
        Checksum m;
        m.tick = r.i32();
        m.sum = r.u64();
        return m;
    }
    case kTag<Result>: {
        const auto v = r.i32();
        if (v < 1 || v > 3) r.fail();
        return Result{static_cast<sim::Outcome>(v)};
    }
    case kTag<Inputs>: {
        Inputs m;
        m.from = r.i32();
        const auto n = r.u16();
        if (n > kMaxInputsPerPacket || m.from < 0) r.fail();
        for (std::uint16_t i = 0; i < n && r.ok(); ++i)
            m.inputs.push_back(static_cast<sim::Input>(r.u16() & sim::kInputMask));
        m.ack = r.i32();
        if (m.ack < 0) r.fail();
        return m;
    }
    case kTag<Welcome>: {
        Welcome m;
        m.serverName = text(r);
        m.motd = text(r);
        m.players = r.u16();
        m.rooms = r.u16();
        return m;
    }
    case kTag<Refused>: {
        Refused m;
        m.reason = enumIn<Refusal>(r, 1, 5);
        m.detail = text(r);
        return m;
    }
    case kTag<Failed>: return Failed{enumIn<Problem>(r, 1, 8)};
    case kTag<RoomState>: {
        RoomState m;
        m.code = text(r);
        m.open = r.boolean();
        m.phase = enumIn<RoomPhase>(r, 0, 3);
        m.you = r.u8();
        const auto n = r.u8();
        if (n < 1 || n > 2 || m.you >= n) r.fail();
        for (int i = 0; i < n && r.ok(); ++i) {
            RoomMember p;
            p.name = text(r);
            p.picked = r.boolean();
            p.ping = r.u16();
            m.members.push_back(p);
        }
        return m;
    }
    case kTag<RoomList>: {
        RoomList m;
        const auto n = r.u8();
        if (n > kMaxListedRooms) r.fail();
        for (int i = 0; i < n && r.ok(); ++i) {
            RoomListEntry e;
            e.code = text(r);
            e.host = text(r);
            e.waitingSeconds = r.u32();
            m.rooms.push_back(e);
        }
        return m;
    }
    case kTag<MatchStart>: {
        MatchStart m;
        m.match = r.u32();
        m.fighters[0] = text(r);
        m.fighters[1] = text(r);
        m.stage = text(r);
        m.stageWidth = r.i32();
        m.side = r.u8();
        if (m.side > 1) r.fail();
        return m;
    }
    case kTag<MatchEnd>: {
        MatchEnd m;
        m.reason = enumIn<EndReason>(r, 1, 4);
        const auto w = r.i32();
        if (w < 0 || w > 3) r.fail();
        m.winner = static_cast<sim::Outcome>(w);
        m.culprit = r.u8();
        return m;
    }
    default: return std::nullopt;
    }
}

} // namespace

std::vector<std::uint8_t> encodeMessage(const Message& message) {
    ByteWriter w;
    std::visit(
        [&w](const auto& m) {
            w.u8(kTag<std::decay_t<decltype(m)>>);
            put(w, m);
        },
        message);
    return w.take();
}

std::optional<Message> decodeMessage(std::span<const std::uint8_t> bytes) {
    if (bytes.empty() || bytes.size() > kMaxPacketBytes) return std::nullopt;
    ByteReader r(bytes);
    const auto tag = r.u8();
    auto message = get(tag, r);
    if (!message || !r.atEnd()) return std::nullopt;
    return message;
}

std::uint8_t channelFor(const Message& message) {
    return std::holds_alternative<Inputs>(message) ? kChannelInputs : kChannelControl;
}

std::string describe(Refusal reason, const std::string& detail) {
    switch (reason) {
    case Refusal::Version:
        return "DIFFERENT GAME VERSION" + (detail.empty() ? std::string{} : " (SERVER " + detail + ")");
    case Refusal::GameData: return "DIFFERENT GAME DATA THAN THE SERVER";
    case Refusal::Full: return "SERVER IS FULL";
    case Refusal::BadName: return "CHOOSE ANOTHER NAME";
    case Refusal::ShuttingDown: return "SERVER IS SHUTTING DOWN";
    }
    return "REFUSED";
}

std::string describe(Problem what) {
    switch (what) {
    case Problem::RoomNotFound: return "ROOM NOT FOUND";
    case Problem::RoomFull: return "ROOM IS FULL";
    case Problem::TooManyRooms: return "TOO MANY ROOMS FROM YOUR ADDRESS";
    case Problem::TooManyAttempts: return "TOO MANY WRONG CODES - WAIT A MINUTE";
    case Problem::NotInRoom: return "NOT IN A ROOM";
    case Problem::ServerFull: return "SERVER HAS NO ROOM LEFT";
    case Problem::BadPick: return "THAT PICK IS NOT AVAILABLE";
    case Problem::RoomClosed: return "ROOM CLOSED";
    }
    return "ERROR";
}

} // namespace iron::net
