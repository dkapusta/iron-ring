# Licensing options (decision pending)

Iron Ring will be **free and open source**. This note compares the realistic options so the owner can choose; nothing is licensed yet (all rights reserved until a `LICENSE` file lands). Not legal advice.

Two things need licences, and they usually get **different** ones:

1. **Code** — everything under `Source/`, `Tests/`, `cmake/`, build files.
2. **Content** — art, audio, character designs, writing (`Assets/`, `docs/design/`).

## Code licences

| Licence | What others must do when they reuse it | Patent grant | Copyleft | Fits Iron Ring? |
| --- | --- | --- | --- | --- |
| **MIT** | Keep the copyright + licence notice | No (implicit at best) | None | ✅ Simple, universally understood; the de-facto default |
| **zlib** | Don't misrepresent origin; mark altered versions; keep notice in *source* (not required in binaries) | No | None | ✅ Same licence as SDL3/SDL3_mixer; least friction for binary-only redistribution |
| **BSD-2-Clause** | Keep notice (source and binary docs) | No | None | ✅ Equivalent to MIT in practice |
| **Apache-2.0** | Keep notice + NOTICE file; state changes | **Yes**, with patent-retaliation clause | None | ✅ Best if you care about patents or corporate contributors; longer and more formal |
| **MPL-2.0** | Changes *to your files* must stay MPL; can be combined with closed code | Yes | File-level (weak) | ⚠️ Keeps engine improvements open without forcing it on whole games |
| **GPL-3.0** | Any distributed derivative must be GPL as a whole, with source | Yes | Strong | ⚠️ Keeps every fork open; blocks closed-source reuse and some console/store releases |

**Practical differences that matter here**

- *MIT vs zlib:* nearly identical freedoms. MIT requires the notice in copies "or substantial portions", which people read as including binaries; zlib only asks you not to misrepresent authorship and to mark modified versions. zlib matches SDL, which makes the whole dependency stack one family.
- *MIT vs Apache-2.0:* Apache adds an explicit patent licence and terminates it for anyone who sues over patents. Irrelevant for most hobby games, valued by companies. Apache code can't be combined into GPL-2.0-only projects (GPL-3.0 is fine).
- *Permissive vs copyleft:* permissive (MIT/zlib/BSD/Apache) lets anyone, including commercial studios, take the engine closed-source. Copyleft (MPL/GPL) forces improvements back into the open. GPL also prevents shipping a fork on platforms whose terms conflict with it (historically console SDKs, some app stores).
- All permissive options are compatible with every current dependency (SDL3, SDL3_mixer: zlib; miniz, nlohmann/json, doctest: MIT; GCC runtime exception; winpthreads: MIT-style).

**Recommendation:** **MIT** for code, as suggested, unless you'd like the zlib match with SDL (equally good) or the patent clause of Apache-2.0. If the wish is "nobody may close it", choose MPL-2.0 rather than GPL to stay engine-friendly.

## Content licences

| Licence | Others may | Must | Fits? |
| --- | --- | --- | --- |
| **CC BY 4.0** | Reuse, remix, sell | Credit the author | ✅ Open, keeps attribution |
| **CC BY-SA 4.0** | Reuse, remix, sell | Credit + share remixes under BY-SA | ✅ Open, keeps derivatives open |
| **CC0** | Anything | Nothing | ✅ Maximal reuse; you keep no control |
| **CC BY-NC(-SA)** | Reuse non-commercially | Credit (+ share-alike) | ⚠️ Not "open" by OSI/DFSG standards; distros/stores may reject |
| **All rights reserved** for characters/names + open art files | Mod the game, not reuse the IP | — | ⚠️ Common for "open-source engine, owned world" projects |

Things to know before choosing:

- **AI-generated art.** In several jurisdictions (including the US Copyright Office's current position) purely AI-generated images may not be copyrightable at all; a licence on them may be partly unenforceable, and the human-authored parts (prompts, selection, edits, character bios) are what's protected. CC0 or CC BY are the honest choices for such assets.
- **Names and logo** are better protected by trademark than copyright. Open code and art do not have to give away the right to call a fork "Iron Ring".
- **Third-party notices** (SDL, miniz, json, winpthreads) must ship with releases regardless of our choice (ROADMAP M5).

## Suggested combinations

| Goal | Code | Content |
| --- | --- | --- |
| Maximum openness, minimum paperwork | MIT | CC BY 4.0 |
| Match the SDL ecosystem | zlib | CC BY 4.0 |
| Keep improvements and remixes open | MPL-2.0 | CC BY-SA 4.0 |
| Open engine, protected world | MIT | CC BY-SA 4.0 for files + trademark policy for name/characters |

Once decided: add `LICENSE` (code) and `Assets/LICENSE` (content), an SPDX line in README, and `THIRD_PARTY_NOTICES.md`.
