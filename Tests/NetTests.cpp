// M4.2 (ADR-0026): room codes, invites, the protocol, server settings, and
// the dedicated server run in-process with two bot clients over loopback.
#include "Net/Codes.h"
#include "Net/Net.h"
#include "Net/Protocol.h"
#include "Net/Transport.h"
#include "Server/GameRules.h"
#include "Server/Server.h"
#include "Server/Settings.h"
#include "Simulation/Cpu.h"
#include "Simulation/Rollback.h"
#include "Vfs/DirectoryMount.h"
#include "Vfs/VirtualFileSystem.h"

#include <doctest/doctest.h>

#include <chrono>
#include <functional>
#include <random>

using namespace iron;
using namespace iron::net;

TEST_CASE("room codes are six unambiguous characters") {
    std::random_device rd;
    for (int i = 0; i < 200; ++i) {
        const auto code = newRoomCode(rd);
        REQUIRE(code.size() == kCodeLength);
        for (const char c : code)
            CHECK(kCodeAlphabet.find(c) != std::string_view::npos);
        CHECK(normalizeCode(code) == code);
    }
    CHECK(kCodeAlphabet.size() == 31);
    CHECK(normalizeCode(" k7qx-2m ") == "K7QX2M");
    CHECK_FALSE(normalizeCode("K7QX2"));   // too short
    CHECK_FALSE(normalizeCode("K7QX2MM")); // too long
    CHECK_FALSE(normalizeCode("K0QX2M"));  // 0 is not in the alphabet
    CHECK_FALSE(normalizeCode("KIQX2M"));
}

TEST_CASE("invites parse what players paste") {
    const auto full = parseInvite("  203.0.113.5:7727/k7qx2m\n");
    REQUIRE(full);
    CHECK(full->host == "203.0.113.5");
    CHECK(full->port == 7727);
    CHECK(full->code == "K7QX2M");
    CHECK(formatInvite(*full) == "203.0.113.5:7727/K7QX2M");

    const auto named = parseInvite("IronRing://Ring.Example.org:9000/");
    REQUIRE(named);
    CHECK(named->host == "ring.example.org");
    CHECK(named->port == 9000);
    CHECK(named->code.empty());
    CHECK(formatInvite(*named) == "RING.EXAMPLE.ORG:9000");

    const auto bare = parseInvite("localhost");
    REQUIRE(bare);
    CHECK(bare->port == kDefaultPort);
    CHECK(parseInvite("ring.example.org/K7QX2M")->code == "K7QX2M");

    CHECK_FALSE(parseInvite(""));
    CHECK_FALSE(parseInvite("host:0"));
    CHECK_FALSE(parseInvite("host:70000"));
    CHECK_FALSE(parseInvite("host:12ab"));
    CHECK_FALSE(parseInvite("host/BADCODE0"));
    CHECK_FALSE(parseInvite("[::1]:7727")); // no IPv6
    CHECK_FALSE(parseInvite("bad host"));
}

TEST_CASE("player names fit the font") {
    CHECK(sanitizeName("  dmitry ") == "DMITRY");
    CHECK(sanitizeName("Ann-Marie.K") == "ANN-MARIE.K");
    CHECK(sanitizeName("a_b!c") == "ABC");
    CHECK(sanitizeName("THIS NAME IS FAR TOO LONG") == "THIS NAME IS");
    CHECK(sanitizeName("two  spaces") == "TWO SPACES");
    CHECK(sanitizeName("!!!").empty());
}

TEST_CASE("game data hashes ignore order and see every change") {
    const auto a = gameDataHash({{"dee", 1}, {"bram", 2}}, {{"pier", 800}});
    CHECK(gameDataHash({{"bram", 2}, {"dee", 1}}, {{"pier", 800}}) == a);
    CHECK(gameDataHash({{"bram", 2}, {"dee", 3}}, {{"pier", 800}}) != a);
    CHECK(gameDataHash({{"dee", 1}, {"bram", 2}}, {{"pier", 640}}) != a);
}

TEST_CASE("every message round-trips") {
    const std::vector<Message> all{
        Hello{kProtocolVersion, "0.1.0", 0x1234, "DMITRY"},
        CreateRoom{true},
        JoinRoom{"K7QX2M"},
        ListRooms{},
        LeaveRoom{},
        SetOpen{true},
        Pick{"dee", "pier_nine"},
        Rematch{},
        Checksum{29, 0xDEADBEEF, 3},
        Result{sim::Outcome::Player2, 3},
        Inputs{100, {1, 2, 3, 511}, 98, 3},
        Welcome{"RING", "HELLO", 3, 1},
        Refused{Refusal::Version, "0.2.0"},
        Failed{Problem::RoomFull},
        RoomState{"K7QX2M", false, RoomPhase::Selecting, 1, {{"A", true, "dee", 40}, {"B", false, "", 55}}},
        RoomList{{{"K7QX2M", "A", 12}}},
        MatchStart{3, {"dee", "bram"}, "pier_nine", 800, 1},
        MatchEnd{EndReason::Desync, sim::Outcome::Running, 1},
    };
    for (const auto& m : all) {
        CAPTURE(m.index());
        const auto bytes = encodeMessage(m);
        const auto back = decodeMessage(bytes);
        REQUIRE(back);
        CHECK(back->index() == m.index());
        CHECK(encodeMessage(*back) == bytes);
    }
}

