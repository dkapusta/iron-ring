"""The ring announcer: eSpeak NG speech, played back through the NES DMC channel.

NES games spoke through the DMC (delta modulation) channel: 1-bit samples
that step a 7-bit output level up or down by 2, at one of sixteen fixed
rates. This module synthesises each line with eSpeak NG (libespeak-ng via
ctypes, from the espeakng-loader wheel), then encodes and decodes it exactly
as the DMC would and mixes it through the 2A03's DAC curve and output filters.
The crunch, the slew-limited consonants and the low sample rate are the point.

Only the rendered WAVs ship with the game; eSpeak NG (GPL-3.0) is a tool.
"""

from __future__ import annotations

import ctypes
from dataclasses import dataclass

import numpy as np
from scipy import signal

from . import apu

# NTSC DMC rates (Hz) by rate index. Index 15 (33.1 kHz) is what NES games used for clear
# speech: at 2 levels per sample, lower rates slew-limit the voice into mush
# (tracking 0.56 at 11.2 kHz vs 0.91 at 33.1 kHz, 60 % level; measured on these lines).
DMC_RATES = [4181.71, 4709.93, 5264.04, 5593.04, 6257.95, 7046.35, 7919.35, 8363.42,
             9419.86, 11186.1, 12604.0, 13982.6, 16884.6, 21306.8, 24858.0, 33143.9]
DMC_RATE_INDEX = 15

AUDIO_OUTPUT_SYNCHRONOUS = 2
ESPEAK_RATE, ESPEAK_VOLUME, ESPEAK_PITCH, ESPEAK_RANGE = 1, 2, 3, 4
POS_CHARACTER = 1
ESPEAK_CHARS_AUTO, ESPEAK_ENDPAUSE = 0, 0x1000


@dataclass(frozen=True)
class Line:
    text: str
    rate: int = 140      # words per minute
    pitch: int = 32      # 0-100
    range: int = 80      # intonation, 0-100
    level: float = 0.6   # share of the DMC's swing used at the peak (higher = more slew distortion)


class Speaker:
    """A minimal ctypes binding: synchronous synthesis into a buffer."""

    CALLBACK = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(ctypes.c_short), ctypes.c_int, ctypes.c_void_p)

    def __init__(self, voice: str = "en-us+announcer") -> None:
        import espeakng_loader  # imported here: only needed when (re)generating voice lines

        self.lib = ctypes.CDLL(espeakng_loader.get_library_path())
        self.lib.espeak_Initialize.restype = ctypes.c_int
        self.rate = self.lib.espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0,
                                               espeakng_loader.get_data_path().encode(), 0)
        if self.rate <= 0:
            raise RuntimeError("espeak-ng failed to initialise")
        self.chunks: list[np.ndarray] = []

        def collect(wav, count, _events):
            if count > 0:
                self.chunks.append(np.ctypeslib.as_array(wav, shape=(count,)).copy())
            return 0

        self._callback = self.CALLBACK(collect)  # keep a reference: espeak holds the pointer
        self.lib.espeak_SetSynthCallback(self._callback)
        if self.lib.espeak_SetVoiceByName(voice.encode()) != 0:
            raise RuntimeError(f"espeak-ng voice '{voice}' not found")

    def say(self, line: Line) -> tuple[np.ndarray, int]:
        for parameter, value in ((ESPEAK_RATE, line.rate), (ESPEAK_PITCH, line.pitch),
                                 (ESPEAK_RANGE, line.range), (ESPEAK_VOLUME, 100)):
            self.lib.espeak_SetParameter(parameter, value, 0)
        self.chunks = []
        text = line.text.encode()
        self.lib.espeak_Synth(text, len(text) + 1, 0, POS_CHARACTER, 0, ESPEAK_CHARS_AUTO | ESPEAK_ENDPAUSE,
                              None, None)
        self.lib.espeak_Synchronize()
        pcm = np.concatenate(self.chunks).astype(np.float64) / 32768 if self.chunks else np.zeros(1)
        return pcm, self.rate


def dmc_encode(samples: np.ndarray, start: int = 64) -> np.ndarray:
    """Greedy delta modulation: one bit per sample, the 7-bit level moves by 2 per bit.

    Returns the decoded output levels (0..127) the DMC would play back.
    """
    level = start
    out = np.empty(len(samples), dtype=np.int32)
    for i, target in enumerate(samples):
        if target > level and level <= 125:
            level += 2
        elif target < level and level >= 2:
            level -= 2
        out[i] = level
    return out


def render(speaker: Speaker, line: Line) -> np.ndarray:
    """One announcer line as float samples at apu.SAMPLE_RATE."""
    speech, rate = speaker.say(line)
    # Trim espeak's leading/trailing silence so the line lands on its cue.
    loud = np.nonzero(np.abs(speech) > 0.01)[0]
    speech = speech[max(0, loud[0] - 40):loud[-1] + 400] if len(loud) else speech

    dmc_rate = DMC_RATES[DMC_RATE_INDEX]
    b, a = signal.butter(4, min(dmc_rate * 0.45, 8000.0), "lowpass", fs=rate)
    speech = signal.lfilter(b, a, speech)
    count = int(round(len(speech) * dmc_rate / rate))
    speech = signal.resample(speech, count)
    speech = speech / (np.max(np.abs(speech)) or 1.0)
    # Gentle compression keeps quiet consonants above the DMC's 2-step floor.
    speech = np.sign(speech) * np.abs(speech) ** 0.7
    levels = dmc_encode(64 + speech * 63 * line.level)

    # Sample-and-hold at the DMC rate, through the 2A03 DAC curve and output filters.
    fs = apu.SAMPLE_RATE * apu.OVERSAMPLE
    hold = levels[np.minimum((np.arange(int(len(levels) * fs / dmc_rate)) * dmc_rate / fs).astype(np.int64),
                             len(levels) - 1)].astype(np.float64)
    dac = np.where(hold > 0, 159.79 / (1.0 / np.maximum(hold / 22638.0, 1e-12) + 100.0), 0.0)
    b, a = signal.butter(1, 37, "highpass", fs=fs)
    dac = signal.lfilter(b, a, dac - dac[0])
    b, a = signal.butter(1, 14_000, "lowpass", fs=fs)
    dac = signal.lfilter(b, a, dac)
    return signal.decimate(dac, apu.OVERSAMPLE, ftype="fir", zero_phase=True)


# The announcer's lines. Fighter lines are "<fighter id>_wins".
LINES = {
    "round_1": Line("Round one!"),
    "round_2": Line("Round two!"),
    "round_3": Line("Round three!"),
    "round_4": Line("Round four!"),
    "final_round": Line("Final round!", rate=130, pitch=28),
    "fight": Line("Fight!", rate=150, pitch=36, range=90),
    "ko": Line("K. O.!", rate=120, pitch=30, range=95),
    "double_ko": Line("Double K. O.!", rate=125, pitch=30, range=90),
    "time_over": Line("Time!", rate=130, pitch=34),
    "draw": Line("Draw game.", rate=130, pitch=30),
    "dee_wins": Line("Dee Marlowe wins!", rate=140, pitch=32),
    "bram_wins": Line("Bram Hollis wins!", rate=140, pitch=32),
}
