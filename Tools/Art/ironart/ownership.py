"""Which assets the procedural generator owns, and which come from outside.

Externally authored assets are never overwritten by generate.py and are not
compared with its output. Their editable sources live in ArtSource/ (see
ArtSource/README.md); fighters are packed from ArtSource/Fighters/<Name>/frames.
"""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
ART_SOURCE = ROOT / "ArtSource"

# Folders under Assets/ whose art is externally authored (Iron Ring Art Pack v1).
EXTERNAL = {
    ("Fighters", "Dee"),
    ("Fighters", "Bram"),
    ("Stages", "PierNine"),
    ("Stages", "LanternRow"),
    ("Stages", "TheCrown"),
}


def is_external(relative: Path) -> bool:
    """True for a path relative to Assets/ inside an externally authored folder."""
    return tuple(relative.parts[:2]) in EXTERNAL


def fighter_frames(fighter: str) -> Path:
    return ART_SOURCE / "Fighters" / fighter.capitalize() / "frames"
