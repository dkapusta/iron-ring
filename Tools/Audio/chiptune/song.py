"""Tracker-style sequencing on top of apu.Channels.

A track is a line of tokens, one per note, durations in rows (one row =
`frames_per_row` driver frames; 6 frames = a 16th note at 150 BPM):

    "d5:4 a4:2 r:2 c#5:4 ~:2"

- `<note><octave>:<rows>` plays a note (C4 = MIDI 60; sharps with #, flats with b);
- `r:<rows>` is a rest, `~:<rows>` holds the previous note;
- on the noise channel the note is a drum name from the kit (`k:2 h:2 s:2`).

Instruments are per-frame tables, as in NES sound drivers: a volume envelope
(the last value sustains), a duty sequence, an arpeggio (semitone offsets
cycled every `arp_rate` frames: the classic chiptune chord) and a delayed
vibrato. `release` frames at the end of each note are silent, so repeated
notes articulate.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field

import numpy as np

from .apu import Channels, NOISE_PERIODS, note_frequency, pulse_period, triangle_period  # noqa: F401

NOTE_INDEX = {"c": 0, "d": 2, "e": 4, "f": 5, "g": 7, "a": 9, "b": 11}
TOKEN = re.compile(r"^(?P<name>[a-g][#b]?\d|r|~|[a-z]+):(?P<rows>\d+)$")


def midi(name: str) -> int:
    """'c#5' -> 73."""
    note = NOTE_INDEX[name[0]]
    rest = name[1:]
    if rest.startswith("#"):
        note, rest = note + 1, rest[1:]
    elif rest.startswith("b"):
        note, rest = note - 1, rest[1:]
    return 12 * (int(rest) + 1) + note


@dataclass(frozen=True)
class Instrument:
    volume: tuple[int, ...] = (12,)
    duty: tuple[int, ...] = (2,)
    arpeggio: tuple[int, ...] = (0,)
    arp_rate: int = 1
    vibrato_delay: int = 0          # frames before vibrato starts; 0 = no vibrato
    vibrato_depth: float = 0.0      # semitones
    vibrato_period: int = 8         # frames
    release: int = 1
    pitch: tuple[float, ...] = (0.0,)  # per-frame semitone offset (slides, kicks); last value holds


@dataclass(frozen=True)
class Drum:
    """Noise channel hit: per-frame (period index, volume, short mode)."""
    frames: tuple[tuple[int, int, bool], ...]


def _at(table: tuple, i: int):
    return table[min(i, len(table) - 1)]


@dataclass
class Score:
    frames_per_row: int = 6
    events: list[tuple[str, int, int, object, object]] = field(default_factory=list)  # (channel, start, rows, inst, pitch)
    length_rows: int = 0

    def add(self, channel: str, start_row: int, text: str, instrument: object, transpose: int = 0) -> int:
        """Adds a track line at start_row; returns the row after it."""
        row = start_row
        for token in text.split():
            m = TOKEN.match(token)
            if not m:
                raise ValueError(f"bad token '{token}'")
            name, rows = m.group("name"), int(m.group("rows"))
            if name == "~":
                if not self.events or self.events[-1][0] != channel:
                    raise ValueError("'~' must follow a note on the same channel")
                ch, st, r, inst, pitch = self.events.pop()
                self.events.append((ch, st, r + rows, inst, pitch))
            elif name != "r":
                pitch = name if channel == "noise" else midi(name) + transpose
                self.events.append((channel, row, rows, instrument, pitch))
            row += rows
        self.length_rows = max(self.length_rows, row)
        return row

    def render_channels(self, extra_frames: int = 0) -> Channels:
        frames = self.length_rows * self.frames_per_row + extra_frames
        ch = Channels(frames)
        for channel, start_row, rows, inst, pitch in self.events:
            start = start_row * self.frames_per_row
            length = rows * self.frames_per_row
            if channel == "noise":
                self._drum(ch, start, length, inst[pitch] if isinstance(inst, dict) else inst)
            else:
                self._tone(ch, channel, start, length, inst, pitch)
        return ch

    @staticmethod
    def _tone(ch: Channels, channel: str, start: int, length: int, inst: Instrument, note: int) -> None:
        for i in range(length):
            f = start + i
            if f >= ch.frames:
                break
            sounding = i < length - inst.release
            semis = note + _at(inst.pitch, i) + inst.arpeggio[(i // inst.arp_rate) % len(inst.arpeggio)]
            if inst.vibrato_delay and i >= inst.vibrato_delay:
                phase = (i - inst.vibrato_delay) / inst.vibrato_period
                semis += inst.vibrato_depth * np.sin(2 * np.pi * phase)
            freq = note_frequency(semis)
            volume = _at(inst.volume, i) if sounding else 0
            if channel == "triangle":
                ch.triangle_period[f] = triangle_period(freq)
                ch.triangle_on[f] = sounding and volume > 0
            else:
                p = 0 if channel == "pulse1" else 1
                ch.pulse_period[p, f] = pulse_period(freq)
                ch.pulse_duty[p, f] = _at(inst.duty, i)
                ch.pulse_volume[p, f] = volume

    @staticmethod
    def _drum(ch: Channels, start: int, length: int, drum: Drum) -> None:
        for i in range(min(length, len(drum.frames))):
            f = start + i
            if f >= ch.frames:
                break
            period, volume, short = drum.frames[i]
            ch.noise_period[f] = period
            ch.noise_volume[f] = volume
            ch.noise_short[f] = short
