""""Gilt and Smoke" — the Broker's Club theme. C minor, swung, ~112 BPM.

The Broker's private club: velvet, brass and money that doesn't care who
bleeds. A lounge tune with teeth: a walking bass, a swung hi-hat, soft
chord comps on the off-beats and a lead that's too smooth to trust.

Form (bars of 16 rows, 8 frames per row):
  intro (2, plays once: the house pianist) | A (8) | B (8) -> loop to A
"""

from __future__ import annotations

from ..instruments import BASS, KIT, LEAD, LEAD_WIDE, ROWS_PER_BAR, bars, chord
from ..song import Score

TITLE = "Gilt and Smoke"
FRAMES_PER_ROW = 8

A_LEAD = bars(
    "g4:6 eb4:2 g4:4 c5:4",
    "bb4:3 ab4:3 g4:2 f4:8",
    "ab4:6 f4:2 ab4:4 d5:4",
    "c5:3 bb4:3 ab4:2 g4:8",
    "g4:6 eb4:2 g4:4 c5:4",
    "eb5:3 d5:3 c5:2 bb4:4 ab4:4",
    "g4:4 f4:4 eb4:4 d4:4",
    "c4:8 r:4 g4:4",
)
A_ROOTS = [("c", "min"), ("bb", "maj"), ("f", "min"), ("g", "maj"), ("c", "min"), ("ab", "maj"), ("f", "min"),
           ("g", "maj")]

B_LEAD = bars(
    "eb5:6 d5:2 eb5:4 g5:4",
    "f5:6 eb5:2 d5:8",
    "d5:6 c5:2 d5:4 f5:4",
    "eb5:6 d5:2 c5:8",
    "ab4:6 bb4:2 c5:4 eb5:4",
    "d5:4 c5:4 b4:8",
    "c5:4 eb5:4 g5:4 f5:4",
    "eb5:4 d5:4 b4:8",
)
B_ROOTS = [("eb", "maj"), ("bb", "maj"), ("g", "min"), ("c", "min"), ("ab", "maj"), ("g", "maj"), ("f", "min"),
           ("g", "maj")]

DRUMS = bars(*(["k:3 h:1 s:3 h:1 k:3 h:1 s:3 h:1"] * 7 + ["k:3 h:1 s:3 s:1 s:2 s:2 s:2 c:2"]))


def walk(roots: list[tuple[str, str]]) -> str:
    return " ".join(f"{r}2:4 {r}3:4 {r}2:2 {r}2:2 {r}3:4" for r, _ in roots)


def comps(s: Score, row: int, roots: list[tuple[str, str]]) -> None:
    for i, (r, quality) in enumerate(roots):
        s.add("pulse2", row + i * ROWS_PER_BAR, f"r:2 {r}4:6 r:2 {r}4:6", chord(quality))


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse1", row, bars("g4:3 r:1 eb4:3 r:1 c4:8", "d4:3 r:1 f4:3 r:1 b3:8"), LEAD)
    s.add("triangle", row, bars("c2:16", "g2:16"), BASS)
    s.add("noise", row, bars("h:3 h:1 h:3 h:1 h:3 h:1 h:3 h:1", "h:3 h:1 h:3 h:1 s:2 s:2 s:2 s:2"), KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD)
    comps(s, row, A_ROOTS)
    s.add("triangle", row, walk(A_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    comps(s, row, B_ROOTS)
    s.add("triangle", row, walk(B_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
