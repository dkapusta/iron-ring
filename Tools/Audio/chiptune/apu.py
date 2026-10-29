"""A frame-driven model of the NES (2A03) sound channels.

Music and effects are described per 60 Hz frame, the way NES sound drivers
work: each frame sets a channel's period register, duty and 4-bit volume.
The renderer turns those register frames into audio with the chip's real
behaviour where it is audible:

- pulse: 8-step duty sequences (12.5 / 25 / 50 / 75 %), period t gives
  f = CPU / (16 (t + 1)), periods below 8 are silent;
- triangle: 32-step 4-bit staircase, f = CPU / (32 (t + 1)), no volume
  control, holds its level when stopped;
- noise: 15-bit LFSR, long (32767) or short (93-step, metallic) mode, the
  NTSC period table;
- the non-linear DAC mixer from the 2A03 and Famicom-style output filters
  (high-pass 37 Hz, low-pass 14 kHz).

Rendering runs at 4x the output rate and decimates with an anti-alias filter.
Everything is deterministic: same frames, same samples.
"""

from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np
from scipy import signal

CPU = 1_789_773          # NTSC 2A03 clock, Hz
FRAME_RATE = 60          # driver tick, Hz (the game also runs at 60)
SAMPLE_RATE = 44_100
OVERSAMPLE = 4
SAMPLES_PER_FRAME = SAMPLE_RATE // FRAME_RATE  # 735

DUTY = np.array([
    [0, 1, 0, 0, 0, 0, 0, 0],  # 12.5 %
    [0, 1, 1, 0, 0, 0, 0, 0],  # 25 %
    [0, 1, 1, 1, 1, 0, 0, 0],  # 50 %
    [1, 0, 0, 1, 1, 1, 1, 1],  # 75 % (25 % inverted)
], dtype=np.float64)
TRIANGLE = np.concatenate([np.arange(15, -1, -1), np.arange(0, 16)]).astype(np.float64)
NOISE_PERIODS = [4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068]


def _lfsr(short: bool) -> np.ndarray:
    """Output sequence (1 = sound) of the noise shift register for one full period."""
    tap = 6 if short else 1
    reg, out = 1, []
    for _ in range(93 if short else 32767):
        out.append(0 if reg & 1 else 1)
        feedback = (reg ^ (reg >> tap)) & 1
        reg = (reg >> 1) | (feedback << 14)
    return np.array(out, dtype=np.float64)


LFSR = {False: _lfsr(False), True: _lfsr(True)}


def note_frequency(note: int) -> float:
    """MIDI note number -> Hz (A4 = 69 = 440 Hz)."""
    return 440.0 * 2 ** ((note - 69) / 12)


def pulse_period(freq: float) -> int:
    return max(0, min(0x7FF, round(CPU / (16 * freq) - 1)))


def triangle_period(freq: float) -> int:
    return max(0, min(0x7FF, round(CPU / (32 * freq) - 1)))


@dataclass
class Channels:
    """Register values per frame for the four tone channels. Index = frame."""
    frames: int
    pulse_period: np.ndarray = field(init=False)   # (2, frames)
    pulse_duty: np.ndarray = field(init=False)     # (2, frames) 0..3
    pulse_volume: np.ndarray = field(init=False)   # (2, frames) 0..15
    triangle_period: np.ndarray = field(init=False)
    triangle_on: np.ndarray = field(init=False)
    noise_period: np.ndarray = field(init=False)   # index into NOISE_PERIODS
    noise_short: np.ndarray = field(init=False)
    noise_volume: np.ndarray = field(init=False)

    def __post_init__(self) -> None:
        n = self.frames
        self.pulse_period = np.zeros((2, n), dtype=np.int32)
        self.pulse_duty = np.full((2, n), 2, dtype=np.int32)
        self.pulse_volume = np.zeros((2, n), dtype=np.int32)
        self.triangle_period = np.zeros(n, dtype=np.int32)
        self.triangle_on = np.zeros(n, dtype=bool)
        self.noise_period = np.zeros(n, dtype=np.int32)
        self.noise_short = np.zeros(n, dtype=bool)
        self.noise_volume = np.zeros(n, dtype=np.int32)


def _per_sample(values: np.ndarray, rate: int) -> np.ndarray:
    return np.repeat(values, rate)


def _phase_steps(step_per_frame: np.ndarray, start: float, rate: int) -> tuple[np.ndarray, float]:
    """Running phase in sequencer steps, continuous across frames."""
    steps = _per_sample(step_per_frame, rate)
    phase = start + np.cumsum(steps) - steps
    end = float(phase[-1] + steps[-1]) if len(phase) else start
    return phase, end


def render(ch: Channels) -> np.ndarray:
    """Register frames -> float samples at SAMPLE_RATE, roughly -1..1."""
    rate = SAMPLES_PER_FRAME * OVERSAMPLE
    fs = SAMPLE_RATE * OVERSAMPLE

    pulses = []
    for i in range(2):
        t = ch.pulse_period[i]
        audible = (t >= 8) & (ch.pulse_volume[i] > 0)
        steps = np.where(audible, CPU / (16.0 * (t + 1)) * 8 / fs, 0.0)
        phase, _ = _phase_steps(steps, 0.0, rate)
        duty = _per_sample(ch.pulse_duty[i], rate)
        level = DUTY[duty, np.floor(phase).astype(np.int64) % 8]
        pulses.append(level * _per_sample(np.where(audible, ch.pulse_volume[i], 0), rate))

    # Triangle: the sequencer stops (and holds its level) while the channel is off.
    t = ch.triangle_period
    running = ch.triangle_on & (t >= 2)
    steps = np.where(running, CPU / (32.0 * (t + 1)) * 32 / fs, 0.0)
    phase, _ = _phase_steps(steps, 0.0, rate)
    triangle = TRIANGLE[np.floor(phase).astype(np.int64) % 32]

    noise = np.zeros(len(triangle))
    clocks = np.array([CPU / NOISE_PERIODS[p] / fs for p in ch.noise_period])
    for short in (False, True):
        mask = ch.noise_short == short
        steps = np.where(mask, clocks, 0.0)
        phase, _ = _phase_steps(steps, 0.0, rate)
        seq = LFSR[short]
        level = seq[np.floor(phase).astype(np.int64) % len(seq)]
        noise += level * _per_sample(np.where(mask, ch.noise_volume, 0), rate)

    p = pulses[0] + pulses[1]
    pulse_out = np.where(p > 0, 95.88 / (8128.0 / np.maximum(p, 1e-9) + 100.0), 0.0)
    tnd = triangle / 8227.0 + noise / 12241.0
    tnd_out = np.where(tnd > 0, 159.79 / (1.0 / np.maximum(tnd, 1e-12) + 100.0), 0.0)
    mix = pulse_out + tnd_out

    # Famicom-like output: high-pass 37 Hz, low-pass 14 kHz (first order each).
    b, a = signal.butter(1, 37, "highpass", fs=fs)
    mix = signal.lfilter(b, a, mix)
    b, a = signal.butter(1, 14_000, "lowpass", fs=fs)
    mix = signal.lfilter(b, a, mix)
    return signal.decimate(mix, OVERSAMPLE, ftype="fir", zero_phase=True)


def to_pcm16(samples: np.ndarray, peak_dbfs: float) -> np.ndarray:
    """Normalise to a fixed peak and quantise to 16-bit."""
    peak = float(np.max(np.abs(samples))) or 1.0
    gain = 10 ** (peak_dbfs / 20) / peak
    return np.clip(np.round(samples * gain * 32767), -32768, 32767).astype(np.int16)
