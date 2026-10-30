# ADR-0014: A theme per venue, and an announcer through the DMC channel

- **Status:** Accepted
- **Date:** 2026-10-30

## Context

After ADR-0013 the game had one fight theme. The owner chose a theme per venue and an announcer voice ("ROUND / FIGHT / K.O."). The environment has no neural text-to-speech: Hugging Face model downloads are blocked, and there is no recorded voice actor. NES games spoke through the DMC channel's 1-bit delta samples.

## Decision

- **One theme per venue.**
  - Each is a module under `Tools/Audio/chiptune/themes/`; `stages.json` names it per stage.
  - `music.json` lists the themes by id, each with its own loop point.
  - `FightScene` plays the current stage's theme when a match starts and when the stage changes.
  - AssetPacker checks that every stage's theme exists.
  - Pier 9 keeps "Iron Ring" (bit-identical); Lantern Row gets "Curfew Market"; The Crown gets "Crown at Dawn".
- **The announcer.**
  - Lines are synthesised with **eSpeak NG** (pinned `espeakng-loader` wheel, ctypes, the "announcer" voice variant).
  - They are then processed exactly as the 2A03's DMC would play them: resampled to 33.1 kHz, delta-modulated to 1 bit (±2 steps on a 7-bit level), played through the DAC curve and the output filters.
  - At 11.2 kHz the 2-step slew limit destroyed intelligibility (tracking correlation 0.56); 33.1 kHz at 60 % level tracks at 0.91, which keeps the grit and the words.
- **A third mix group for voice**: one track (a new line cuts the previous one) and `voice-volume`. A small cue queue in `FightScene` delays the K.O. call until after the impact and the victory jingle until after the winner's call. The announcer replaces the round-call fanfare.
- `sim::isFinalRound()` moves out of the HUD so the banner and the announcer always agree.

## Consequences

- Each venue has its own mood. About 1.7 MB of OGG and 1 MB of voice WAV in total.
- eSpeak NG is GPL-3.0. It is a generation tool, like a recording studio, and only its rendered output ships. That output is not covered by the GPL, so the WAVs are under the project's zlib licence. Recorded in `THIRD_PARTY_NOTICES.md`.
- The voice is robotic by nature; the DMC treatment makes that read as period-appropriate rather than cheap. A recorded or neural voice can replace the source later without changing the pipeline: replace `Speaker.say` and keep the DMC stage.
- Byte-exact voice checks in CI depend on the pinned wheel producing the same speech on Linux x86-64.

## Alternatives considered

- **Neural TTS (Piper)**: better voice, but its models are unreachable here.
- **No voice, banners only**: the owner asked for an announcer.
- **Raw speech without the DMC stage**: clean, but out of place against the 2A03 music.
- **11.2 kHz DMC (the more common rate)**: unintelligible at the 2-step slew limit.
