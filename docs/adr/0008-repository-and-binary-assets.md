# ADR-0008: Binary assets in plain Git until a size threshold, then Git LFS

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

The initial commit carries about 14 MB of source art and audio in `Assets/` and 3.9 MB of JPEG references in `docs/`. Production sprite work will add many PNG revisions. Git LFS keeps clones small but needs installation on every machine and CI, and has hosting quotas.

## Decision

- Commit binary assets to plain Git for now; `.gitattributes` marks them `binary` (no diffs, no EOL conversion).
- Convert to **Git LFS** when either is true: the repository exceeds ~250 MB, or any single source file exceeds ~10 MB. The `.gitattributes` comment shows the one-line change; history migration via `git lfs migrate import`.
- Never commit generated files (`Build/`, packed archives, logs, screenshots).
- Large raw sources (layered PSD/Aseprite masters, audio stems) live outside the repository until LFS is adopted; only exported PNG/WAV/OGG enter `Assets/`.

## Consequences

- Zero setup for contributors today.
- A future LFS migration rewrites history; do it early in M1 if the art pass starts producing many large revisions.

## Alternatives considered

- **LFS from day one**: extra setup for a repository that is still small.
- **Separate asset repository/submodule**: coordination overhead with one contributor.
