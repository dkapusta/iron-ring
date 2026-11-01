// M4.1 foundations (ADR-0026): state serialization, replay files, and the
// rollback session against a simulated network.
#include "Simulation/Cpu.h"
#include "Simulation/Match.h"
#include "Simulation/Replay.h"
#include "Simulation/Rollback.h"
#include "Simulation/Serialize.h"

#include <doctest/doctest.h>

#include <deque>
#include <random>

using namespace iron::sim;

namespace {

// Two CPUs fight; calls onTick(inputs) before each step.
template <typename F> Match cpuMatch(int ticks, std::uint32_t seed, F&& onTick) {
    Match m;
    m.reset();
    CpuPlayer a(3, seed), b(3, seed * 7 + 1);
    for (int i = 0; i < ticks && !m.finished; ++i) {
        const std::array<Input, 2> in{a.think(m.round, 0), b.think(m.round, 1)};
        onTick(m, in);
        (void)m.step(in);
    }
    return m;
}

} // namespace

TEST_CASE("serialized match state restores exactly and plays on identically") {
    int checked = 0;
    (void)cpuMatch(4000, 11, [&](const Match& m, const std::array<Input, 2>& in) {
        if (m.round.tick % 97 != 0) return;
        Match copy; // the kit specs, as the original
        REQUIRE(deserialize(serialize(m), copy));
        CHECK(copy.checksum() == m.checksum());
        Match a = m, b = copy;
        for (int i = 0; i < 60; ++i) {
            (void)a.step(in);
            (void)b.step(in);
        }
        CHECK(a.checksum() == b.checksum());
        ++checked;
    });
    CHECK(checked > 10);
}

TEST_CASE("serialization covers the training rules and wide stages") {
    Match m;
    m.round.training = true;
    m.round.stageWidth = 800;
    m.round.fighters[0].recoverTicks = 17;
    Match copy;
    REQUIRE(deserialize(serialize(m), copy));
    CHECK(copy.round.training);
    CHECK(copy.round.stageWidth == 800);
    CHECK(copy.round.fighters[0].recoverTicks == 17);
    CHECK(copy.checksum() == m.checksum());
}

TEST_CASE("damaged or foreign state is rejected") {
    Match m;
    auto bytes = serialize(m);
    Match out;
    CHECK_FALSE(deserialize({bytes.data(), bytes.size() - 1}, out)); // truncated
    auto longer = bytes;
    longer.push_back(0);
    CHECK_FALSE(deserialize(longer, out)); // trailing bytes
    auto version = bytes;
    version[0] ^= 0xFF;
    CHECK_FALSE(deserialize(version, out)); // another state version

    // A move index the simulation would dereference: out of range must not load.
    m.round.fighters[0].move = 99;
    CHECK_FALSE(deserialize(serialize(m), out));
}

TEST_CASE("replays round-trip and reproduce the match") {
    Replay replay;
    replay.gameVersion = "test";
    replay.fighters = {"dee", "bram"};
    replay.stage = "pier_nine";
    replay.contentHash = replayContentHash(kitBoxer(), kitGrappler(), kStageWidth);
    replay.recordedAt = 1793534400;
    const Match recorded =
        cpuMatch(20000, 5, [&](const Match&, const std::array<Input, 2>& in) { replay.inputs.push_back(in); });
    REQUIRE(recorded.finished);
    replay.finalChecksum = recorded.checksum();

    const auto bytes = encodeReplay(replay);
    CHECK(bytes.size() < replay.inputs.size() * 4); // the run-length encoding pays off
    std::string error;
    const auto decoded = decodeReplay(bytes, &error);
    REQUIRE_MESSAGE(decoded, error);
    CHECK(decoded->fighters == replay.fighters);
    CHECK(decoded->stage == "pier_nine");
    CHECK(decoded->recordedAt == replay.recordedAt);
    CHECK(decoded->inputs == replay.inputs);

    Match playback;
    playback.reset();
    for (const auto& in : decoded->inputs)
        (void)playback.step(in);
    CHECK(playback.checksum() == decoded->finalChecksum);
    CHECK(playback.winner == recorded.winner);
}

TEST_CASE("damaged replays are rejected with a reason") {
    Replay replay;
    replay.inputs.assign(10, {1, 2});
    auto bytes = encodeReplay(replay);
    std::string error;
    CHECK_FALSE(decodeReplay({bytes.data(), bytes.size() - 3}, &error));
    CHECK_FALSE(error.empty());
    bytes[0] = 'X';
    CHECK_FALSE(decodeReplay(bytes, &error));
    CHECK(error == "not an Iron Ring replay");
}

