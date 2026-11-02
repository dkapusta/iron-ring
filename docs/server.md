# Running an Iron Ring server

`IronRingServer` (`IronRingServer.exe` on Windows) is the dedicated server for online play ([ADR-0026](adr/0026-online-play.md)). It is a small console program with no window and no sound. It does four jobs:

- it holds the lobby;
- it gives each room a code;
- it relays fight inputs between the two players;
- it referees every fight with the game's own simulation.

Players connect to it with **ONLINE → SERVER `your.address:7727`**. One server hosts many rooms at once, and the traffic is tiny, so a 1-vCPU VPS with 512 MB is plenty for hundreds of rooms.

## 1. Get the server

| Source | What to do |
| --- | --- |
| **CI artifact** | Download `server-linux` or `server-windows` from a CI run of the build you want. Each holds the binary, a `server.cfg` and the `IronRing_Data` folder from the same build. |
| **Quick build** | On Linux, from a checkout: `./build-server.sh` builds the server and packs `IronRing_Data` next to it in `Build/server/Server/Release/`. `./build-server.sh --run` also starts it, and extra arguments go to the server (`--name "MY RING"`). On machines with little memory it builds one file at a time; if the compiler is still killed, add swap. Re-run it after every `git pull`. |
| **Build it** | `cmake --preset release && cmake --build --preset release --target IronRingServer`. The binary lands in `Build/release/Server/Release/`, next to `server.cfg.example`. It needs no SDL, so the `core` preset builds it too. |
| **Install it** | `cmake --install Build/release --component Server --prefix /opt/ironring-server` installs only the server. Copy `Build/release/Player/Release/IronRing_Data` next to it. |
| **Docker** | See §5. |

**Use the game data from the same build as your players' game.** The server only accepts players whose fighters and stages match its own, so a game and a server from different builds refuse each other with a clear message, never a silent desync.

## 2. The server folder

```text
/opt/ironring-server/
  IronRingServer          the program
  server.cfg              settings (copy of server.cfg.example)
  IronRing_Data/          the game's data folder, for refereeing
  server.log              written when log= is set
```

## 3. Settings

`server.cfg` sits next to the program, or wherever `--config <file>` points. Each setting can also be given on the command line as `--key value`, which overrides the file.

| Key | Default | Meaning |
| --- | --- | --- |
| `port` | 7727 | UDP port players connect to. |
| `bind` | (all) | Address to listen on, for machines with several. |
| `name` | IRON RING SERVER | Shown in the lobby (capitals; up to 40 characters). |
| `motd` | (empty) | One line shown under the name. `#` starts a comment in `server.cfg`, so it can't appear in the text. |
| `maxPlayers` | 256 | Connections at once. One more gets `SERVER IS FULL`. |
| `maxRooms` | 128 | Rooms at once. |
| `roomsPerAddress` | 3 | Rooms one IP may hold open. |
| `idleMinutes` | 30 | Rooms with no activity close. |
| `joinAttemptsPerMinute` | 5 | Wrong room codes one IP may try per minute. |
| `maxMessagesPerSecond` | 400 | A connection sending more is dropped. A game sends about 130 a second in a fight. 0 turns the limit off. |
| `verify` | 1 | Referee every fight with the game's simulation (needs `IronRing_Data`). With 0, the server only relays inputs and compares the two players' checksums and results. |
| `data` | IronRing_Data | The game data folder, relative to the program unless absolute. |
| `log` | (empty) | Also append the log to this file. The log always goes to stdout as well. |

`IronRingServer --help` lists them all, and `--version` prints the version and the protocol number.

## 4. Linux, with systemd

```sh
sudo useradd --system --home-dir /opt/ironring-server --shell /usr/sbin/nologin ironring
sudo mkdir -p /opt/ironring-server
sudo cp -r IronRingServer server.cfg IronRing_Data /opt/ironring-server/
sudo chown -R ironring:ironring /opt/ironring-server

sudo ufw allow 7727/udp            # and the same in any cloud firewall / security group

sudo cp Tools/Server/ironring-server.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now ironring-server
journalctl -u ironring-server -f   # the log
```

The unit restarts the server if it fails. It runs it as its own user, which can write only to the server folder. `systemctl stop` shuts it down cleanly, and players get `SERVER IS SHUTTING DOWN`.

## 5. Docker

From the repository root, with Docker Compose (`docker-compose.yml`):

```sh
docker compose up -d --build       # build the image from this checkout and start the server
docker compose logs -f             # the log
docker compose down                # stop it (players get SERVER IS SHUTTING DOWN)
```

After `git pull`, run `docker compose up -d --build` again. The image packs the game data from the same checkout, so it always matches games built from that checkout. The container restarts by itself unless you stop it, also after a reboot of the machine (when Docker starts at boot, as it does by default).

Settings go in a `.env` file next to `docker-compose.yml` (ignored by git), or in the environment:

```ini
IRONRING_NAME=MY RING
IRONRING_MOTD=FRIDAY NIGHT FIGHTS
IRONRING_PORT=7727          # the UDP port on the machine
IRONRING_BUILD_JOBS=1       # parallel compile jobs while building the image, about 1 GB of memory each
```

Without Compose:

```sh
docker build -f Tools/Server/Dockerfile -t ironring-server .
docker run -d --name ironring -p 7727:7727/udp --restart unless-stopped ironring-server --name "MY RING"
docker logs -f ironring
```

## 6. Windows

Put `IronRingServer.exe`, `server.cfg` and `IronRing_Data` in one folder and run the program. Allow it through Windows Defender Firewall for UDP 7727 when asked. Ctrl+C stops it cleanly.

To run it at startup without a session, create a Task Scheduler task with these settings:
- trigger: "At startup";
- "Run whether user is logged on or not";
- action: the full path to `IronRingServer.exe`, with "Start in" set to its folder.

## 7. Check it

1. Run `IronRingServer --check`. It loads the data, opens the port, prints what it would run and exits, with status 0 when all is well:

   ```text
   [2026-11-01 14:06:53] game data /opt/ironring-server/IronRing_Data: 7 fighters, 7 stages
   [2026-11-01 14:06:53] listening on UDP 7727, refereeing fights (7 fighters, 7 stages)
   [2026-11-01 14:06:53] check ok: 'IRON RING SERVER' on UDP 7727
   ```

2. From another network, in the game: **ONLINE → SERVER `your.address:7727` → CONNECT**. You should see the lobby with your server's name.

If players can't connect:
- the UDP port is closed somewhere: the host firewall, a cloud security group or a home router (forward UDP 7727 to the machine);
- or the server's `IronRing_Data` is from another build (the game says `DIFFERENT GAME DATA THAN THE SERVER`).

## 8. Updating

Replace `IronRingServer` and `IronRing_Data` together with the files from the new build, then restart. Players on an older game get `DIFFERENT GAME VERSION` until they update.

## 9. What the server keeps

The server has no accounts and no database.
- **The log:** connection times, IP addresses, player names, room codes and match results. Mention this to your players if your jurisdiction expects it.
- **Rooms:** they live only in memory and disappear when the server stops.
- **IP privacy:** players never see each other's IP addresses, because everything goes through the server.

## 10. Testing a server build

`Tools/Server/soak.sh` (Linux, after `cmake --build --preset release`) runs a real server and two headless game clients through match after match over a simulated bad link. It fails if the referee sees a desync, if too few matches finish, or if a saved replay does not play back the same. CI runs it on every change. The unit tests also run 100 refereed rooms at once against an in-process server and check every abuse limit.
