"""Sound effects, scripted per 60 Hz frame on the 2A03 channels.

Each effect is a function returning apu.Channels. Helpers write sweeps and
envelopes over frame ranges; everything is in frames (1/60 s).
"""

from __future__ import annotations

import numpy as np

from .apu import Channels, pulse_period, triangle_period
from .song import Drum, Instrument, Score
from .instruments import KIT


def _env(start: int, end: int, frames: int) -> np.ndarray:
    """Linear volume ramp start -> end over frames, rounded to 4-bit steps."""
    return np.clip(np.round(np.linspace(start, end, frames)), 0, 15).astype(np.int32)


def _glide(f0: float, f1: float, frames: int) -> np.ndarray:
    """Exponential frequency glide (sounds linear in pitch)."""
    return f0 * (f1 / f0) ** np.linspace(0, 1, frames)


def pulse(ch: Channels, which: int, at: int, freqs, volumes, duty: int) -> None:
    for i, (f, v) in enumerate(zip(freqs, volumes)):
        if at + i < ch.frames:
            ch.pulse_period[which, at + i] = pulse_period(float(f))
            ch.pulse_volume[which, at + i] = int(v)
            ch.pulse_duty[which, at + i] = duty


def triangle(ch: Channels, at: int, freqs) -> None:
    for i, f in enumerate(freqs):
        if at + i < ch.frames:
            ch.triangle_period[at + i] = triangle_period(float(f))
            ch.triangle_on[at + i] = True


def noise(ch: Channels, at: int, periods, volumes, short: bool = False) -> None:
    for i, (p, v) in enumerate(zip(periods, volumes)):
        if at + i < ch.frames:
            ch.noise_period[at + i] = int(p)
            ch.noise_volume[at + i] = int(v)
            ch.noise_short[at + i] = short


def light_hit() -> Channels:
    ch = Channels(10)
    noise(ch, 0, [4, 5, 5, 6, 6, 7], [14, 11, 8, 5, 3, 1])
    pulse(ch, 0, 0, _glide(700, 220, 5), [11, 9, 6, 3, 1], duty=2)
    return ch


def heavy_hit() -> Channels:
    ch = Channels(20)
    noise(ch, 0, [8, 9, 10, 11, 12, 12, 13, 13, 13, 13, 14, 14, 14], _env(15, 1, 13))
    triangle(ch, 0, _glide(220, 40, 11))
    pulse(ch, 0, 0, _glide(380, 80, 9), _env(13, 1, 9), duty=3)
    return ch


def guard() -> Channels:
    ch = Channels(10)
    pulse(ch, 0, 0, _glide(1900, 1500, 6), [12, 10, 7, 5, 3, 1], duty=0)
    pulse(ch, 1, 0, [2500, 2500, 2400], [7, 4, 2], duty=1)
    noise(ch, 0, [3, 3, 4, 4], [9, 6, 3, 1], short=True)
    return ch


def breakaway() -> Channels:
    ch = Channels(30)
    noise(ch, 0, np.round(np.linspace(9, 2, 24)).astype(int),
          np.concatenate([_env(4, 13, 8), _env(13, 0, 16)]))
    pulse(ch, 0, 0, _glide(180, 1700, 22), _env(12, 1, 22), duty=2)
    pulse(ch, 1, 2, _glide(270, 2500, 20), _env(7, 0, 20), duty=1)
    return ch


def tech() -> Channels:
    ch = Channels(16)
    for at in (0, 6):
        pulse(ch, 0, at, [2100, 2050, 2000, 1950], [13, 9, 5, 2], duty=1)
        noise(ch, at, [2, 2, 3], [10, 6, 3], short=True)
    pulse(ch, 1, 6, [2800, 2750, 2700, 2650, 2600], [8, 6, 4, 2, 1], duty=0)
    return ch


def slam() -> Channels:
    ch = Channels(36)
    noise(ch, 0, [13] * 4 + [14] * 8 + [15] * 12, _env(15, 1, 24))
    triangle(ch, 0, _glide(140, 30, 16))
    pulse(ch, 0, 0, _glide(200, 55, 10), _env(12, 0, 10), duty=3)
    noise(ch, 16, [12, 13, 13, 14, 14, 15, 15], [9, 7, 5, 4, 3, 2, 1])  # the bounce
    triangle(ch, 16, _glide(90, 35, 8))
    return ch


