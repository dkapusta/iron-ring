""""Crown at Dawn" — The Crown theme, the final bout. E minor, 180 BPM.

The last fight of the Iron Ring, on a hotel roof at first light: faster and
grander than the other venues. Power-chord hits in the intro, a galloping
bass, a lead doubled in parallel fifths in B.

Form (bars of 16 rows, 5 frames per row):
  intro (4, plays once: power-chord hits) | A (8) | B (8) | A' (8) | climb (4) -> loop to A

- pulse 1: lead (50 % duty, vibrato); intro power chords;
- pulse 2: shimmering fast arpeggios in A, the lead a fifth lower in B, chord roots in the intro;
- triangle: galloping bass (eighth + two sixteenths);
- noise: driving kit, crashes on section starts.
"""

from __future__ import annotations

from ..instruments import BASS, KIT, ROWS_PER_BAR, bars
from ..song import Instrument, Score

TITLE = "Crown at Dawn"
FRAMES_PER_ROW = 5

LEAD = Instrument(volume=(15, 14, 13, 12, 11, 11, 10), duty=(2,), vibrato_delay=12,
                  vibrato_depth=0.22, vibrato_period=7, release=1)
FIFTH = Instrument(volume=(8, 8, 7, 7, 6), duty=(1,), vibrato_delay=12, vibrato_depth=0.22,
                   vibrato_period=7, release=1)
HIT = Instrument(volume=(15, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4), duty=(2,),
                 vibrato_delay=20, vibrato_depth=0.15, vibrato_period=10, release=2)
HIT_LOW = Instrument(volume=(11, 11, 10, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3), duty=(1,), release=2)


def shimmer(quality: str) -> Instrument:
    offsets = {"min": (0, 3, 7, 12), "maj": (0, 4, 7, 12)}[quality]
    return Instrument(volume=(7, 6, 6, 5, 5, 5, 4), duty=(0,), arpeggio=offsets, arp_rate=1, release=1)


def gallop(root: str, octave_last: bool = True) -> str:
    """Eighth + two sixteenths, four times; the last group jumps an octave."""
    low, high = f"{root}2", f"{root}3"
    groups = [f"{low}:2 {low}:1 {low}:1"] * 3 + [f"{high if octave_last else low}:2 {low}:1 {low}:1"]
    return " ".join(groups)


DRIVE = "k:2 h:1 h:1 s:2 h:1 h:1 k:2 k:1 h:1 s:2 h:1 h:1"
DRIVE_CRASH = "c:2 h:1 h:1 s:2 h:1 h:1 k:2 k:1 h:1 s:2 h:1 h:1"
FILL = "k:2 s:1 s:1 s:1 s:1 s:2 k:1 k:1 s:1 s:1 s:1 s:1 s:1 s:1"

INTRO_HITS = bars("e5:12 r:4", "c5:12 r:4", "d5:12 r:4", "b4:8 d#5:4 f#5:4")
INTRO_ROOTS = bars("b4:12 r:4", "g4:12 r:4", "a4:12 r:4", "f#4:8 b4:4 d#5:4")
INTRO_BASS = bars("e2:12 r:4", "c2:12 r:4", "d2:12 r:4", "b1:2 b1:1 b1:1 b1:2 b1:1 b1:1 b2:2 b1:1 b1:1 b2:2 b1:1 b1:1")
INTRO_DRUMS = bars("c:12 k:2 k:2", "c:12 k:2 k:2", "c:12 k:2 s:2", FILL)

A_BARS = [
    "e5:4 g5:4 b5:6 a5:2",
    "g5:2 e5:2 g5:4 c6:6 b5:2",
    "a5:4 f#5:4 d5:4 a5:4",
    "b5:8 a5:2 g5:2 f#5:2 d#5:2",
    "e5:4 g5:4 b5:4 e6:4",
    "d6:2 c6:2 b5:2 c6:2 e6:6 c6:2",
    "a5:4 c6:2 e6:2 a6:6 g6:2",
    "f#6:8 d#6:4 b5:4",
]
A_CHORDS = [("e", "min"), ("c", "maj"), ("d", "maj"), ("b", "maj"),
            ("e", "min"), ("c", "maj"), ("a", "min"), ("b", "maj")]
B_BARS = [
    "d6:6 b5:2 g5:4 b5:4",
    "a5:6 f#5:2 d5:4 f#5:4",
    "g5:4 b5:4 e6:6 d6:2",
    "c6:8 b5:4 g5:4",
    "a5:2 b5:2 c6:2 e6:2 a6:8",
    "f#6:4 e6:2 d6:2 a5:4 d6:4",
    "d#6:8 f#6:4 b6:4",
    "a6:2 g6:2 f#6:2 e6:2 d#6:4 b5:4",
]
B_ROOTS = ["g", "d", "e", "c", "a", "d", "b", "b"]
CLIMB_LEAD = bars("g5:4 b5:4 d6:8", "a5:4 c6:4 e6:8", "b5:4 d#6:4 f#6:8", "b6:4 a6:2 g6:2 f#6:2 e6:2 d#6:4")
CLIMB_ROOTS = ["g", "a", "b", "b"]


def _shimmer(score: Score, row: int, progression: list[tuple[str, str]]) -> None:
    for root, quality in progression:
        score.add("pulse2", row, f"{root}4:8 {root}4:8", shimmer(quality))
        row += ROWS_PER_BAR


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse1", row, INTRO_HITS, HIT)
    s.add("pulse2", row, INTRO_ROOTS, HIT_LOW)
    s.add("triangle", row, INTRO_BASS, BASS)
    s.add("noise", row, INTRO_DRUMS, KIT)
    row += 4 * ROWS_PER_BAR
    loop = row

    a_drums = bars(DRIVE_CRASH, *[DRIVE] * 6, FILL)
    s.add("pulse1", row, bars(*A_BARS), LEAD)
    _shimmer(s, row, A_CHORDS)
    s.add("triangle", row, " ".join(gallop(r) for r, _ in A_CHORDS), BASS)
    s.add("noise", row, a_drums, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, bars(*B_BARS), LEAD)
    s.add("pulse2", row, bars(*B_BARS), FIFTH, transpose=-7)  # parallel fifths below
    s.add("triangle", row, " ".join(gallop(r) for r in B_ROOTS), BASS)
    s.add("noise", row, a_drums, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, bars(*A_BARS[:7], "e6:8 b5:4 g5:4"), LEAD)
    _shimmer(s, row, A_CHORDS[:7] + [("e", "min")])
    s.add("triangle", row, " ".join(gallop(r) for r, _ in A_CHORDS[:7]) + " " + gallop("e"), BASS)
    s.add("noise", row, a_drums, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, CLIMB_LEAD, LEAD)
    s.add("pulse2", row, CLIMB_LEAD, FIFTH, transpose=-7)
    s.add("triangle", row, " ".join(gallop(r, octave_last=False) for r in CLIMB_ROOTS), BASS)
    s.add("noise", row, bars(DRIVE_CRASH, DRIVE, DRIVE, FILL), KIT)
    row += 4 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
