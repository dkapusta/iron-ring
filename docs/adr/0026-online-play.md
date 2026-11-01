# ADR-0026: Online play through a dedicated server, with rollback and room codes

- **Status:** Accepted
- **Date:** 2026-11-01

## Context

M4 makes Iron Ring playable online. The owner asked for:
- a dedicated server as a separate CMake target, with a short deployment guide;
- an ONLINE menu with a server address field (history, paste), a lobby and player names;
- an easy way to share a game with a friend on a server that other people also use.

The full design is in [docs/design/online.md](../design/online.md).

## Decision

- **Topology.** Both clients connect out to an `IronRingServer`, which relays fight inputs between them. Nobody needs port forwarding, and players never learn each other's IP. The extra hop costs the difference between player↔server and player↔player latency.
- **Netcode.** Rollback on top of the existing deterministic, copyable `sim::Match`:
  - input delay: AUTO (1–3 frames from the ping) or set in options;
  - up to 8 frames of prediction and re-simulation;
  - time sync by frame advantage.
- **Transport.** ENet (MIT) over UDP, fetched like the other dependencies:
  - lobby messages travel on a reliable channel;
  - fight inputs travel on an unreliable channel, with the last 8 frames in every packet;
  - messages are explicit and versioned.
- **Sessions:**
  - a server hosts many rooms of two players;
  - every room has a six-character code from a 31-letter alphabet (no `0 O 1 I L`), drawn by a CSPRNG;
  - rooms are private unless the host opts in to the OPEN ROOMS list;
  - an invite is one string, `SERVER:PORT/CODE`, copied and pasted through the clipboard.
- **Safety:**
  - every client sends its protocol version and a hash of the simulation data;
  - the server only pairs equal ones;
  - the server referees by default, running `sim::Match` on the confirmed inputs and comparing checksums every 30 ticks, so a desync names the client at fault;
  - rate limits, size caps and idle timeouts are built in;
  - there are no accounts and no persistent data beyond the log.
- **Code:**
  - `IronRingNet`, SDL-free: protocol, ENet wrapper, codes, invites;
  - `IronRingServer`, an SDL-free executable: lobby, rooms, relay, referee;
  - the simulation gains versioned serialization, a replay format and `sim::Rollback`;
  - the player gains the online scenes.
- **Not planned:**
  - spectators, rankings, accounts, a server browser and voice;
  - a default server, unless `GameSettings.json → defaultServer` is set.

## Consequences

- **Server needs:** a VPS with one open UDP port can host hundreds of rooms. With refereeing on, the server needs the game data from the same build.
- **Compatibility:** a client and server from different builds refuse each other with a clear message, never a silent desync.
- **Testing:**
  - the rollback core, protocol and parsing are unit-tested without a network;
  - CI runs a server and two headless clients through a full match.
- **Replays** come from the same serialization work, for local matches too.
- **Simulation:** it stays SDL-free and integer-only. Anything new in its state must also appear in `checksum()` and in serialization; the tests check both.
