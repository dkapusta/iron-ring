#include "Simulation/Serialize.h"

namespace iron::sim {

void ByteWriter::u16(std::uint16_t v) {
    u8(static_cast<std::uint8_t>(v & 0xFF));
    u8(static_cast<std::uint8_t>(v >> 8));
}

void ByteWriter::u32(std::uint32_t v) {
    for (int i = 0; i < 4; ++i)
        u8(static_cast<std::uint8_t>((v >> (8 * i)) & 0xFF));
}

void ByteWriter::u64(std::uint64_t v) {
    for (int i = 0; i < 8; ++i)
        u8(static_cast<std::uint8_t>((v >> (8 * i)) & 0xFF));
}

void ByteWriter::str(std::string_view s) {
    const auto n = s.size() > 0xFFFF ? std::size_t{0xFFFF} : s.size();
    u16(static_cast<std::uint16_t>(n));
    for (std::size_t i = 0; i < n; ++i)
        u8(static_cast<std::uint8_t>(s[i]));
}

bool ByteReader::take(std::size_t n) {
    if (!ok_ || data_.size() - pos_ < n) {
        ok_ = false;
        return false;
    }
    return true;
}

std::uint8_t ByteReader::u8() {
    return take(1) ? data_[pos_++] : 0;
}

std::uint16_t ByteReader::u16() {
    if (!take(2)) return 0;
    const auto v = static_cast<std::uint16_t>(data_[pos_] | (data_[pos_ + 1] << 8));
    pos_ += 2;
    return v;
}

std::uint32_t ByteReader::u32() {
    if (!take(4)) return 0;
    std::uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
        v |= static_cast<std::uint32_t>(data_[pos_ + static_cast<std::size_t>(i)]) << (8 * i);
    pos_ += 4;
    return v;
}

std::uint64_t ByteReader::u64() {
    if (!take(8)) return 0;
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<std::uint64_t>(data_[pos_ + static_cast<std::size_t>(i)]) << (8 * i);
    pos_ += 8;
    return v;
}

bool ByteReader::boolean() {
    const auto v = u8();
    if (v > 1) fail();
    return v == 1;
}

std::string ByteReader::str() {
    const auto n = u16();
    if (!take(n)) return {};
    std::string s(reinterpret_cast<const char*>(data_.data() + pos_), n);
    pos_ += n;
    return s;
}

namespace {

template <typename E> E readEnum(ByteReader& r, std::int32_t last) {
    const auto v = r.i32();
    if (v < 0 || v > last) r.fail();
    return static_cast<E>(v);
}

} // namespace

// Field order is the format: change it only with kStateVersion.
void write(ByteWriter& w, const Fighter& f) {
    for (const int v : {f.x, f.y, f.vy, f.facing, f.hp, f.meter, f.move, f.age, f.stun, f.buffered, f.bufferTicks,
                        f.breakTicks, f.side})
        w.i32(v);
    w.u16(f.previous);
    for (const bool b : {f.connected, f.crouching, f.guarding, f.blockstun})
        w.boolean(b);
    for (const int v : {f.knockdown, f.throwTicks, f.throwSlot, f.heldTicks, f.techTicks, f.motionStep, f.motionTimer,
                        f.motionReady, f.recoverTicks, f.combo})
        w.i32(v);
}

bool read(ByteReader& r, Fighter& f) {
    for (int* v : {&f.x, &f.y, &f.vy, &f.facing, &f.hp, &f.meter, &f.move, &f.age, &f.stun, &f.buffered, &f.bufferTicks,
                   &f.breakTicks, &f.side})
        *v = r.i32();
    f.previous = static_cast<Input>(r.u16() & kInputMask);
    for (bool* b : {&f.connected, &f.crouching, &f.guarding, &f.blockstun})
        *b = r.boolean();
    for (int* v : {&f.knockdown, &f.throwTicks, &f.throwSlot, &f.heldTicks, &f.techTicks, &f.motionStep, &f.motionTimer,
                   &f.motionReady, &f.recoverTicks, &f.combo})
        *v = r.i32();
    // Indices the simulation dereferences must be in range, or a crafted packet could crash it.
    if (f.side < 0 || f.side > 1 || f.move < -1 || f.move >= kSlotCount || f.throwSlot < -1 ||
        f.throwSlot >= kSlotCount || f.buffered < -1 || f.buffered >= kSlotCount)
        r.fail();
    return r.ok();
}

void write(ByteWriter& w, const Round& round) {
    for (const auto& f : round.fighters)
        write(w, f);
    for (const int v : {round.tick, round.remaining, round.hitstop})
        w.i32(v);
    w.i32(static_cast<std::int32_t>(round.outcome));
    for (const int v : {round.effectTicks, round.effectX, round.effectY})
        w.i32(v);
    w.i32(static_cast<std::int32_t>(round.effect));
    w.boolean(round.training);
    w.i32(round.stageWidth);
}

bool read(ByteReader& r, Round& round) {
    for (auto& f : round.fighters)
        if (!read(r, f)) return false;
    for (int* v : {&round.tick, &round.remaining, &round.hitstop})
        *v = r.i32();
    round.outcome = readEnum<Outcome>(r, static_cast<std::int32_t>(Outcome::Draw));
    for (int* v : {&round.effectTicks, &round.effectX, &round.effectY})
        *v = r.i32();
    round.effect = readEnum<Effect>(r, static_cast<std::int32_t>(Effect::Slam));
    round.training = r.boolean();
    round.stageWidth = r.i32();
    if (round.stageWidth < kStageWidth || round.stageWidth > 4096) r.fail();
    return r.ok();
}

void write(ByteWriter& w, const Match& m) {
    w.u16(kStateVersion);
    write(w, m.round);
    w.i32(m.roundNumber);
    w.i32(m.wins[0]);
    w.i32(m.wins[1]);
    w.i32(static_cast<std::int32_t>(m.phase));
    w.i32(m.phaseTicks);
    w.i32(static_cast<std::int32_t>(m.lastRound));
    w.i32(static_cast<std::int32_t>(m.winner));
    w.boolean(m.finished);
}

bool read(ByteReader& r, Match& m) {
    if (r.u16() != kStateVersion) return false;
    if (!read(r, m.round)) return false;
    m.roundNumber = r.i32();
    m.wins[0] = r.i32();
    m.wins[1] = r.i32();
    m.phase = readEnum<Phase>(r, static_cast<std::int32_t>(Phase::MatchOver));
    m.phaseTicks = r.i32();
    m.lastRound = readEnum<Outcome>(r, static_cast<std::int32_t>(Outcome::Draw));
    m.winner = readEnum<Outcome>(r, static_cast<std::int32_t>(Outcome::Draw));
    m.finished = r.boolean();
    return r.ok();
}

std::vector<std::uint8_t> serialize(const Match& m) {
    ByteWriter w;
    write(w, m);
    return w.take();
}

bool deserialize(std::span<const std::uint8_t> bytes, Match& m) {
    ByteReader r(bytes);
    return read(r, m) && r.atEnd();
}

} // namespace iron::sim