TEST_CASE("spec hashes change with gameplay data") {
    FighterSpec a = kitBoxer();
    const auto before = specHash(a);
    CHECK(specHash(kitBoxer()) == before);
    a.moves[LightPunchSlot].damage += 1;
    CHECK(specHash(a) != before);
    CHECK(replayContentHash(kitBoxer(), kitGrappler(), 640) != replayContentHash(kitBoxer(), kitGrappler(), 800));
}

// ---- rollback over a simulated network ----------------------------------------------------

namespace {

struct Link {
    int latency = 0, jitter = 0; // in frames
    int lossPercent = 0;
};

// Two peers, each sending every unacknowledged input each frame (the M4.4
// protocol's redundancy), over a link with latency, jitter, loss and the
// reordering jitter causes. Each peer's input comes from a CPU looking at
// that peer's own (possibly predicted) view of the match, as a player would.
struct NetplayRun {
    std::array<Rollback, 2> peers;
    std::array<CpuPlayer, 2> cpus;
    struct Packet {
        int arrive;
        int from; // first tick in inputs
        std::vector<Input> inputs;
        int ack; // the sender's remoteTicks()
    };
    std::array<std::deque<Packet>, 2> inFlight; // to peer i
    std::array<int, 2> acked{0, 0};             // what peer i knows its partner received
    std::mt19937 rng{1234};
    Link link;
    int frame = 0;
    int stalls = 0;

    NetplayRun(const Match& start, Link l, int delay)
        : peers{Rollback(start, 0, delay), Rollback(start, 1, delay)}, cpus{CpuPlayer(3, 21), CpuPlayer(3, 77)},
          link(l) {}

    void step(int stopInputsAt) {
        for (int i = 0; i < 2; ++i) {
            auto& p = peers[static_cast<std::size_t>(i)];
            // Deliver what has arrived.
            auto& q = inFlight[static_cast<std::size_t>(i)];
            for (auto it = q.begin(); it != q.end();) {
                if (it->arrive > frame) {
                    ++it;
                    continue;
                }
                for (std::size_t k = 0; k < it->inputs.size(); ++k)
                    p.addRemoteInput(it->from + static_cast<int>(k), it->inputs[k]);
                acked[static_cast<std::size_t>(i)] = std::max(acked[static_cast<std::size_t>(i)], it->ack);
                it = q.erase(it);
            }
        }
        for (int i = 0; i < 2; ++i) {
            auto& p = peers[static_cast<std::size_t>(i)];
            if (!p.canAdvance()) {
                ++stalls;
            } else {
                const Input in = p.localTicks() < stopInputsAt
                                     ? cpus[static_cast<std::size_t>(i)].think(p.state().round, i)
                                     : Input{0};
                p.addLocalInput(in);
                (void)p.advance();
            }
            // Send everything the partner hasn't acknowledged.
            const int from = acked[static_cast<std::size_t>(i)];
            if (static_cast<int>(rng() % 100) >= link.lossPercent) {
                const int delayFrames =
                    link.latency + (link.jitter ? static_cast<int>(rng() % static_cast<unsigned>(link.jitter + 1)) : 0);
                inFlight[static_cast<std::size_t>(1 - i)].push_back(
                    {frame + delayFrames, from, p.localInputsFrom(from), p.remoteTicks()});
            }
        }
        ++frame;
    }
};

// Plays `ticks` ticks over the link, then lets the peers settle, and checks
// both against an offline match fed the inputs both sides actually sent.
void playOver(Link link, int delay, int ticks) {
    Match start;
    start.reset();
    NetplayRun run(start, link, delay);
    while ((run.peers[0].confirmedTicks() < ticks || run.peers[1].confirmedTicks() < ticks) && run.frame < ticks * 4)
        run.step(ticks);
    INFO("frames " << run.frame << " ticks " << run.peers[0].tick() << "/" << run.peers[1].tick() << " remote "
                   << run.peers[0].remoteTicks() << "/" << run.peers[1].remoteTicks() << " local "
                   << run.peers[0].localTicks() << "/" << run.peers[1].localTicks() << " acked " << run.acked[0] << "/"
                   << run.acked[1] << " stalls " << run.stalls);
    REQUIRE(run.peers[0].confirmedTicks() >= ticks);
    REQUIRE(run.peers[1].confirmedTicks() >= ticks);

    Match reference = start;
    std::vector<std::uint64_t> expected;
    for (int t = 0; t < ticks; ++t) {
        (void)reference.step({run.peers[0].localInput(t), run.peers[1].localInput(t)});
        if ((t + 1) % kChecksumInterval == 0) expected.push_back(reference.checksum());
    }
    for (std::size_t k = 0; k < expected.size(); ++k) {
        const int t = static_cast<int>((k + 1) * kChecksumInterval - 1);
        CAPTURE(t);
        REQUIRE(run.peers[0].checksumAfter(t));
        REQUIRE(run.peers[1].checksumAfter(t));
        CHECK(*run.peers[0].checksumAfter(t) == expected[k]);
        CHECK(*run.peers[1].checksumAfter(t) == expected[k]);
    }
    // Each side saw the other's inputs exactly.
    for (int t = 0; t < ticks; ++t) {
        CHECK(run.peers[0].remoteInput(t) == run.peers[1].localInput(t));
        CHECK(run.peers[1].remoteInput(t) == run.peers[0].localInput(t));
    }
    MESSAGE("latency " << link.latency << " jitter " << link.jitter << " loss " << link.lossPercent << "%: rollbacks "
                       << run.peers[0].rollbacks() << "/" << run.peers[1].rollbacks() << ", deepest "
                       << std::max(run.peers[0].deepestRollback(), run.peers[1].deepestRollback()) << ", stalls "
                       << run.stalls);
}

} // namespace

