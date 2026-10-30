""""Iron Ring" — the fight theme. D minor, 150 BPM, NES 2A03 voices.

Form (bars of 16 rows, 6 frames per row):
  intro (2, plays once) | A (8) | B (8) | A' (8) | breakdown (4) -> loop to A

- pulse 1: lead (25 % duty, delayed vibrato); intro hook and breakdown stabs at 50 %;
- pulse 2: arpeggiated chords in A (12.5 %), a one-row echo of the lead in B;
- triangle: driving eighth-note bass with octave jumps;
- noise: kick / snare / hats, fills into each section.
"""

from __future__ import annotations

from .song import Drum, Instrument, Score

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


# --- the score ----------------------------------------------------------------------

INTRO_LEAD = bars("d4:2 r:2 d4:2 r:2 f4:2 r:2 g4:2 a4:2",
                  "d5:2 r:2 c5:2 r:2 a4:3 r:1 g4:2 a4:2")
INTRO_DRUMS = bars("c:4 h:2 h:2 k:2 h:2 s:2 h:2",
                   "k:2 h:2 s:2 k:2 s:1 s:1 s:2 s:1 s:1 s:2")
INTRO_BASS = bars(bass_bar("d"), "d2:2 d2:2 d3:2 d2:2 f2:2 f2:2 g2:2 a2:2")

A_LEAD = bars(
    "d5:4 a4:2 d5:2 f5:4 e5:2 d5:2",
    "c5:2 d5:2 a4:4 r:4 a4:2 c5:2",
    "d5:4 c5:2 bb4:2 f4:4 bb4:2 c5:2",
    "e5:6 d5:2 c5:4 g4:4",
    "d5:4 a4:2 d5:2 f5:4 g5:2 a5:2",
    "bb5:4 a5:2 g5:2 f5:4 e5:2 f5:2",
    "d5:4 f5:2 bb5:2 a5:4 g5:2 f5:2",
    "e5:8 c#5:4 a4:4",
)
A2_LEAD = bars(
    "d5:4 a4:2 d5:2 f5:4 e5:2 d5:2",
    "c5:2 d5:2 a4:4 r:4 a4:2 c5:2",
    "d5:4 c5:2 bb4:2 f4:4 bb4:2 c5:2",
    "e5:6 d5:2 c5:4 g4:4",
    "d5:4 a4:2 d5:2 f5:4 g5:2 a5:2",
    "bb5:4 a5:2 g5:2 f5:4 e5:2 f5:2",
    "d5:4 f5:2 bb5:2 a5:4 g5:2 e5:2",
    "f5:2 e5:2 c#5:2 e5:2 d5:8",
)
A_CHORDS = [("d", "min"), ("d", "min"), ("bb", "maj"), ("c", "maj"),
            ("d", "min"), ("d", "min"), ("bb", "maj"), ("a", "maj")]
A_BASS = bars(bass_bar("d"), bass_bar("d", "c3"), bass_bar("bb"), bass_bar("c"),
              bass_bar("d"), bass_bar("d", "c3"), bass_bar("bb"), bass_bar("a", "c#3"))
A_DRUMS = bars(*(["c:2 h:2 s:2 h:2 k:2 k:2 s:2 h:2"]
                 + ["k:2 h:2 s:2 h:2 k:2 k:2 s:2 h:2"] * 6
                 + ["k:2 h:2 s:2 k:2 s:1 s:1 s:1 s:1 s:2 o:2"]))

B_LEAD = bars(
    "g5:6 f5:2 d5:4 bb4:4",
    "c5:2 d5:2 bb4:4 g4:8",
    "a4:4 d5:4 f5:4 a5:4",
    "g5:2 f5:2 e5:2 f5:2 d5:8",
    "f5:4 d5:2 f5:2 bb5:4 a5:2 g5:2",
    "g5:4 e5:2 g5:2 c6:4 bb5:2 a5:2",
    "a5:8 g5:2 f5:2 e5:2 c#5:2",
    "e5:4 a4:4 c#5:4 e5:4",
)
B_BASS = bars(bass_bar("g"), bass_bar("g", "a2"), bass_bar("d"), bass_bar("d", "a2"),
              bass_bar("bb"), bass_bar("c"), bass_bar("a"), "a2:2 a2:2 a3:2 a2:2 c#3:2 e3:2 a3:2 c#3:2")
B_DRUMS = bars(*(["c:2 h:1 h:1 s:2 h:2 k:1 k:1 h:2 s:2 t:1 h:1"]
                 + ["k:2 h:1 h:1 s:2 h:2 k:1 k:1 h:2 s:2 t:1 h:1"] * 6
                 + ["k:2 s:1 s:1 s:2 s:1 s:1 k:2 s:1 s:1 s:1 s:1 s:1 s:1"]))

# Breakdown: bass and drums carry it, lead hits stabs, chords sustain.
BREAK_LEAD = bars("bb3:2 r:2 bb3:2 r:4 bb3:2 c4:2 r:2",
                  "c4:2 r:2 c4:2 r:4 c4:2 d4:2 r:2",
                  "d4:2 r:2 d4:2 r:4 f4:2 g4:2 r:2",
                  "a4:2 r:2 a4:2 r:2 c#5:2 r:2 e5:2 r:2")
BREAK_CHORDS = [("bb", "maj"), ("c", "maj"), ("d", "min"), ("a", "sus")]
BREAK_BASS = bars(bass_bar("bb"), bass_bar("c"), bass_bar("d"), "a2:2 a2:2 a3:2 a2:2 a2:2 g2:2 f2:2 e2:2")
BREAK_DRUMS = bars("c:4 r:4 k:2 k:2 s:4",
                   "k:4 h:2 h:2 k:2 k:2 s:4",
                   "k:2 h:2 s:2 h:2 k:2 k:2 s:2 h:2",
                   "s:1 s:1 s:1 s:1 s:1 s:1 s:1 s:1 k:2 s:1 s:1 s:2 s:2")


def _chords(score: Score, row: int, progression: list[tuple[str, str]], octave: int = 4) -> None:
    for root, quality in progression:
        # Re-strike each half bar so the pad breathes with the beat.
        score.add("pulse2", row, f"{root}{octave}:8 {root}{octave}:8", chord(quality))
        row += ROWS_PER_BAR


def build() -> tuple[Score, int]:
    """The score and its loop start, in rows."""
    s = Score(frames_per_row=6)
    row = 0
    s.add("pulse1", row, INTRO_LEAD, STAB)
    s.add("triangle", row, INTRO_BASS, BASS)
    s.add("noise", row, INTRO_DRUMS, KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    s.add("pulse1", row, A_LEAD, LEAD)
    _chords(s, row, A_CHORDS)
    s.add("triangle", row, A_BASS, BASS)
    s.add("noise", row, A_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    s.add("pulse2", row + 1, B_LEAD.rsplit(" ", 1)[0], ECHO)  # one-row echo, last note dropped
    s.add("triangle", row, B_BASS, BASS)
    s.add("noise", row, B_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, A2_LEAD, LEAD)
    _chords(s, row, A_CHORDS[:7] + [("d", "min")])
    s.add("triangle", row, A_BASS.rsplit(" ", 8)[0] + " " + bass_bar("a", "c#3"), BASS)
    s.add("noise", row, A_DRUMS, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, BREAK_LEAD, STAB)
    _chords(s, row, BREAK_CHORDS, octave=3)
    s.add("triangle", row, BREAK_BASS, BASS)
    s.add("noise", row, BREAK_DRUMS, KIT)
    row += 4 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