TEST_CASE("random and truncated bytes never decode into nonsense") {
    std::mt19937 rng(7);
    int decoded = 0;
    for (int i = 0; i < 20000; ++i) {
        std::vector<std::uint8_t> bytes(1 + rng() % 40);
        for (auto& b : bytes)
            b = static_cast<std::uint8_t>(rng());
        if (decodeMessage(bytes)) ++decoded; // allowed, but must be a well-formed message
    }
    CHECK(decoded < 2000);
    const auto bytes = encodeMessage(RoomState{"K7QX2M", false, RoomPhase::Waiting, 0, {{"A", false, "", 1}}});
    for (std::size_t n = 0; n < bytes.size(); ++n)
        CHECK_FALSE(decodeMessage({bytes.data(), n}));
    CHECK_FALSE(decodeMessage(encodeMessage(Inputs{-1, {1}, 0})));
    CHECK_FALSE(decodeMessage(encodeMessage(Inputs{0, std::vector<sim::Input>(kMaxInputsPerPacket + 1, 0), 0})));
}

TEST_CASE("server settings: server.cfg, then the command line") {
    server::Settings s;
    const auto warnings = server::applyConfigText(s, "port=9000\nname=my ring\nverify=0\nbogus=1\nmaxRooms=x\n");
    CHECK(s.server.port == 9000);
    CHECK(s.server.name == "MY RING");
    CHECK_FALSE(s.server.verify);
    CHECK(warnings.size() == 2);
    const std::vector<std::string> args{"--port", "7000", "--motd=no sandbagging", "--check", "--config", "x.cfg"};
    CHECK(server::applyArguments(s, args).empty());
    CHECK(s.server.port == 7000);
    CHECK(s.server.motd == "NO SANDBAGGING");
    CHECK(s.check);
    CHECK(s.config == "x.cfg");
    CHECK_FALSE(server::applyArguments(s, std::vector<std::string>{"--port", "99999"}).empty());
}

// ---- the server, in-process ---------------------------------------------------------------

namespace {

server::GameRules kitRules() {
    server::GameRules rules;
    rules.version = "test";
    rules.fighters = {{"dee", sim::kitBoxer()}, {"bram", sim::kitGrappler()}};
    rules.stages = {{"pier", 800}};
    rules.dataHash = gameDataHash(
        {{"dee", sim::specHash(sim::kitBoxer())}, {"bram", sim::specHash(sim::kitGrappler())}}, {{"pier", 800}});
    return rules;
}

// A headless player: connects, says hello, and (when told) fights with a
// CPU through a rollback session, exactly as the game will.
struct Bot {
    Client client;
    std::vector<Message> inbox;
    std::optional<RoomState> room;
    std::optional<MatchStart> start;
    std::optional<MatchEnd> end;
    bool welcomed = false, refused = false, dropped = false;
    std::optional<Problem> failed;
    // Fighting:
    std::unique_ptr<sim::Match> match;
    std::unique_ptr<sim::Rollback> session;
    std::optional<sim::CpuPlayer> cpu;
    int acked = 0;
    std::size_t sentSums = 0;
    bool corruptChecksums = false;
    bool sendResult = false, sentResult = false;

    void pump() {
        for (auto& e : client.poll(0)) {
            if (e.kind == ClientEvent::Kind::Disconnected) dropped = true;
            if (e.kind != ClientEvent::Kind::Received) continue;
            std::visit(
                [&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, Welcome>) welcomed = true;
                    if constexpr (std::is_same_v<T, Refused>) refused = true;
                    if constexpr (std::is_same_v<T, Failed>) failed = m.what;
                    if constexpr (std::is_same_v<T, RoomState>) room = m;
                    if constexpr (std::is_same_v<T, MatchStart>) begin(m);
                    if constexpr (std::is_same_v<T, MatchEnd>) end = m;
                    if constexpr (std::is_same_v<T, Inputs>) {
                        if (session && start && m.match == start->match) {
                            for (std::size_t k = 0; k < m.inputs.size(); ++k)
                                session->addRemoteInput(m.from + static_cast<int>(k), m.inputs[k]);
                            acked = std::max(acked, m.ack);
                        }
                    }
                },
                e.message);
            inbox.push_back(std::move(e.message));
        }
        if (session && !end) fight();
    }

