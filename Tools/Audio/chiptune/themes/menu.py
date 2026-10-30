""""Invitation Only" — the front-end theme (title, select, options). A minor, 112 BPM.

Slower and sparser than the venue themes, so it sits under menu sounds and
the announcer. The A section quotes the main theme's opening motif
("Iron Ring", Pier 9: root, fifth below, root, third) in A minor.

Form (bars of 16 rows, 8 frames per row):
  intro (2, plays once: bass and ticks) | A (8) | B (8) -> loop to A

- pulse 1: lead (25 % duty, slow vibrato);
- pulse 2: slow arpeggiated pads, an echo of the lead in B;
- triangle: half-note bass with a walk into each bar;
- noise: half-time kit, the metallic tick on the off-beats.
"""

from __future__ import annotations

from ..instruments import BASS, ECHO, KIT, ROWS_PER_BAR, bars, chord
from ..song import Instrument, Score

TITLE = "Invitation Only"
FRAMES_PER_ROW = 8

LEAD = Instrument(volume=(12, 12, 11, 10, 10, 9, 9, 8), duty=(1,), vibrato_delay=20, vibrato_depth=0.16,
                  vibrato_period=11, release=1)

INTRO_BASS = bars("a2:8 a2:4 e2:4", "a2:8 a2:4 g2:2 g#2:2")
INTRO_DRUMS = bars("k:4 t:4 r:4 t:4", "k:4 t:4 k:2 s:2 t:2 t:2")

A_LEAD = bars(
    "a4:4 e4:2 a4:2 c5:6 b4:2",
    "a4:2 b4:2 e4:12",
    "f4:4 a4:2 c5:2 f5:6 e5:2",
    "d5:4 c5:2 b4:2 g4:8",
    "a4:4 e4:2 a4:2 c5:6 d5:2",
    "e5:6 d5:2 c5:4 a4:4",
    "f5:4 e5:2 d5:2 c5:4 b4:2 a4:2",
    "b4:8 g#4:4 e4:4",
)
A_CHORDS = [("a", "min"), ("a", "min"), ("f", "maj"), ("g", "maj"),
            ("a", "min"), ("c", "maj"), ("d", "min"), ("e", "maj")]
A_BASS = bars("a2:8 a2:4 c3:4", "a2:8 a2:4 g2:4", "f2:8 f2:4 a2:4", "g2:8 g2:4 b2:4",
              "a2:8 a2:4 e2:4", "c3:8 c3:4 g2:4", "d3:8 d3:4 a2:4", "e2:8 e2:4 g#2:4")
A_DRUMS = bars(*(["c:4 t:4 s:4 t:4"] + ["k:4 t:4 s:4 t:4"] * 6 + ["k:4 t:4 s:2 s:2 s:2 o:2"]))

B_LEAD = bars(
    "c5:6 d5:2 e5:4 c5:4",
    "d5:4 b4:4 g4:8",
    "a4:4 c5:2 e5:2 a5:6 g5:2",
    "f5:4 e5:2 d5:2 e5:8",
    "c5:6 d5:2 e5:4 g5:4",
    "f5:4 e5:4 d5:8",
    "c5:4 b4:2 a4:2 b4:4 c5:2 d5:2",
    "e5:8 b4:4 g#4:4",
)
B_BASS = bars("f2:8 f2:4 a2:4", "g2:8 g2:4 b2:4", "a2:8 a2:4 c3:4", "e2:8 e2:4 g2:4",
              "f2:8 f2:4 c3:4", "d3:8 d3:4 a2:4", "a2:8 a2:4 e2:4", "e2:8 e2:4 g#2:2 b2:2")
B_DRUMS = bars(*(["c:4 t:4 s:4 t:4"] + ["k:4 t:2 k:2 s:4 t:4"] * 6 + ["k:2 s:2 k:2 s:2 s:2 s:2 s:2 o:2"]))


def _pads(score: Score, row: int, progression: list[tuple[str, str]]) -> None:
    for root, quality in progression:
        score.add("pulse2", row, f"{root}4:16", chord(quality))
        row += ROWS_PER_BAR


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("triangle", row, INTRO_BASS, BASS)
    s.add("noise", row, INTRO_DRUMS, KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD)
    _pads(s, row, A_CHORDS)
    s.add("triangle", row, A_BASS, BASS)
    s.add("noise", row, A_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD)
    s.add("pulse2", row + 2, B_LEAD.rsplit(" ", 1)[0], ECHO)  # two-row echo, last note dropped
    s.add("triangle", row, B_BASS, BASS)
    s.add("noise", row, B_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
