# ADR-0013: Generate all audio from an NES sound-chip model; music as looping Vorbis

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

The concept kit's placeholder WAVs were the last kit content in `Assets/`. The owner asked for retro NES-style sound effects and a music theme. The engine could only fire one-shot effects. Requirements: authentic chip character, reproducible files, small repository footprint, levels that hold up when effects overlap the music.

## Decision

- **Model the 2A03 sound channels in Python** (`Tools/Audio/chiptune/apu.py`) and write every sound as per-frame register data, as NES sound drivers do:
  - two pulse channels with the four duty cycles, the 4-bit triangle and the LFSR noise in long and short modes, using the hardware's period formulas and noise table;
  - the chip's non-linear mixer and Famicom-like output filtering;
  - rendering at 4× the output rate, decimated through an anti-alias filter.
- **Effects** are scripted functions (`sfx.py`); the **theme** is a small tracker score (`theme.py`) with per-frame instruments. Both are rendered at one shared gain.
- **Formats.** Effects ship as 44.1 kHz mono 16-bit WAV, byte-reproducible and decoded at load. The theme ships as Ogg Vorbis q5 (about 600 KB for 48 s), streamed. Lossless FLAC was measured at 2.9 MB even with a reference encoder, because the output filtering defeats FLAC's predictors.
- **Reproducibility.** Vorbis output differs between encoder versions, so `music.json` stores the SHA-256 of the rendered PCM. `generate_audio.py --check` (CI) compares effect bytes and that hash. The OGG is re-encoded only when the music changes.
- **Engine.** `AudioSystem` gains:
  - a looping music track with a loop point (`MIX_PROP_PLAY_LOOP_START_FRAME_NUMBER`), so the intro plays once;
  - an eight-track effect pool with its own gain;
  - `music-volume` and `sfx-volume` settings;
  - a soft limiter on the final mix.

  `FightScene` adds round-call, K.O., tech, slam and victory cues.

## Consequences

- No third-party or kit audio remains; all sound is reproducible from the repository and covered by its licence.
- Changing a sound means editing data in Python and regenerating, not editing audio files. Encoding music needs ffmpeg with libvorbis on the author's machine; CI and the build don't.
- The theme is decoded while it plays (not predecoded), so its compressed bytes stay in memory for the match.
- The sounds were verified by measurement (spectra, levels, loop continuity captured through SDL's disk audio driver), not by ear in this environment: the owner's listening pass is the real acceptance test.

## Alternatives considered

- **Keep WAV for the music**: 4.2 MB per revision in Git history.
- **FLAC (own encoder, implemented and measured)**: lossless and reproducible, but 2.9 MB.
- **Hand-made tracker modules (FamiTracker/NSF)**: SDL_mixer can't play them without an extra decoder, and they can't be generated or checked in CI.
- **Mix down effects into the music stream**: impossible; effects are event-driven.
