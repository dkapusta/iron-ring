""""Curfew Market" — the Lantern Row theme. A minor (harmonic: G#), shuffle, ~129 BPM.

The night market after curfew: shutters down, lanterns half lit, the crowd
leaning in. A sneaky shuffle (dotted-eighth + sixteenth pairs) instead of
Pier 9's straight drive.

Form (bars of 16 rows, 7 frames per row):
  intro (2, plays once: lantern chimes) | A (8) | B (8) | A' (8) -> loop to A

- pulse 1: lead, 25 % duty in A, 50 % in B;
- pulse 2: lantern-chime arpeggios in the intro, off-beat chord stabs in A, a one-row echo in B;
- triangle: shuffle bass, root-fifth-octave;
- noise: shuffled kick / snare / hats, a metallic tick on the off-beats.
"""

from __future__ import annotations

from ..instruments import BASS, ECHO, KIT, LEAD, LEAD_WIDE, ROWS_PER_BAR, bars
from ..song import Instrument, Score

TITLE = "Curfew Market"
FRAMES_PER_ROW = 7

CHIME = Instrument(volume=(9, 7, 5, 4, 3, 2), duty=(0,), release=0)
SKANK_OFFSETS = {"min": (0, 3, 7), "maj": (0, 4, 7)}


def skank(quality: str) -> Instrument:
    """A short arpeggiated chord stab on the off-beat."""
    return Instrument(volume=(9, 8, 6, 4, 2, 1), duty=(0,), arpeggio=SKANK_OFFSETS[quality], arp_rate=1, release=2)


NOTE_STEPS = {"c": 0, "c#": 1, "d": 2, "d#": 3, "e": 4, "f": 5, "f#": 6, "g": 7, "g#": 8, "a": 9, "a#": 10, "b": 11}
NAMES = ["c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"]


def _up(note: str, octave: int, semitones: int) -> str:
    n = NOTE_STEPS[note] + semitones
    return f"{NAMES[n % 12]}{octave + n // 12}"


def shuffle_bass(root: str) -> str:
    """Root, fifth, octave, fifth in shuffled eighths (3 + 1 rows)."""
    low, fifth, high = f"{root}2", _up(root, 2, 7), f"{root}3"
    return f"{low}:3 {low}:1 {fifth}:3 {low}:1 {high}:3 {low}:1 {fifth}:3 {low}:1"


SHUFFLE_DRUMS = "k:3 h:1 t:3 h:1 s:3 h:1 t:3 k:1"

INTRO_CHIMES = bars("a5:1 c6:1 e6:1 a6:1 e6:1 c6:1 a5:2 r:8",
                    "g#5:1 b5:1 e6:1 g#6:1 e6:1 b5:1 g#5:2 r:4 e5:2 g#5:2")
INTRO_BASS = bars(shuffle_bass("a"), "e2:3 e2:1 b2:3 e2:1 e3:3 e2:1 g#2:3 b2:1")
INTRO_DRUMS = bars("k:4 h:3 h:1 t:4 h:3 h:1", "k:4 h:3 h:1 s:2 s:1 s:1 s:1 s:1 s:2")

A_BARS = [
    "e5:3 a5:1 r:2 g#5:2 a5:3 b5:1 c6:4",
    "b5:3 a5:1 e5:4 r:4 c5:3 e5:1",
    "f5:3 e5:1 f5:2 a5:2 c6:6 a5:2",
    "g#5:8 f5:3 e5:1 d5:3 b4:1",
    "e5:3 a5:1 r:2 g#5:2 a5:3 b5:1 c6:3 d6:1",
    "e6:4 d6:3 c6:1 b5:3 a5:1 g#5:4",
    "f5:3 a5:1 d6:4 c6:3 b5:1 a5:4",
    "b5:4 g#5:3 e5:1 b4:8",
]
A_LEAD = bars(*A_BARS)
A2_LEAD = bars(*A_BARS[:7], "a5:8 e5:4 a4:4")
A_CHORDS = [("a", "min"), ("a", "min"), ("f", "maj"), ("e", "maj"),
            ("a", "min"), ("a", "min"), ("d", "min"), ("e", "maj")]

B_LEAD = bars(
    "c6:6 a5:2 f5:4 a5:4",
    "b5:6 g5:2 d5:4 g5:4",
    "e5:3 g5:1 b5:4 e6:6 d6:2",
    "c6:8 b5:4 a5:4",
    "a5:3 f5:1 d5:4 f5:3 a5:1 d6:4",
    "b5:3 g#5:1 e5:4 g#5:3 b5:1 e6:4",
    "f6:4 e6:3 d6:1 c6:3 b5:1 a5:4",
    "g#5:8 e5:4 b4:4",
)
B_ROOTS = ["f", "g", "e", "a", "d", "e", "f", "e"]
B_DRUM_FILL = "k:3 s:1 s:3 s:1 k:3 s:1 s:1 s:1 s:1 s:1"


def _skanks(score: Score, row: int, progression: list[tuple[str, str]]) -> None:
    for root, quality in progression:
        # Off-beat stabs: silent on the beat, chord on the "and".
        score.add("pulse2", row, f"r:3 {root}4:1 r:3 {root}4:1 r:3 {root}4:1 r:3 {root}4:1", skank(quality))
        row += ROWS_PER_BAR


def build() -> tuple[Score, int]:
    s = Score(frames_per_row=FRAMES_PER_ROW)
    row = 0
    s.add("pulse2", row, INTRO_CHIMES, CHIME)
    s.add("triangle", row, INTRO_BASS, BASS)
    s.add("noise", row, INTRO_DRUMS, KIT)
    row += 2 * ROWS_PER_BAR
    loop = row

    a_bass = " ".join(shuffle_bass(root) for root, _ in A_CHORDS)
    a_drums = bars(*([SHUFFLE_DRUMS] * 7 + [B_DRUM_FILL]))
    s.add("pulse1", row, A_LEAD, LEAD)
    _skanks(s, row, A_CHORDS)
    s.add("triangle", row, a_bass, BASS)
    s.add("noise", row, a_drums, KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, B_LEAD, LEAD_WIDE)
    s.add("pulse2", row + 1, B_LEAD.rsplit(" ", 1)[0], ECHO)
    s.add("triangle", row, " ".join(shuffle_bass(r) for r in B_ROOTS), BASS)
    s.add("noise", row, bars(*([SHUFFLE_DRUMS] * 7 + [B_DRUM_FILL])), KIT)
    row += 8 * ROWS_PER_BAR

    s.add("pulse1", row, A2_LEAD, LEAD)
    _skanks(s, row, A_CHORDS[:7] + [("a", "min")])
    s.add("triangle", row, " ".join(shuffle_bass(root) for root, _ in A_CHORDS[:7]) + " " + shuffle_bass("a"), BASS)
    s.add("noise", row, a_drums, KIT)
    row += 8 * ROWS_PER_BAR
    s.length_rows = row
    return s, loop