    void begin(const MatchStart& m) {
        start = m;
        end.reset();
        const auto& spec = [](const std::string& id) -> const sim::FighterSpec& {
            return id == "dee" ? sim::kitBoxer() : sim::kitGrappler();
        };
        match = std::make_unique<sim::Match>(spec(m.fighters[0]), spec(m.fighters[1]));
        match->round.stageWidth = m.stageWidth;
        match->round.reset();
        session = std::make_unique<sim::Rollback>(*match, m.side, 2);
        cpu.emplace(3, 1000u + m.side);
        acked = 0;
        sentSums = 0;
        sentResult = false;
    }

    void fight() {
        if (session->canAdvance()) {
            session->addLocalInput(cpu->think(session->state().round, session->side()));
            (void)session->advance();
        }
        client.send(Inputs{acked, session->localInputsFrom(acked), session->remoteTicks(), start->match});
        const auto& sums = session->checksums();
        for (; sentSums < sums.size(); ++sentSums)
            client.send(
                Checksum{sums[sentSums].first, sums[sentSums].second ^ (corruptChecksums ? 1u : 0u), start->match});
        const auto& m = session->state();
        if (sendResult && !sentResult && m.finished && session->confirmedTicks() == session->tick()) {
            client.send(Result{m.winner, start->match});
            sentResult = true;
        }
    }
};

struct Harness {
    server::Server srv;
    std::vector<Bot*> bots;

    explicit Harness(server::Config config = {})
        : srv(
              [&] {
                  config.port = 0; // any free port
                  return config;
              }(),
              kitRules(), nullptr) {
        std::string error;
        REQUIRE_MESSAGE(srv.start(&error), error);
    }

    void connect(Bot& b, const std::string& name, std::uint64_t dataHash = 0) {
        bots.push_back(&b);
        REQUIRE(b.client.connect("127.0.0.1", srv.port()));
        REQUIRE(until([&] { return b.client.connected(); }));
        b.client.send(Hello{kProtocolVersion, "test", dataHash ? dataHash : kitRules().dataHash, name});
    }

    bool until(const std::function<bool()>& done, double seconds = 10) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(seconds);
        while (std::chrono::steady_clock::now() < deadline) {
            srv.poll(0);
            for (auto* b : bots)
                b->pump();
            if (done()) return true;
        }
        return false;
    }
};

// Host creates a private room, guest joins by code, both pick.
void meet(Harness& h, Bot& host, Bot& guest) {
    h.connect(host, "HOST");
    h.connect(guest, "GUEST");
    REQUIRE(h.until([&] { return host.welcomed && guest.welcomed; }));
    host.client.send(CreateRoom{false});
    REQUIRE(h.until([&] { return host.room.has_value(); }));
    CHECK(host.room->members.size() == 1);
    guest.client.send(JoinRoom{host.room->code});
    REQUIRE(h.until([&] { return guest.room && host.room->members.size() == 2; }));
    CHECK(guest.room->you == 1);
    CHECK(host.room->phase == RoomPhase::Selecting);
    host.client.send(Pick{"dee", "pier"});
    guest.client.send(Pick{"bram", ""});
    REQUIRE(h.until([&] { return host.start && guest.start; }));
}

} // namespace

TEST_CASE("server: two players meet by room code and the referee confirms the result") {
    Harness h;
    Bot host, guest;
    meet(h, host, guest);
    CHECK(host.start->side == 0);
    CHECK(guest.start->side == 1);
    CHECK(host.start->stageWidth == 800);
    CHECK(host.start->fighters == std::array<std::string, 2>{"dee", "bram"});
    REQUIRE(h.until([&] { return host.end && guest.end; }, 60));
    CHECK(host.end->reason == EndReason::Finished);
    CHECK(guest.end->winner == host.end->winner);
    CHECK(host.end->winner == host.session->state().winner); // the client's view agrees with the referee's
    MESSAGE("match of " << host.session->tick() << " ticks; rollbacks " << host.session->rollbacks() << "/"
                        << guest.session->rollbacks());

    // Both ask for a rematch: a second match starts.
    host.session.reset();
    guest.session.reset();
    host.start.reset();
    guest.start.reset();
    host.client.send(Rematch{});
    guest.client.send(Rematch{});
    REQUIRE(h.until([&] { return host.start && guest.start; }));
    CHECK(host.start->match == 2);
}

TEST_CASE("server: a client that goes out of sync is named") {
    Harness h;
    Bot host, guest;
    guest.corruptChecksums = true;
    meet(h, host, guest);
    REQUIRE(h.until([&] { return host.end && guest.end; }, 30));
    CHECK(host.end->reason == EndReason::Desync);
    CHECK(host.end->culprit == 1);
}

