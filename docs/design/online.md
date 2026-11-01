# M4 Online: proposal

- **Status:** Proposed, for the owner's review. When accepted, it becomes the M4 section of the ROADMAP and an ADR.
- **Date:** 2026-11-01

## 1. What players get

Two friends anywhere play a best-of-3 over the internet. It should feel like local versus: rollback netcode, so inputs respond immediately, and the same character select, stage and HUD.

**The connection.** They meet on a **server**, either a public one or one either of them runs with the new **Iron Ring Dedicated Server**.

**The rooms.** A server hosts many **rooms** at once. Each room is two players and gets a **room code**, such as `K7QX2M`. Strangers on the same server never land in your room unless you open it to them.

**Sharing a game is one string.** The host presses **COPY INVITE** and gets, for example:

```text
203.0.113.5:7727/K7QX2M
```

They send it through any messenger. The friend pastes it into **ONLINE → PASTE INVITE** and lands straight in the room. The parts:
- `SERVER:PORT` before the slash, and the room code after it;
- a host name works too: `RING.EXAMPLE.ORG:7727/K7QX2M`;
- it's case-insensitive, and the game shows it in capitals, which is all its font has.

## 2. Screens

The screens are the existing menu style (`MenuView`), drawn with the existing fonts.

### 2.1 Title → ONLINE

A new item between VERSUS CPU and TRAINING.

```text
            ONLINE
  NAME      DMITRY_______
  SERVER    203.0.113.5:7727        < RECENT >
  PASTE INVITE
  CONNECT
  BACK

  PASTE AN INVITE (SERVER/CODE) OR TYPE A SERVER ADDRESS
```

- **Name:**
  - 1–12 characters from the font: A–Z, 0–9, `.` and `-`.
  - Saved in `prefs.cfg`. On first visit it is empty, and CONNECT asks for it.
- **Server:**
  - **History:** left and right cycle through the last 8 servers, newest first (saved in `prefs.cfg`). The default port is 7727.
  - **Typing:** a keyboard types normally. Ctrl+V pastes, and the field sheds anything outside the charset.
  - **Pads:** they get an arcade-style character grid: A–Z, 0–9, `. : / -`, ←, OK.
- **PASTE INVITE:**
  - It reads the clipboard. If it holds `SERVER/CODE`, it fills SERVER, connects and joins the room in one press.
  - If it holds only an address, it fills SERVER.
  - Pasting an invite into the SERVER field does the same.
- **Errors** are plain one-line messages:
  - `SERVER NOT FOUND`;
  - `NO ANSWER FROM SERVER`;
  - `SERVER IS FULL`;
  - `DIFFERENT GAME VERSION (SERVER 0.4.0)`;
  - `ROOM NOT FOUND`;
  - `ROOM IS FULL`.

### 2.2 The server lobby

```text
   RING.EXAMPLE.ORG                     38 PLAYERS  12 ROOMS
   "WELCOME. NO SANDBAGGING."           (the server's message)

  CREATE ROOM
  JOIN WITH CODE  ______
  OPEN ROOMS
  BACK
```

- **CREATE ROOM:**
  - **Private** is the default: only the code gets you in.
  - **Open** lists the room under OPEN ROOMS for anyone on the server, so strangers can find a game.
- **JOIN WITH CODE:** six characters, typed or pasted.
- **OPEN ROOMS:** the waiting open rooms, each showing the host's name and how long they have waited. Pick one to join.

### 2.3 The room

```text
   ROOM K7QX2M                                   PRIVATE
   DMITRY            HOST       READY
   WAITING FOR A CHALLENGER...

  COPY INVITE        203.0.113.5:7727/K7QX2M
  MAKE OPEN / MAKE PRIVATE
  LEAVE
```

- **The code** is shown large, so it can also be read out over voice.
- **When the guest arrives:**
  - each side shows the other's name and ping (`PING 48`);
  - both press READY, then go to the **character select**;
  - each player picks only their own side, with the other's cursor shown live;
  - the stage is the host's pick, or RANDOM.
- **After the match:**
  - **REMATCH** starts when both agree;
  - **CHANGE FIGHTERS** returns to the select;
  - **LEAVE** returns to the lobby.
- **Leaving:** if one player leaves, the other stays in the room with the same code, so the friend can come back.

### 2.4 The fight

The fight is the normal fight screen, with a small `PING 48 · DELAY 2` under the timer.

| Situation | What players see |
| --- | --- |
| Connection hiccup | A short stall (a few frames) before anything worse. |
| Connection lost | `CONNECTION LOST`, then back to the room after 5 seconds; the match counts for no one. |
| Pause | No pause online. Escape offers `LEAVE MATCH?`. |
| Desync (§4.4) | `MATCH OUT OF SYNC`, reported to the server, back to the room. It should never happen; when it does, the logs say why. |

Options gain an **ONLINE** section:
- the input delay, AUTO or 0–4 frames;
- SHOW PING.

