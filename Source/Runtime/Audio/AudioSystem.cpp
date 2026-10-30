#include "Audio/AudioSystem.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cmath>
#include <utility>

namespace iron {
namespace {

// Safety limiter on the final mix: transparent below the knee, then a soft
// tanh shoulder up to full scale. Several impacts landing on the same frame
// over the music would otherwise clip.
constexpr float kKnee = 0.8f;

void SDLCALL softLimit(void*, MIX_Mixer*, const SDL_AudioSpec*, float* pcm, int samples) {
    constexpr float headroom = 1.0f - kKnee;
    for (int i = 0; i < samples; ++i) {
        const float x = pcm[i];
        const float magnitude = std::fabs(x);
        if (magnitude > kKnee)
            pcm[i] = std::copysign(kKnee + headroom * std::tanh((magnitude - kKnee) / headroom), x);
    }
}

} // namespace

namespace {

MIX_Audio* decode(MIX_Mixer* mixer, const std::string& id, const vfs::Bytes& encoded) {
    SDL_IOStream* io = SDL_IOFromConstMem(encoded.data(), encoded.size());
    // predecode: short sounds are decoded to PCM once, at load time.
    MIX_Audio* audio = MIX_LoadAudio_IO(mixer, io, true, true);
    if (!audio) SDL_Log("Audio: cannot decode '%s' (%s)", id.c_str(), SDL_GetError());
    return audio;
}

void store(std::unordered_map<std::string, MIX_Audio*>& map, const std::string& id, MIX_Audio* audio) {
    if (!audio) return;
    if (auto [it, inserted] = map.emplace(id, audio); !inserted) {
        MIX_DestroyAudio(it->second);
        it->second = audio;
    }
}

} // namespace

AudioSystem::AudioSystem(float musicVolume, float sfxVolume, float voiceVolume)
    : musicVolume_(musicVolume), sfxVolume_(sfxVolume), voiceVolume_(voiceVolume) {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO) || !MIX_Init()) {
        SDL_Log("Audio: disabled (%s)", SDL_GetError());
        return;
    }
    mixer_ = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer_) {
        SDL_Log("Audio: no playback device (%s)", SDL_GetError());
        return;
    }
    MIX_SetPostMixCallback(mixer_, softLimit, nullptr);
    for (auto& track : sfx_) track = MIX_CreateTrack(mixer_);
    musicTrack_ = MIX_CreateTrack(mixer_);
    voiceTrack_ = MIX_CreateTrack(mixer_);
}

AudioSystem::~AudioSystem() {
    // Tracks first: they reference the audio they play.
    for (auto* track : sfx_)
        if (track) MIX_DestroyTrack(track);
    if (musicTrack_) MIX_DestroyTrack(musicTrack_);
    if (voiceTrack_) MIX_DestroyTrack(voiceTrack_);
    for (auto& [_, audio] : voices_) MIX_DestroyAudio(audio);
    if (musicAudio_) MIX_DestroyAudio(musicAudio_);
    for (auto& [_, audio] : sounds_) MIX_DestroyAudio(audio);
    if (mixer_) MIX_DestroyMixer(mixer_);
    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioSystem::load(const std::string& id, const vfs::Bytes& encoded) {
    if (mixer_) store(sounds_, id, decode(mixer_, id, encoded));
}

void AudioSystem::loadVoice(const std::string& id, const vfs::Bytes& encoded) {
    if (mixer_) store(voices_, id, decode(mixer_, id, encoded));
}

void AudioSystem::playVoice(std::string_view id) {
    if (!voiceTrack_ || voiceVolume_ <= 0.0f) return;
    const auto it = voices_.find(std::string(id));
    if (it == voices_.end()) return;
    MIX_SetTrackAudio(voiceTrack_, it->second);
    MIX_SetTrackGain(voiceTrack_, voiceVolume_);
    MIX_PlayTrack(voiceTrack_, 0);
}

void AudioSystem::play(std::string_view id, float pitch) {
    if (!mixer_ || sfxVolume_ <= 0.0f) return;
    const auto it = sounds_.find(std::string(id));
    if (it == sounds_.end()) return;
    // A free track if there is one, otherwise the oldest effect is cut off.
    MIX_Track* track = nullptr;
    for (std::size_t i = 0; i < kSfxTracks && !track; ++i) {
        MIX_Track* candidate = sfx_[(nextSfx_ + i) % kSfxTracks];
        if (candidate && !MIX_TrackPlaying(candidate)) track = candidate;
    }
    if (!track) track = sfx_[nextSfx_];
    nextSfx_ = (nextSfx_ + 1) % kSfxTracks;
    if (!track) return;
    MIX_SetTrackAudio(track, it->second);
    MIX_SetTrackGain(track, sfxVolume_);
    MIX_SetTrackFrequencyRatio(track, pitch);
    MIX_PlayTrack(track, 0);
}

void AudioSystem::playMusic(Music music) {
    if (!mixer_ || !musicTrack_) return;
    if (!music.id.empty() && music.id == musicId_ && MIX_TrackPlaying(musicTrack_)) return;
    MIX_StopTrack(musicTrack_, 0);
    MIX_SetTrackAudio(musicTrack_, nullptr);
    if (musicAudio_) MIX_DestroyAudio(musicAudio_);
    musicAudio_ = nullptr;
    musicBytes_ = std::move(music.encoded);
    SDL_IOStream* io = SDL_IOFromConstMem(musicBytes_.data(), musicBytes_.size());
    // Not predecoded: a minute of music stays compressed and streams while it plays.
    musicAudio_ = MIX_LoadAudio_IO(mixer_, io, false, true);
    if (!musicAudio_) {
        SDL_Log("Audio: cannot decode music (%s)", SDL_GetError());
        return;
    }
    MIX_SetTrackAudio(musicTrack_, musicAudio_);
    musicGain_ = music.gain;
    musicId_ = music.id;
    MIX_SetTrackGain(musicTrack_, musicGain_ * musicVolume_);
    const SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOP_START_FRAME_NUMBER, music.loopStartFrame);
    if (!MIX_PlayTrack(musicTrack_, options)) SDL_Log("Audio: cannot play music (%s)", SDL_GetError());
    SDL_DestroyProperties(options);
}

void AudioSystem::stopMusic(int fadeOutMs) {
    musicId_.clear();
    if (!musicTrack_) return;
    MIX_StopTrack(musicTrack_, MIX_TrackMSToFrames(musicTrack_, fadeOutMs));
}

void AudioSystem::setVolumes(float music, float sfx, float voice) {
    musicVolume_ = music;
    sfxVolume_ = sfx;
    voiceVolume_ = voice;
    if (musicTrack_) MIX_SetTrackGain(musicTrack_, musicGain_ * musicVolume_);
}

} // namespace iron
