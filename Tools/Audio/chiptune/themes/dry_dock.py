""""Low Tide" — the Dry Dock theme. D minor, a rolling 3-3-2, ~112 BPM.

The flooded dry dock: water slapping the stepped walls, a hull on its keel
blocks. The bass rolls like a swell (3 + 3 + 2 rows), the lead is a shanty
turned sour, the triangle sits under it; a buoy bell opens it.

Form (bars of 16 rows, 8 frames per row):
  intro (2, plays once: the buoy bell) | A (8) | B (8) -> loop to A
"""

from __future__ import annotations

from ..instruments import BASS, KIT, LEAD, LEAD_WIDE, ROWS_PER_BAR, bars, chord
from ..song import Instrument, Score

TITLE = "Low Tide"
FRAMES_PER_ROW = 8

BELL = Instrument(volume=(10, 8, 7, 6, 5, 4, 3, 3, 2, 2, 1), duty=(0,), release=0)

A_LEAD = bars(
    "d4:6 f4:2 a4:4 d5:4",
    "c5:3 bb4:3 a4:2 g4:8",
    "f4:6 g4:2 a4:4 bb4:4",
    "a4:3 g4:3 f4:2 e4:8",
    "d4:6 f4:2 a4:4 d5:4",
    "e5:3 d5:3 c5:2 bb4:4 a4:4",
    "g4:4 bb4:4 a4:3 g4:3 f4:2",
    "e4:4 c#4:4 d4:8",
)
A_ROOTS = [("d", "min"), ("g", "min"), ("bb", "maj"), ("a", "maj"), ("d", "min"), ("c", "maj"), ("g", "min"),
           ("a", "maj")]

B_LEAD = bars(
    "f5:6 e5:2 d5:4 c5:4",
    "bb4:6 c5:2 d5:8",
    "c5:6 bb4:2 a4:4 g4:4",
    "a4:4 c5:4 f5:8",
    "g5:6 f5:2 e5:4 d5:4",
    "c5:3 d5:3 e5:2 f5:8",
    "e5:4 d5:4 c#5:4 bb4:4",
    "a4:8 c#5:4 e5:4",
)
B_ROOTS = [("bb", "maj"), ("bb", "maj"), ("f", "maj"), ("f", "maj"), ("g", "min"), ("a", "maj"), ("g", "min"),
           ("a", "maj")]

DRUMS = bars(*(["k:3 h:3 s:2 k:3 h:3 s:2"] * 7 + ["k:3 s:3 s:2 s:2 s:2 s:2 c:2"]))


def swell(roots: list[tuple[str, str]]) -> str:
    return " ".join(f"{r}2:3 {r}3:3 {r}2:2 {r}2:3 {r}3:3 {r}3:2" for r, _ in roots)


def pads(s: Score, row: int, roots: list[tuple[str, str]]) -> None:
    for i, (r, quality) in enumerate(roots):
        s.add("pulse2", row + i * ROWS_PER_BAR, f"{r}4:8 {r}4:8", chord(quality))


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse2", row, bars("a4:8 r:8", "d4:8 r:8"), BELL)
    s.add("triangle", row, bars("d2:16", "a2:8 d2:8"), BASS)
    s.add("noise", row, bars("h:8 h:8", "h:8 s:2 s:2 s:2 s:2"), KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD)
    pads(s, row, A_ROOTS)
    s.add("triangle", row, swell(A_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    pads(s, row, B_ROOTS)
    s.add("triangle", row, swell(B_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
