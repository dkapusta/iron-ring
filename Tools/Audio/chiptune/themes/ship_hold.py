""""Below Decks" — the Ship's Hold theme. E minor, ~150 BPM.

The hold of an impounded freighter: steel ribs, stacked crates, a hatch of
night sky far above. Tense and driving: eighth-note bass, off-beat stabs
like a hull ringing, a lead that climbs and falls back.

Form (bars of 16 rows, 6 frames per row):
  intro (2, plays once: the hull creaks) | A (8) | B (8) -> loop to A
"""

from __future__ import annotations

from ..instruments import BASS, KIT, LEAD, LEAD_WIDE, ROWS_PER_BAR, STAB, bars, bass_bar
from ..song import Score

TITLE = "Below Decks"
FRAMES_PER_ROW = 6

A_LEAD = bars(
    "e4:2 e4:2 g4:2 e4:2 b4:4 a4:4",
    "g4:2 f#4:2 e4:2 d4:2 e4:8",
    "e4:2 e4:2 g4:2 b4:2 d5:4 c5:4",
    "b4:4 a4:2 g4:2 f#4:8",
    "e5:4 d5:2 b4:2 c5:4 a4:4",
    "b4:4 g4:2 e4:2 f#4:8",
    "g4:2 a4:2 b4:2 c5:2 d5:4 c5:4",
    "b4:4 d#5:4 e5:8",
)
A_ROOTS = ["e", "e", "c", "d", "a", "e", "c", "b"]

B_LEAD = bars(
    "c5:8 b4:4 a4:4",
    "b4:8 g4:4 e4:4",
    "a4:8 g4:4 f#4:4",
    "g4:4 a4:4 b4:8",
    "c5:4 e5:4 d5:4 c5:4",
    "b4:4 d5:4 g5:8",
    "f#5:4 e5:4 d#5:4 c5:4",
    "b4:8 b4:4 d#5:4",
)
B_ROOTS = ["c", "g", "a", "e", "a", "g", "a", "b"]

DRUMS = bars(*(["k:2 h:2 s:2 h:2 k:2 k:2 s:2 h:2"] * 7 + ["k:2 s:2 s:2 s:2 k:2 s:1 s:1 s:2 c:2"]))


def stabs(roots: list[str]) -> str:
    return " ".join(f"r:2 {r}4:2 r:2 {r}4:2 r:2 {r}4:2 r:2 {r}4:2" for r in roots)


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse2", row, bars("e3:2 r:6 e3:2 r:6", "e3:2 r:6 b2:2 r:2 d3:2 r:2"), STAB)
    s.add("triangle", row, bars("e2:16", "e2:8 b2:8"), BASS)
    s.add("noise", row, bars("t:4 t:4 t:4 t:4", "t:4 t:4 s:2 s:2 s:2 s:2"), KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD)
    s.add("pulse2", row, stabs(A_ROOTS), STAB)
    s.add("triangle", row, " ".join(bass_bar(r) for r in A_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    s.add("pulse2", row, stabs(B_ROOTS), STAB)
    s.add("triangle", row, " ".join(bass_bar(r) for r in B_ROOTS), BASS)
    s.add("noise", row, DRUMS, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