TEST_CASE("rollback: a perfect link never rolls back with enough input delay") {
    Match start;
    start.reset();
    NetplayRun run(start, {1, 0, 0}, 2);
    for (int i = 0; i < 600; ++i)
        run.step(600);
    CHECK(run.peers[0].rollbacks() == 0);
    CHECK(run.peers[1].rollbacks() == 0);
    playOver({1, 0, 0}, 2, 600);
}

TEST_CASE("rollback: both peers agree under latency, jitter and loss") {
    playOver({3, 0, 0}, 1, 1800);  // about 100 ms round trip, little delay: predictions and rollbacks
    playOver({4, 3, 10}, 2, 1800); // jitter reorders packets, 10 % are lost
    playOver({6, 2, 25}, 0, 1200); // no delay at all, a bad link: deep rollbacks and stalls
}

TEST_CASE("rollback: a silent peer stalls the session instead of predicting forever") {
    Match start;
    start.reset();
    Rollback peer(start, 0, 2);
    int advanced = 0;
    for (int i = 0; i < 40; ++i) {
        if (!peer.canAdvance()) continue;
        peer.addLocalInput(0);
        if (peer.advance().advanced) ++advanced;
    }
    CHECK(advanced == kMaxRollback);
    CHECK(peer.predicted() == kMaxRollback);
    CHECK(peer.localTicks() == kMaxRollback + 2); // still exactly `delay` ahead
    for (int t = 0; t < 4; ++t)
        peer.addRemoteInput(t, 0);
    CHECK(peer.canAdvance());
}

TEST_CASE("rollback: a misprediction re-simulates from the wrong tick") {
    Match start;
    start.reset();
    Rollback peer(start, 0, 0);
    for (int t = 0; t < 5; ++t) {
        peer.addLocalInput(0);
        (void)peer.advance();
    }
    peer.addRemoteInput(0, 0);
    peer.addRemoteInput(1, 0);
    peer.addRemoteInput(2, Right); // predicted 0 for ticks 2..4
    peer.addLocalInput(0);
    const auto result = peer.advance();
    CHECK(result.rolledBack == 3);
    REQUIRE(result.steps.size() == 4);
    CHECK(result.steps[0].tick == 2);
    CHECK(result.steps[0].resimulated);
    CHECK_FALSE(result.steps[3].resimulated);
    CHECK(peer.tick() == 6);
}

TEST_CASE("rollback: settle corrects the past without running a new tick") {
    Match start;
    start.reset();
    Rollback peer(start, 0, 1);
    for (int t = 0; t < 4; ++t) {
        peer.addLocalInput(0);
        (void)peer.advance();
    }
    CHECK(peer.tick() == 4);
    CHECK(peer.confirmedTicks() == 0);
    for (int t = 0; t < 4; ++t)
        peer.addRemoteInput(t, t == 2 ? Right : 0);
    const auto result = peer.settle();
    CHECK(result.rolledBack == 2);
    CHECK(result.steps.size() == 2);
    CHECK_FALSE(result.advanced);
    CHECK(peer.tick() == 4);
    CHECK(peer.confirmedTicks() == 4);
    CHECK(peer.settle().steps.empty());
}