## 3. The dedicated server

### 3.1 What it is

`IronRingServer` (`IronRingServer.exe` on Windows) is a separate CMake target. It is a headless console program with no SDL, no window and no audio.

| Area | What it does |
| --- | --- |
| Lobby | Holds the lobby and the rooms. |
| Codes | Hands out room codes. |
| Relay | Forwards fight inputs between the two players. |
| Referee | Optionally referees: §4.4. |

One server holds many rooms. Traffic is tiny: inputs are a few bytes a frame, so a 1-vCPU, 512 MB VPS can host hundreds of rooms. One UDP port, 7727 by default, is all it needs open.

### 3.2 Configuration

The command line, or `server.cfg` next to it (the same `key=value` format as `boot.config`):

```text
port=7727
name=RING.EXAMPLE.ORG
motd=WELCOME. NO SANDBAGGING.
maxPlayers=256
maxRooms=128
roomsPerAddress=3        # rooms one IP may hold open
idleMinutes=30           # empty or idle rooms close
verify=1                 # referee fights with the game's own simulation (needs the game data)
data=./IronRing_Data     # the game data, for verify=1
log=server.log           # also printed to stdout, for journald / docker logs
```

It handles SIGINT and SIGTERM, or Ctrl+C on Windows, cleanly: players get `SERVER SHUTTING DOWN`.

### 3.3 Deployment guide (to ship as `docs/server.md`)

1. **Build:** `cmake --build --preset release --target IronRingServer`. CI also uploads the server binaries for Linux and Windows.
2. **Copy to the server:**
   - `IronRingServer`;
   - `server.cfg`;
   - for `verify=1`, the player's `IronRing_Data/` folder from the same build.
3. **Open the port:** UDP 7727, for example `ufw allow 7727/udp`, plus any cloud firewall or security group.
4. **Run it as a service:** the guide includes a 10-line systemd unit (`Restart=on-failure`, its own user). On Windows: a console window, or the Task Scheduler at startup.
5. **Docker, as an alternative:** a small `Dockerfile` (Debian slim, about 30 MB): `docker run -p 7727:7727/udp ironring-server`.
6. **Check it:**
   - `IronRingServer --check` binds the port, prints the version and exits;
   - in the game, ONLINE → SERVER `your.address:7727` → CONNECT shows the lobby.
7. **Update the server with the game.** A client and server of different protocol versions refuse each other with a clear message. A room only pairs clients with the same game data.

### 3.4 Abuse and privacy (built in, not configurable away)

- **No IP sharing:** players never see each other's IP address, because everything goes through the server.
- **Codes:**
  - six characters from a 31-character alphabet with no `0 O 1 I L`, about 887 million codes, picked by a CSPRNG;
  - an IP gets five wrong guesses a minute.
- **Limits:**
  - names are restricted to the charset;
  - packet sizes and rates are capped;
  - rooms per IP are limited;
  - idle rooms expire.
- **No accounts, no persistent data:** the log holds connection times and IP addresses only. The guide says so, so server operators can mention it.

## 4. How it works (outline)

### 4.1 Topology: rollback, relayed by the server

Both clients connect *out* to the server, and the server forwards each player's inputs to the other.

- **Advantages:**
  - **No setup for players:** nobody needs port forwarding, and NAT is no problem, because only the server needs an open port.
  - **IPs stay private.**
  - **Referee and replays:** the server can referee and record every fight.
- **Cost:** one extra hop, about the ping between the two players and the server instead of between each other. With a server near both players, that is a few milliseconds. Direct peer-to-peer with server-assisted hole punching can come later, behind the same protocol.

### 4.2 Transport: ENet