def round_bell() -> Channels:
    """Boxing bell, struck twice: two detuned pulses, slow decay."""
    ch = Channels(70)
    for at in (0, 13):
        frames = 56 if at else 13
        pulse(ch, 0, at, np.full(frames, 1318.5) * (1 + 0.004 * np.sin(np.arange(frames) / 2.2)),
              np.maximum(_env(15, 0, 56)[:frames], 0), duty=2)
        pulse(ch, 1, at, np.full(frames, 1979.0), _env(9, 0, 30)[:frames].tolist() + [0] * max(0, frames - 30),
              duty=0)
        noise(ch, at, [1, 2, 3], [9, 5, 2], short=True)
    return ch


def round_call() -> Channels:
    """Short brass-like call under the "ROUND n" banner."""
    ch = Channels(48)
    lead = [(0, 587.3, 5), (5, 880.0, 5), (10, 1174.7, 30)]
    for at, f, n in lead:
        vib = f * (1 + 0.006 * np.sin(np.arange(n) / 1.6) * (np.arange(n) > 8))
        pulse(ch, 0, at, vib, np.concatenate([_env(14, 11, min(n, 6)), _env(11, 0, max(n - 6, 0))])[:n], duty=2)
        pulse(ch, 1, at + 1, vib / 1.4983, np.concatenate([_env(8, 6, min(n, 6)), _env(6, 0, max(n - 6, 0))])[:n],
              duty=1)
    triangle(ch, 0, np.full(9, 146.8))
    triangle(ch, 10, np.full(28, 73.4))
    noise(ch, 0, KIT_FRAMES("s")[0], KIT_FRAMES("s")[1])
    noise(ch, 10, KIT_FRAMES("c")[0], KIT_FRAMES("c")[1])
    return ch


def menu_confirm() -> Channels:
    ch = Channels(12)
    pulse(ch, 0, 0, [1046.5] * 3 + [1568.0] * 7, [12, 11, 10, 12, 10, 8, 6, 4, 2, 1], duty=1)
    return ch


def ko() -> Channels:
    """A heavy impact, then a long falling tone over a rumble."""
    ch = Channels(84)
    noise(ch, 0, [9, 10, 11, 12, 12, 13], [15, 14, 12, 10, 9, 8])
    noise(ch, 6, [13] * 10 + [14] * 20 + [15] * 30, _env(10, 0, 60))
    triangle(ch, 0, _glide(240, 45, 50))
    frames = 64
    fall = _glide(988, 98, frames) * (1 + 0.015 * np.sin(np.arange(frames) / 1.3))
    pulse(ch, 0, 4, fall, _env(13, 0, frames), duty=2)
    pulse(ch, 1, 8, fall * 0.749, _env(7, 0, frames), duty=1)
    return ch


VICTORY_LEAD = Instrument(volume=(14, 13, 12, 11, 10, 10, 9), duty=(2,), vibrato_delay=10,
                          vibrato_depth=0.2, vibrato_period=8, release=1)
VICTORY_HARMONY = Instrument(volume=(9, 8, 8, 7, 7, 6), duty=(1,), vibrato_delay=10,
                             vibrato_depth=0.2, vibrato_period=8, release=1)
VICTORY_BASS = Instrument(volume=(15,), release=1)


def victory() -> Channels:
    """Match-winner jingle: the theme's D minor resolves to D major."""
    s = Score(frames_per_row=6)
    s.add("pulse1", 0, "d5:2 f#5:2 a5:2 d6:6 c6:2 a5:2 bb5:2 c6:2 d6:12", VICTORY_LEAD)
    s.add("pulse2", 0, "a4:2 d5:2 f#5:2 a5:6 a5:2 f5:2 g5:2 a5:2 f#5:12", VICTORY_HARMONY)
    s.add("triangle", 0, "d3:2 d3:2 d3:2 d2:6 f2:2 f2:2 g2:2 a2:2 d2:12", VICTORY_BASS)
    s.add("noise", 0, "c:6 r:6 k:2 k:2 s:2 s:2 c:12", KIT)
    return s.render_channels(extra_frames=24)


def KIT_FRAMES(name: str) -> tuple[list[int], list[int]]:  # noqa: N802 - mirrors the kit's names
    drum: Drum = KIT[name]
    return [p for p, _, _ in drum.frames], [v for _, v, _ in drum.frames]


EFFECTS = {
    "light_hit": light_hit,
    "heavy_hit": heavy_hit,
    "guard": guard,
    "breakaway": breakaway,
    "tech": tech,
    "slam": slam,
    "round_bell": round_bell,
    "round_call": round_call,
    "menu_confirm": menu_confirm,
    "ko": ko,
    "victory": victory,
}
