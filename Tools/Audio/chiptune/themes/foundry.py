""""Slag and Sparks" — The Foundry theme. G minor, ~129 BPM.

The shut-down Voll Works: heavy and mechanical. A hammering ostinato on
pulse 2 (the drop hammer), a lead that rises in fourths, the triangle
pounding the root in octaves, the metallic tick as an anvil on every
off-beat.

Form (bars of 16 rows, 7 frames per row):
  intro (2, plays once: the hammer alone) | A (8) | B (8) -> loop to A
"""

from __future__ import annotations

from ..instruments import BASS, KIT, LEAD_WIDE, ROWS_PER_BAR, STAB, bars
from ..song import Instrument, Score

TITLE = "Slag and Sparks"
FRAMES_PER_ROW = 7  # ~128 BPM at 16 rows per bar

HAMMER = Instrument(volume=(11, 8, 5, 3, 2), duty=(2, 1), release=1)

INTRO_HAMMER = bars("g3:2 g3:2 r:2 g3:2 g3:2 r:2 bb3:2 a3:2", "g3:2 g3:2 r:2 g3:2 g3:2 r:2 d4:2 c4:2")
INTRO_DRUMS = bars("k:4 t:2 t:2 k:4 t:2 t:2", "k:4 t:2 t:2 k:2 s:2 s:2 s:2")


def hammer_bar(root: str, top: str) -> str:
    return f"{root}3:2 {root}3:2 r:2 {root}3:2 {root}3:2 r:2 {top}:2 {root}3:2"


A_LEAD = bars(
    "g4:4 c5:4 d5:6 bb4:2",
    "a4:4 d5:4 g4:8",
    "g4:4 c5:4 eb5:6 d5:2",
    "c5:4 bb4:2 a4:2 d5:8",
    "g5:4 f5:2 eb5:2 d5:4 c5:4",
    "bb4:2 c5:2 d5:4 g4:8",
    "eb5:4 d5:2 c5:2 bb4:4 a4:4",
    "d5:8 f#4:4 a4:4",
)
A_ROOTS = [("g", "bb3"), ("g", "a3"), ("eb", "g3"), ("f", "a3"), ("g", "bb3"), ("g", "d4"), ("c", "eb3"),
           ("d", "f#3")]
A_DRUMS = bars(*(["c:4 t:2 t:2 s:4 t:2 t:2"] + ["k:4 t:2 k:2 s:4 t:2 t:2"] * 6 + ["k:2 s:2 k:2 s:2 s:1 s:1 s:2 o:4"]))

B_LEAD = bars(
    "bb4:6 c5:2 d5:4 f5:4",
    "eb5:6 d5:2 c5:8",
    "c5:6 d5:2 eb5:4 g5:4",
    "f5:4 eb5:2 d5:2 c5:8",
    "d5:4 g5:4 a5:4 bb5:4",
    "a5:4 g5:2 f5:2 d5:8",
    "eb5:4 d5:4 c5:4 a4:4",
    "d5:4 c5:2 bb4:2 a4:4 f#4:4",
)
B_ROOTS = [("eb", "g3"), ("c", "eb3"), ("ab", "c4"), ("f", "a3"), ("bb", "d4"), ("f", "a3"), ("c", "eb3"),
           ("d", "f#3")]
B_DRUMS = bars(*(["c:4 t:2 t:2 s:4 t:2 t:2"] + ["k:2 k:2 t:2 k:2 s:4 t:2 k:2"] * 6 + ["s:1 s:1 s:2 s:2 s:2 k:2 s:2 c:4"]))


def bass(roots: list[tuple[str, str]]) -> str:
    return " ".join(f"{r}2:2 {r}3:2 {r}2:2 {r}3:2 {r}2:2 {r}3:2 {r}2:2 {r}3:2" for r, _ in roots)


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse2", row, INTRO_HAMMER, HAMMER)
    s.add("noise", row, INTRO_DRUMS, KIT)
    s.add("triangle", row, bars("g2:2 g3:2 g2:2 g3:2 g2:2 g3:2 g2:2 g3:2", "g2:2 g3:2 g2:2 g3:2 d2:2 d3:2 d2:2 d3:2"),
          BASS)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD_WIDE)
    s.add("pulse2", row, " ".join(hammer_bar(r, top) for r, top in A_ROOTS), HAMMER)
    s.add("triangle", row, bass(A_ROOTS), BASS)
    s.add("noise", row, A_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    s.add("pulse2", row, " ".join(hammer_bar(r, top) for r, top in B_ROOTS[:7]), HAMMER)
    s.add("pulse2", row + 7 * ROWS_PER_BAR, "d3:2 d3:2 d3:2 d3:2 f#3:2 f#3:2 a3:2 c4:2", STAB)  # turnaround
    s.add("triangle", row, bass(B_ROOTS), BASS)
    s.add("noise", row, B_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
