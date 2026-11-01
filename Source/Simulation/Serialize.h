#pragma once

// Explicit, versioned binary encoding of the match state (ADR-0026): field by
// field, little-endian, never raw struct memory. The network, the server's
// referee and replays use it; rollback snapshots in memory stay plain copies.
//
// Fighter specs are not part of the state: a reader fills an existing Match
// constructed with the right specs (the replay header or the room names them).

#include "Simulation/Match.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace iron::sim {

// Bump when a state field is added, removed or reordered.
inline constexpr std::uint16_t kStateVersion = 1;

class ByteWriter {
  public:
    void u8(std::uint8_t v) { bytes_.push_back(v); }
    void u16(std::uint16_t v);
    void u32(std::uint32_t v);
    void u64(std::uint64_t v);
    void i32(std::int32_t v) { u32(static_cast<std::uint32_t>(v)); }
    void boolean(bool v) { u8(v ? 1 : 0); }
    // Length-prefixed (u16), at most 65535 bytes.
    void str(std::string_view s);
    void raw(std::span<const std::uint8_t> data) { bytes_.insert(bytes_.end(), data.begin(), data.end()); }

    [[nodiscard]] const std::vector<std::uint8_t>& bytes() const { return bytes_; }
    [[nodiscard]] std::vector<std::uint8_t> take() { return std::move(bytes_); }

  private:
    std::vector<std::uint8_t> bytes_;
};

// Reads what a ByteWriter wrote. Reading past the end, or a value out of
// range, sets fail() and returns zeros: callers check ok() once at the end,
// which is safe for untrusted input (network packets, replay files).
class ByteReader {
  public:
    explicit ByteReader(std::span<const std::uint8_t> data) : data_(data) {}

    std::uint8_t u8();
    std::uint16_t u16();
    std::uint32_t u32();
    std::uint64_t u64();
    std::int32_t i32() { return static_cast<std::int32_t>(u32()); }
    bool boolean();
    std::string str();

    void fail() { ok_ = false; }
    [[nodiscard]] bool ok() const { return ok_; }
    [[nodiscard]] std::size_t remaining() const { return ok_ ? data_.size() - pos_ : 0; }
    [[nodiscard]] bool atEnd() const { return ok_ && pos_ == data_.size(); }

  private:
    bool take(std::size_t n);
    std::span<const std::uint8_t> data_;
    std::size_t pos_ = 0;
    bool ok_ = true;
};

void write(ByteWriter& w, const Fighter& f);
void write(ByteWriter& w, const Round& r);
void write(ByteWriter& w, const Match& m); // with a kStateVersion prefix

// Fill state from the reader, keeping the target's specs. False (and the
// target unspecified) on truncated or invalid data or another state version.
bool read(ByteReader& r, Fighter& f);
bool read(ByteReader& r, Round& round);
bool read(ByteReader& r, Match& m);

[[nodiscard]] std::vector<std::uint8_t> serialize(const Match& m);
bool deserialize(std::span<const std::uint8_t> bytes, Match& m);

} // namespace iron::sim