[ENet](http://enet.bespin.org/) is a small C library for UDP games, released under MIT and compatible with zlib. It provides:
- reliable and unreliable channels;
- connection handling and ping;
- support for every toolchain we build with: MSVC, MinGW, GCC and Clang.

It is pulled in with FetchContent like SDL. Lobby messages go reliable, fight inputs unreliable. Each fight input packet carries the last 8 frames of input and an acknowledgement, so a lost packet costs nothing. The messages are explicit, versioned binary messages (§4.5), not raw structs.

### 4.3 Rollback

The simulation is already deterministic, integer-only and copyable; a `Match` copy is a snapshot. A new **SDL-free `sim::Rollback`** session sits on top of it, unit-testable without a network:

- **Each tick:**
  - send the local input for tick *t* + delay;
  - predict the remote input as its last known value;
  - step and keep a snapshot.
- **Correction:** when a remote input differs from the prediction, restore the snapshot at that tick and re-simulate to now, up to 8 frames. Beyond that, wait briefly instead.
- **Input delay:** AUTO picks 1–3 frames from the ping (about RTT / 2 / 16.7 ms).
- **Time sync:** the client that runs ahead slows by one frame now and then, so neither drifts.
- **Events:** `StepEvents` are already transient. The fight scene keys presentation events by tick, so re-simulated ticks fire only new events (a sound plays once). A predicted hit that turns out not to have happened has its spark cancelled. A sound can't be unplayed; that is accepted, as in every rollback game.

### 4.4 Version safety and desyncs

- **Join checks:** every client sends its protocol version and a **content hash** of the game data that drives the simulation: fighter files, frame data and stages. The server rejects other protocol versions and only pairs equal hashes.
- **Checksums:** every 30 ticks, both clients send `Match::checksum()`.
- **With `verify=1`:** the server runs the same `sim::Match` from the confirmed inputs. It uses the same SDL-free library the game uses and is fast, since there are no graphics. It can then say *which* client went out of sync, and it decides the result.
- **Without it:** the server compares the two clients' checksums.

### 4.5 Serialization and replays

This covers the existing M4 item.
- **Snapshots:** `Match` and `Round` gain an explicit, versioned `write(Buffer&)` / `read(Buffer&)`, field by field. The network, replays and the server use it; rollback's in-memory snapshots can stay plain copies.
- **Replay files** hold:
  - a header: version, content hash, fighters, stage and the CPU seeds;
  - one input pair per tick.
- **Saving:** local and online matches save a replay to the user folder's `Replays/` (capped at the last 50), and `IronRing -replay <file>` plays one back.
- **Server copy:** the server can keep replays of refereed fights (off by default).

### 4.6 Code layout

| Target | What | Depends on |
| --- | --- | --- |
| `IronRingNet` (new, static) | protocol messages, the ENet wrapper, room codes, invite parsing | ENet, IronRingCore |
| `IronRingServer` (new, executable) | lobby, rooms, relay, referee, `server.cfg` | IronRingNet, IronRingSimulation, IronRingContent, IronRingVfs |
| `IronRingSimulation` | + `Rollback`, + serialization, + replay format | (unchanged: no SDL) |
| `IronRingEngine` (the player) | ONLINE, lobby and room scenes, the text field, online fights | + IronRingNet |

The server is SDL-free by construction. It builds on Linux and Windows, and in CI, like everything else.

### 4.7 Testing

- **Unit tests:**
  - `Rollback` against a simulated network (latency, jitter, loss, reordering): both sides must end with identical checksums;
  - invite and code parsing, and the protocol's round trip;
  - serialization round trips, plus a replay corpus of pinned checksums run on every compiler in CI (the existing M4 determinism item).
- **`-netsim 80,10,2`:** adds 80 ms latency, ±10 ms jitter and 2 % loss to a client, for manual testing.
- **A CI soak:**
  - start `IronRingServer`;
  - two headless clients (`IronRing -batchmode -autoplay -connect 127.0.0.1:7727 -room create` / `-room join <code>`) play a CPU-vs-CPU match through it under `-netsim`;
  - the server's referee must agree with both.

## 5. Proposed roadmap (M4)

| Step | Content |
| --- | --- |
| **M4.1 Foundations** | Versioned serialization of `Match`. Replay files and `-replay`. `sim::Rollback` with simulated-network tests. Event reconciliation in the fight scene. Cross-compiler replay-checksum tests. |
| **M4.2 Server** | `IronRingNet` (ENet, protocol, codes, invites). `IronRingServer`: lobby, rooms, relay, referee, `server.cfg`, signals, logs. `docs/server.md`, with the systemd unit and Dockerfile. Server binaries as CI artifacts. |
| **M4.3 Online front end** | The ONLINE menu. A text field: keyboard, pad grid, paste, history. The lobby and room screens. COPY / PASTE INVITE. `-connect`. |
| **M4.4 Online fights** | Online character and stage select. Rollback fights through the relay. Rematch. Disconnect and desync handling. The ping and delay HUD. Online options. `-netsim`. |
| **M4.5 Soak and hardening** | The CI soak: server plus two headless clients. A load test of 100 rooms. Abuse limits verified. |

Each step is one or two PRs, as in M3.

## 6. Decisions for the owner

1. **Relay through the server, not direct peer-to-peer** (§4.1). Recommended, for no port forwarding and private IPs. Direct can come later.
2. **Rooms are private by default, with an opt-in OPEN ROOMS list.** Alternatively, private only, or also a QUICK MATCH that pairs any two waiting players. QUICK MATCH is cheap to add once open rooms exist.
3. **The server referees by default (`verify=1`).** This needs the game data on the server. With it off, the server compares the clients' checksums only.
4. **A default server.** None, so players type an address. Alternatively, a server address shipped in `GameSettings.json` for a community server, if you plan to run one.
5. **HOST ON THIS PC:** the same server code run inside the game, for LAN parties or friends with port forwarding. Proposed as a small M4.5 extra, or later.
6. **Not in M4:** spectators, rankings, accounts, a server browser across servers, and voice. Candidates for later.
