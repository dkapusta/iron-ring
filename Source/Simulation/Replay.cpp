#include "Simulation/Replay.h"

#include "Simulation/Serialize.h"

namespace iron::sim {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic{'I', 'R', 'R', 'P'};
constexpr std::size_t kMaxTicks = 60 * 60 * 30; // half an hour: far beyond any best-of-5

} // namespace

std::vector<std::uint8_t> encodeReplay(const Replay& replay) {
    ByteWriter w;
    w.raw(kMagic);
    w.u16(kReplayVersion);
    w.str(replay.gameVersion);
    w.u64(replay.contentHash);
    w.str(replay.fighters[0]);
    w.str(replay.fighters[1]);
    w.str(replay.stage);
    w.i32(replay.stageWidth);
    w.u64(static_cast<std::uint64_t>(replay.recordedAt));
    w.u32(static_cast<std::uint32_t>(replay.inputs.size()));
    for (std::size_t i = 0; i < replay.inputs.size();) {
        std::size_t run = 1;
        while (i + run < replay.inputs.size() && run < 0xFFFF && replay.inputs[i + run] == replay.inputs[i])
            ++run;
        w.u16(static_cast<std::uint16_t>(run));
        w.u16(replay.inputs[i][0]);
        w.u16(replay.inputs[i][1]);
        i += run;
    }
    w.u64(replay.finalChecksum);
    return w.take();
}

std::optional<Replay> decodeReplay(std::span<const std::uint8_t> bytes, std::string* error) {
    const auto fail = [&](const char* why) -> std::optional<Replay> {
        if (error) *error = why;
        return std::nullopt;
    };
    ByteReader r(bytes);
    for (const auto m : kMagic)
        if (r.u8() != m) return fail("not an Iron Ring replay");
    if (const auto v = r.u16(); v != kReplayVersion) return fail("unsupported replay version");
    Replay replay;
    replay.gameVersion = r.str();
    replay.contentHash = r.u64();
    replay.fighters[0] = r.str();
    replay.fighters[1] = r.str();
    replay.stage = r.str();
    replay.stageWidth = r.i32();
    replay.recordedAt = static_cast<std::int64_t>(r.u64());
    const auto ticks = r.u32();
    if (!r.ok() || ticks > kMaxTicks) return fail("damaged replay header");
    replay.inputs.reserve(ticks);
    while (replay.inputs.size() < ticks && r.ok()) {
        const auto run = r.u16();
        const std::array<Input, 2> pair{static_cast<Input>(r.u16() & kInputMask),
                                        static_cast<Input>(r.u16() & kInputMask)};
        if (run == 0 || replay.inputs.size() + run > ticks) return fail("damaged replay inputs");
        replay.inputs.insert(replay.inputs.end(), run, pair);
    }
    replay.finalChecksum = r.u64();
    if (!r.atEnd()) return fail("damaged replay");
    return replay;
}

std::uint64_t replayContentHash(const FighterSpec& p1, const FighterSpec& p2, int stageWidth) {
    std::uint64_t h = 14695981039346656037ULL;
    for (const auto v : {specHash(p1), specHash(p2), static_cast<std::uint64_t>(stageWidth)})
        for (int j = 0; j < 8; ++j) {
            h ^= (v >> (8 * j)) & 255U;
            h *= 1099511628211ULL;
        }
    return h;
}

} // namespace iron::sim
