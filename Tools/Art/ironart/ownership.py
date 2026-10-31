"""Which assets the procedural generator owns, and which come from outside.

Externally authored assets are never overwritten by generate.py and are not
compared with its output. Their editable sources live in ArtSource/ (see
ArtSource/README.md); fighters are packed from ArtSource/Fighters/<Name>/frames.
"""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
ART_SOURCE = ROOT / "ArtSource"

# Folders under Assets/ whose art is externally authored (Iron Ring Art Packs v1, v2 and v3).
EXTERNAL = {
    ("Fighters", "Dee"),
    ("Fighters", "Bram"),
    ("Fighters", "Sable"),
    ("Fighters", "Idris"),
    ("Fighters", "Greta"),
    ("Fighters", "Jun"),
    ("Fighters", "Broker"),
    ("Stages", "PierNine"),
    ("Stages", "LanternRow"),
    ("Stages", "TheCrown"),
    ("Stages", "Foundry"),
    ("Stages", "DryDock"),
    ("Stages", "ShipHold"),
    ("Stages", "BrokersClub"),
}

# Protect only the two external front-end pictures; fonts and HUD stay procedural.
EXTERNAL_FILES = {
    ("Resources", "UI", "title_logo.png"),
    ("Resources", "UI", "menu_backdrop.png"),
}


def is_external(relative: Path) -> bool:
    """True for a path relative to Assets/ inside an externally authored folder."""
    return tuple(relative.parts[:2]) in EXTERNAL or tuple(relative.parts) in EXTERNAL_FILES


def fighter_frames(fighter: str) -> Path:
    return ART_SOURCE / "Fighters" / fighter.capitalize() / "frames"
