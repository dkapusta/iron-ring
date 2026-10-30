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

// SDL3_mixer front end with three mix groups: one-shot sound effects (a small
// pool of tracks, so effects overlap), the announcer (one track: a new line
// cuts the previous one) and one looping music track. If no
// audio device can be opened (headless CI, -batchmode) it stays silent
// instead of failing.
class AudioSystem {
public:
    AudioSystem(float musicVolume, float sfxVolume, float voiceVolume);
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] bool available() const { return mixer_ != nullptr; }

    // Decodes a sound effect from memory (WAV, OGG, FLAC) under a short id.
    void load(const std::string& id, const vfs::Bytes& encoded);
    void play(std::string_view id);
    // Announcer lines: a separate set of ids and their own track and volume.
    void loadVoice(const std::string& id, const vfs::Bytes& encoded);
    void playVoice(std::string_view id);

    struct Music {
        std::string id;                // theme id; playMusic() of the theme already playing is a no-op
        vfs::Bytes encoded;            // kept alive: music is decoded while it plays
        std::int64_t loopStartFrame = 0; // sample frame the loop returns to (skips the intro)
        float gain = 1.0f;             // the track's own level, before musicVolume
    };
    // Starts the music from the beginning, looping forever, unless the same
    // theme is already playing.
    void playMusic(Music music);
    void stopMusic(int fadeOutMs);
    [[nodiscard]] const std::string& musicId() const { return musicId_; } // empty when stopped

    // Mix group levels, 0..1; applied at once, also to the music playing.
    void setVolumes(float music, float sfx, float voice);

private:
    static constexpr std::size_t kSfxTracks = 8;

    MIX_Mixer* mixer_ = nullptr;
    float musicVolume_ = 1.0f;
    float sfxVolume_ = 1.0f;
    float voiceVolume_ = 1.0f;
    std::unordered_map<std::string, MIX_Audio*> sounds_;
    std::unordered_map<std::string, MIX_Audio*> voices_;
    MIX_Track* voiceTrack_ = nullptr;
    std::array<MIX_Track*, kSfxTracks> sfx_{};
    std::size_t nextSfx_ = 0;
    MIX_Track* musicTrack_ = nullptr;
    MIX_Audio* musicAudio_ = nullptr;
    vfs::Bytes musicBytes_;
    std::string musicId_;
    float musicGain_ = 1.0f;
};

} // namespace iron
