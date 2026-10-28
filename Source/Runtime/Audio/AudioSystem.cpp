#include "Audio/AudioSystem.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace iron {

AudioSystem::AudioSystem() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO) || !MIX_Init()) {
        SDL_Log("Audio: disabled (%s)", SDL_GetError());
        return;
    }
    mixer_ = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer_) SDL_Log("Audio: no playback device (%s)", SDL_GetError());
}

AudioSystem::~AudioSystem() {
    for (auto& [_, audio] : sounds_) MIX_DestroyAudio(audio);
    if (mixer_) MIX_DestroyMixer(mixer_);
    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioSystem::load(const std::string& id, const vfs::Bytes& encoded) {
    if (!mixer_) return;
    SDL_IOStream* io = SDL_IOFromConstMem(encoded.data(), encoded.size());
    // predecode: short effects are decoded to PCM once, at load time.
    MIX_Audio* audio = MIX_LoadAudio_IO(mixer_, io, true, true);
    if (!audio) {
        SDL_Log("Audio: cannot decode '%s' (%s)", id.c_str(), SDL_GetError());
        return;
    }
    if (auto [it, inserted] = sounds_.emplace(id, audio); !inserted) {
        MIX_DestroyAudio(it->second);
        it->second = audio;
    }
}

void AudioSystem::play(std::string_view id) {
    if (!mixer_) return;
    if (const auto it = sounds_.find(std::string(id)); it != sounds_.end()) MIX_PlayAudio(mixer_, it->second);
}

} // namespace iron