TEST_CASE("server: without the referee, the clients' results must agree") {
    server::Config config;
    config.verify = false;
    Harness h(config);
    Bot host, guest;
    host.sendResult = guest.sendResult = true;
    meet(h, host, guest);
    REQUIRE(h.until([&] { return host.end && guest.end; }, 60));
    CHECK(host.end->reason == EndReason::Finished);
    CHECK(host.end->winner == host.session->state().winner);
}

TEST_CASE("server: refusals, private rooms, the open list and wrong codes") {
    server::Config config;
    config.joinAttemptsPerMinute = 3;
    Harness h(config);

    Bot stranger;
    h.connect(stranger, "X", 0xBAD);
    CHECK(h.until([&] { return stranger.refused; }));

    Bot nameless;
    h.connect(nameless, "!!!");
    CHECK(h.until([&] { return nameless.refused; }));

    Bot host, other, guesser;
    h.connect(host, "HOST");
    h.connect(other, "OTHER");
    h.connect(guesser, "GUESS");
    REQUIRE(h.until([&] { return host.welcomed && other.welcomed && guesser.welcomed; }));

    host.client.send(CreateRoom{false});
    REQUIRE(h.until([&] { return host.room.has_value(); }));
    // A private room is not listed...
    other.client.send(ListRooms{});
    REQUIRE(h.until([&] { return !other.inbox.empty() && std::holds_alternative<RoomList>(other.inbox.back()); }));
    CHECK(std::get<RoomList>(other.inbox.back()).rooms.empty());
    // ...until the host opens it.
    host.client.send(SetOpen{true});
    REQUIRE(h.until([&] { return host.room->open; }));
    other.client.send(ListRooms{});
    REQUIRE(h.until([&] {
        return std::holds_alternative<RoomList>(other.inbox.back()) &&
               !std::get<RoomList>(other.inbox.back()).rooms.empty();
    }));
    const auto listed = std::get<RoomList>(other.inbox.back()).rooms;
    CHECK(listed[0].code == host.room->code);
    CHECK(listed[0].host == "HOST");

    // A full room refuses a third player.
    other.client.send(JoinRoom{host.room->code});
    REQUIRE(h.until([&] { return host.room->members.size() == 2; }));
    Bot third;
    h.connect(third, "THIRD");
    REQUIRE(h.until([&] { return third.welcomed; }));
    third.client.send(JoinRoom{host.room->code});
    REQUIRE(h.until([&] { return third.failed.has_value(); }));
    CHECK(*third.failed == Problem::RoomFull);
    // Wrong codes: three misses, then the address waits (every bot here is 127.0.0.1).
    for (int i = 0; i < 3; ++i) {
        guesser.failed.reset();
        guesser.client.send(JoinRoom{"222222"});
        REQUIRE(h.until([&] { return guesser.failed.has_value(); }));
        CHECK(*guesser.failed == Problem::RoomNotFound);
    }
    guesser.failed.reset();
    guesser.client.send(JoinRoom{host.room->code});
    REQUIRE(h.until([&] { return guesser.failed.has_value(); }));
    CHECK(*guesser.failed == Problem::TooManyAttempts);
}

TEST_CASE("server: a player leaving mid-match ends it, and the room waits with its code") {
    Harness h;
    Bot host, guest;
    meet(h, host, guest);
    const auto code = host.room->code;
    REQUIRE(h.until([&] { return host.session && host.session->tick() > 200; }));
    guest.client.disconnect();
    REQUIRE(h.until([&] { return host.end.has_value(); }));
    CHECK(host.end->reason == EndReason::Disconnected);
    REQUIRE(h.until([&] { return host.room->members.size() == 1 && host.room->phase == RoomPhase::Waiting; }));
    CHECK(host.room->code == code);
    CHECK(h.srv.rooms() == 1);
}

TEST_CASE("the server loads the shipped game data for its referee") {
    vfs::VirtualFileSystem vfs;
    vfs.mount(std::make_unique<vfs::DirectoryMount>(std::filesystem::path(IR_SOURCE_DIR)));
    vfs.mount(std::make_unique<vfs::DirectoryMount>(std::filesystem::path(IR_SOURCE_DIR) / "Assets"));
    const auto rules = server::loadGameRules(vfs, "test");
    CHECK(rules.fighters.size() == 7);
    CHECK(rules.fighters.count("broker") == 1);
    CHECK(rules.stages.size() == 7);
    CHECK(rules.stages.at("pier_nine") == 800);
    CHECK(rules.dataHash != 0);
    // The same data loaded again hashes the same.
    CHECK(server::loadGameRules(vfs, "again").dataHash == rules.dataHash);
}
