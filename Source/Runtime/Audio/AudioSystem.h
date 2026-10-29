#pragma once

#include "Vfs/Mount.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

struct MIX_Mixer;
struct MIX_Audio;
struct MIX_Track;

namespace iron {

// SDL3_mixer front end with two mix groups: one-shot sound effects (a small
// pool of tracks, so effects overlap) and one looping music track. If no
// audio device can be opened (headless CI, -batchmode) it stays silent
// instead of failing.
class AudioSystem {
public:
    AudioSystem(float musicVolume, float sfxVolume);
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] bool available() const { return mixer_ != nullptr; }

    // Decodes a sound effect from memory (WAV, OGG, FLAC) under a short id.
    void load(const std::string& id, const vfs::Bytes& encoded);
    void play(std::string_view id);

    struct Music {
        vfs::Bytes encoded;            // kept alive: music is decoded while it plays
        std::int64_t loopStartFrame = 0; // sample frame the loop returns to (skips the intro)
        float gain = 1.0f;             // the track's own level, before musicVolume
    };
    // Starts (or restarts) the music from the beginning, looping forever.
    void playMusic(Music music);
    void stopMusic(int fadeOutMs);

private:
    static constexpr std::size_t kSfxTracks = 8;

    MIX_Mixer* mixer_ = nullptr;
    float musicVolume_ = 1.0f;
    float sfxVolume_ = 1.0f;
    std::unordered_map<std::string, MIX_Audio*> sounds_;
    std::array<MIX_Track*, kSfxTracks> sfx_{};
    std::size_t nextSfx_ = 0;
    MIX_Track* musicTrack_ = nullptr;
    MIX_Audio* musicAudio_ = nullptr;
    vfs::Bytes musicBytes_;
};

} // namespace iron
