#pragma once

#include "Vfs/Mount.h"

#include <string>
#include <string_view>
#include <unordered_map>

struct MIX_Mixer;
struct MIX_Audio;

namespace iron {

// SDL3_mixer front end for one-shot sound effects. If no audio device can be
// opened (headless CI, -batchmode) it stays silent instead of failing.
class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] bool available() const { return mixer_ != nullptr; }

    // Decodes a sound from memory (WAV, OGG, FLAC) under a short id.
    void load(const std::string& id, const vfs::Bytes& encoded);
    void play(std::string_view id);

private:
    MIX_Mixer* mixer_ = nullptr;
    std::unordered_map<std::string, MIX_Audio*> sounds_;
};

} // namespace iron
