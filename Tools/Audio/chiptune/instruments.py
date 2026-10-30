"""Shared instruments, drum kit and score helpers for the themes and effects."""

from __future__ import annotations

from .song import Drum, Instrument

ROWS_PER_BAR = 16

LEAD = Instrument(volume=(14, 13, 12, 11, 10, 10, 9), duty=(1,), vibrato_delay=16,
                  vibrato_depth=0.18, vibrato_period=9, release=1)
LEAD_WIDE = Instrument(volume=(14, 13, 12, 11, 10, 10, 9), duty=(2,), vibrato_delay=16,
                       vibrato_depth=0.2, vibrato_period=9, release=1)
STAB = Instrument(volume=(15, 13, 11, 9, 7, 5, 4, 3, 2), duty=(2, 2, 1), release=1)
ECHO = Instrument(volume=(5, 5, 4, 4, 3), duty=(1,), vibrato_delay=16, vibrato_depth=0.18,
                  vibrato_period=9, release=1)


def chord(quality: str) -> Instrument:
    offsets = {"min": (0, 3, 7), "maj": (0, 4, 7), "sus": (0, 5, 7)}[quality]
    return Instrument(volume=(8, 7, 7, 6, 6, 5, 5, 5, 4), duty=(0,), arpeggio=offsets, arp_rate=2, release=1)


BASS = Instrument(volume=(15,), release=1)
KIT = {
    "k": Drum(((12, 15, False), (13, 12, False), (14, 8, False), (14, 5, False), (15, 3, False), (15, 1, False))),
    "s": Drum(((5, 15, False), (6, 13, False), (7, 11, False), (7, 9, False), (8, 7, False), (8, 5, False),
               (9, 4, False), (9, 3, False), (9, 2, False), (9, 1, False))),
    "h": Drum(((0, 7, False), (0, 4, False), (0, 2, False))),
    "o": Drum(tuple((1, max(1, 8 - i // 2), False) for i in range(12))),
    "c": Drum(((11, 15, False), (12, 13, False)) + tuple((3, max(1, 12 - i // 2), False) for i in range(22))),
    "t": Drum(((4, 13, True), (4, 10, True), (5, 7, True), (5, 4, True), (6, 2, True))),  # metallic tick
}


def bars(*lines: str) -> str:
    """Joins bar strings, checking that each one is exactly one bar long."""
    for line in lines:
        rows = sum(int(token.split(":")[1]) for token in line.split())
        if rows != ROWS_PER_BAR:
            raise ValueError(f"bar has {rows} rows, expected {ROWS_PER_BAR}: {line}")
    return " ".join(lines)


def bass_bar(root: str, approach: str | None = None) -> str:
    """Eighth notes on the root with octave jumps; optional approach note at the end."""
    low = root + "2"
    high = root + "3"
    last = approach or low
    return f"{low}:2 {low}:2 {high}:2 {low}:2 {low}:2 {low}:2 {high}:2 {last}:2"
