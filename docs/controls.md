# Controls and move lists

These are the default bindings. You can change them in **Options → Controls** (they are saved to `prefs.cfg`); the defaults live in [`ProjectSettings/InputManager.json`](../ProjectSettings/InputManager.json). The in-game **Move list** shows each side's moves with the buttons that side currently uses, on the keyboard or the pad, whichever that player pressed last: heavy punch on the character select, or **Move list** in the pause menu.

## Both players at a glance

| Action | Player 1 keyboard | Player 2 keyboard | Gamepad (either player) |
| --- | --- | --- | --- |
| Walk back / forward | A / D | ← / → | D-pad or left stick |
| Jump | W | ↑ | Up |
| Crouch | S | ↓ | Down |
| Light punch (LP) | F | Keypad 1 | X (west) |
| Heavy punch (HP) | G | Keypad 2 | Y (north) |
| Light kick (LK) | H | Keypad 3 | A (south) |
| Heavy kick (HK) | J | Keypad 0 | B (east) |
| Guard | Space, or hold back | Right Shift, or hold back | Right shoulder or right trigger, or hold back |
| Pause | Escape | Escape | Start |

The first gamepad connected is player 1, the second player 2; the keyboard works alongside them. Player 2's default keys need a numeric keypad; on a laptop without one, rebind them in Options → Controls.

### In menus

| | Player 1 | Player 2 | Gamepad | Either |
| --- | --- | --- | --- | --- |
| Move the cursor | W A S D | Arrow keys | D-pad or left stick | — |
| Confirm | F (light punch) | Keypad 1 (light punch) | A (south) | Return |
| Back | H (light kick) | Keypad 3 (light kick) | B (east) | Backspace, Escape |
| Move list (character select) | G (heavy punch) | Keypad 2 (heavy punch) | Y (north) | — |

On the character select, heavy punch opens the move list of the fighter under your cursor, in your keys (or your pad's buttons, if you used the pad last) and written as your side faces at the start of a round; any button closes it. Menus follow your bindings: if you move light punch to another key, that key confirms. On screens one player drives (title, Versus CPU and Training select, options), either player's keys work; on the 2 Players select each side moves its own cursor.

### Text fields (ONLINE)

Choose a field (your name, the server, a room code) to edit it. On a keyboard, type, use Backspace, and paste with Ctrl+V; Return keeps the text and Escape restores it. On a pad, Up/Down change the last character, Right adds one, Left removes one, A keeps and B restores. Names and codes are in the font's capitals; lower case is raised as you type.

## Playing online

You need an Iron Ring server to meet on. Anyone can run one ([server.md](server.md)), and the game ships with none unless `GameSettings.json` names a `defaultServer`.

1. **ONLINE** on the title menu. Enter your **NAME** (up to 12 characters) and the **SERVER** as `address:port` (the port defaults to 7727). Left/Right on SERVER steps through the servers you used before. **CONNECT**.
2. In the lobby, one of you chooses **CREATE ROOM**. The room shows a six-character code, such as `K7M2QX`, and **COPY INVITE** puts `address:port/K7M2QX` in the clipboard. Send that to your friend however you like.
3. Your friend chooses **ONLINE → PASTE INVITE**, or types it into **INVITE** (Return joins). Either connects and joins in one step. With the server already in SERVER, typing just the six-letter code into INVITE is enough, and someone already in the lobby can use **JOIN WITH CODE**.
4. With two players in the room, both go to the character select. Each of you steers only your own side, with any keys or pad. The room's host (who created it) also picks the venue. The fight starts when both have picked.
5. After a match: **REMATCH** (when both choose it), **CHANGE FIGHTERS** (back to the select) or **LEAVE ROOM**.

Rooms are private: only someone with the code can join. A host can choose **MAKE OPEN** to list the room under **OPEN ROOMS** in the lobby, for anyone on that server to join.

**In an online fight** nothing pauses: Start (or Escape) once shows *PRESS START AGAIN TO LEAVE THE MATCH*, and a second press within two seconds leaves. The line under the timers shows the ping and the input delay. If the opponent leaves, the connection drops or the two games disagree (a desync, caught by the server's referee), the fight stops with a message and you're back in the room after three seconds. Every finished online match is saved to `Replays/` like any other.

**Input delay** (Options → ONLINE DELAY): AUTO picks one to three frames from the ping. A higher delay means fewer rollbacks (corrections when the opponent's input arrives late); 0 feels most like playing offline on a good connection.

### Anywhere

F11 or Alt+Enter: fullscreen · F12: screenshot. In a fight: F3 training overlay (hit and hurt boxes, frame data, frame advantage); in training F4 resets positions and F6 changes what the dummy does. Developer keys: Tab (vs CPU → 2 players → CPU vs CPU), F2 (next stage), F5 (new match), F1 (debug overlay).

## Directions and facing

Motions are written in numpad notation **relative to the way the fighter faces**:

```text
7 8 9      ↖ ↑ ↗          7 = up-back      8 = up      9 = up-forward
4 5 6      ← · →   facing  4 = back         5 = neutral 6 = forward
1 2 3      ↙ ↓ ↘   right   1 = down-back    2 = down    3 = down-forward
```

Fighters always turn to face each other, so the keys for "forward" swap when they cross. Player 2 starts on the right, facing left, so their forward is ←. The tables below give each motion both ways; if the fighters swap sides, swap the left and right keys.

Each direction of a motion must follow the last within 12 ticks (0.2 s), and the button within 8 ticks of the last direction. Diagonals count: you can roll the stick or the keys.

## System moves (every fighter)

| Move | Input | Notes |
| --- | --- | --- |
| **Guard** | Guard button, or hold back | Blocks high and mid attacks standing; crouch-guard (guard + down, or down-back) also blocks lows. Not in the air. |
| **Throw** | LP + LK together, close | Each fighter has one (below). Can't be blocked; ground only. |
| **Throw tech** | LP + LK within 10 ticks of being grabbed | Breaks a normal throw. Command throws (Dock Hoist) can't be teched. |
| **Breakaway** | Guard + HP + HK while blocking, with 50 Heat | A guard cancel: only in blockstun. Spends half the Heat bar, ends your blockstun and staggers the attacker for 18 ticks (no damage), close enough to punish. Outside blockstun the buttons are just a heavy attack. The only thing Heat pays for. |

Heat builds by landing hits (7), taking hits (4), blocking (5) and having an attack blocked (2), and **lasts the whole match**: what you don't spend carries into the next round. There are no super or desperation moves.

### Counter hits, combos and cancels

- **Counter hit:** hitting a fighter while their attack is starting up or swinging (a trade doesn't count). It does 25 % more damage, adds 6 ticks of hitstun and a longer freeze, and the HUD calls **COUNTER**.
- **Combo:** each hit that lands while the opponent is still reeling from the last one. The HUD counts the hits ("3 HITS"). Damage scales down per hit: 100 %, 85 %, 70 %, 55 %, then 50 %.
- **Cancel:** light punches and light kicks marked *cancels* can be cut short by the special when they hit or are blocked. Enter the motion during the light attack and press the button within 6 ticks of its active frames ending. For example, Dee's jab into Rush Uppercut.
- **On block**, light attacks are roughly even (−1 to +2). Heavies, sweeps and specials stay well minus, so blocking them earns you a punish.

## Dee Marlowe — boxer / counter

Fast, precise, punishes whiffs. Inputs written for each side at the start of a round.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Stiff Jab | LP | F | Keypad 1 | 4 / 3 / 10 | 6 | Fastest button; cancels; −1 to +1 on block |
| Cross Counter | HP | G | Keypad 2 | 8 / 3 / 20 | 12 | Heavy |
| Shin Kick | LK | H | Keypad 3 | 7 / 4 / 15 | 9 | Cancels; −1 to +2 on block |
| Leg Sweep | HK | J | Keypad 0 | 12 / 5 / 23 | 14 | Low (crouch-guard it), knocks down |
| Clinch Toss | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 22 | 10 | Throw, techable |
| **Rush Uppercut** | **236 + P** (↓ ↘ → + either punch) | **S, S+D, D + F or G** | **↓, ↓+←, ← + Keypad 1 or 2** | 6 / 4 / 20 | 14 | Special: drives forward and rises; knocks down. Whiffs are very punishable |

## Bram Hollis — grappler / power

Slow advance, close-range reads, a command throw.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Palm Shove | LP | F | Keypad 1 | 4 / 3 / 10 | 6 | Cancels; −1 to +1 on block |
| Anchor Hook | HP | G | Keypad 2 | 10 / 3 / 23 | 16 | Heavy |
| Boot Kick | LK | H | Keypad 3 | 7 / 4 / 15 | 9 | Cancels; −1 to +2 on block |
| Hull Breaker | HK | J | Keypad 0 | 14 / 5 / 26 | 18 | Low (crouch-guard it), knocks down |
| Pier Toss | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 24 | 13 | Throw, techable |
| **Dock Hoist** | **63214 + P** (→ ↘ ↓ ↙ ← + either punch) | **D, S+D, S, S+A, A + F or G** | **←, ↓+←, ↓, ↓+→, → + Keypad 1 or 2** | 5 / 2 / 34 | 20 | Special: command throw with a longer reach than Pier Toss; can't be teched, only avoided (jump, or hit him first). Huge recovery on a miss |

## Sable Reyes — mobile kickboxer

Fast feet, a high, floaty jump and kicks from odd angles: a quick low kick, a head-high roundhouse, and a flying knee that crosses half the ring. Placeholder art for now.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Snap Jab | LP | F | Keypad 1 | 4 / 2 / 10 | 5 | Cancels; 0 to +1 on block |
| Rear Cross | HP | G | Keypad 2 | 9 / 3 / 19 | 11 | Heavy |
| Calf Kick | LK | H | Keypad 3 | 7 / 3 / 13 | 5 | Low (crouch-guard it); −3 to −1 on block |
| Wire Kick | HK | J | Keypad 0 | 11 / 4 / 21 | 12 | Head-high roundhouse; heavy, long reach |
| Rooftop Drop | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 22 | 10 | Throw, techable; lands the opponent far away |
| **Switch Step** | **236 + K** (↓ ↘ → + either kick) | **S, S+D, D + H or J** | **↓, ↓+←, ← + Keypad 3 or 0** | 7 / 4 / 26 | 10 | Special: a stance switch, a long step and a flying knee; knocks down. Cancels from Snap Jab. Very punishable on block |

## Idris Okafor — long-range striker

Long arms, longer legs, and the patience to wait at the end of them. Everything works at range; up close he's slow to recover. Placeholder art for now.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Long Jab | LP | F | Keypad 1 | 5 / 3 / 12 | 6 | Cancels; −2 to 0 on block |
| Hawser Straight | HP | G | Keypad 2 | 10 / 3 / 22 | 13 | Heavy, long |
| Mast Kick | LK | H | Keypad 3 | 9 / 4 / 20 | 10 | A push kick at the end of his reach: pushes far back, hit or block |
| Deck Sweep | HK | J | Keypad 0 | 14 / 5 / 26 | 15 | Low (crouch-guard it), knocks down, the longest sweep |
| Gangway Heave | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 24 | 12 | Throw, techable |
| **Bowline Heel** | **214 + K** (↓ ↙ ← + either kick) | **S, S+A, A + H or J** | **↓, ↓+→, → + Keypad 3 or 0** | 13 / 4 / 22 | 16 | Special: a turning heel kick at head height with a short step; knocks down. Slow to start, so throw it where the opponent will be |

## Greta Voll — power brawler

Square, planted and hard to move: short reach, heavy hands. She waits behind her guard and makes every opening cost. Placeholder art for now.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Short Jab | LP | F | Keypad 1 | 5 / 3 / 10 | 7 | Cancels; 0 to +2 on block |
| Slag Knuckle | HP | G | Keypad 2 | 11 / 3 / 22 | 16 | Heavy, short |
| Toe-Cap Kick | LK | H | Keypad 3 | 7 / 4 / 15 | 10 | Cancels; −1 to +2 on block |
| Pig-Iron Sweep | HK | J | Keypad 0 | 15 / 5 / 26 | 18 | Low (crouch-guard it), knocks down |
| Crucible Toss | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 24 | 14 | Throw, techable |
| **Ladle Drop** | **623 + P** (→ ↓ ↘ + either punch) | **D, S, S+D + F or G** | **←, ↓, ↓+← + Keypad 1 or 2** | 12 / 4 / 24 | 18 | Special: both fists locked and brought down like a tipping ladle, lunging in; knocks down. Huge recovery on a miss |

## Jun Arata — rushdown / feint

Small, quick and always in your face: the fastest jab in the game, a stepping low kick, and a dash that ends safe so the pressure starts again. Placeholder art for now.

| Move | Button / motion | Player 1 (facing right) | Player 2 (facing left) | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Pick Jab | LP | F | Keypad 1 | 3 / 2 / 9 | 4 | The fastest button; cancels; 0 to +1 on block |
| Pocket Hook | HP | G | Keypad 2 | 7 / 3 / 16 | 11 | Heavy |
| Lift Step | LK | H | Keypad 3 | 6 / 3 / 14 | 7 | Low (crouch-guard it); steps in; cancels; −3 to −1 on block |
| Turnstile Kick | HK | J | Keypad 0 | 12 / 4 / 20 | 14 | A spinning kick at chest height; heavy |
| Purse Snatch | LP + LK, close | F + H | Keypad 1 + 3 | 3 / 2 / 20 | 9 | Throw, techable; lands them close |
| **Sleight Feint** | **236 + P** (↓ ↘ → + either punch) | **S, S+D, D + F or G** | **↓, ↓+←, ← + Keypad 1 or 2** | 8 / 3 / 10 | 8 | Special: a low dash into a quick elbow; +2 to +4 on block, so it resets the pressure. Cancels from Pick Jab or Lift Step |

## The Broker — boss (arcade only)

Not on the character select: he waits at the end of the arcade. Listed for reference.

| Move | Button / motion | Startup / active / recovery | Damage | Notes |
| --- | --- | --- | --- | --- |
| Cufflink Jab | LP | 4 / 3 / 10 | 7 | Cancels |
| Signet Cross | HP | 9 / 3 / 20 | 15 | Heavy |
| Oxford Kick | LK | 6 / 4 / 14 | 9 | |
| Collection Sweep | HK | 13 / 5 / 24 | 16 | Low, knocks down |
| Foreclosure | LP + LK, close | 3 / 2 / 22 | 15 | Throw, techable |
| **Final Offer** | **236 + P** | 9 / 4 / 20 | 16 | A long step into an open-palm strike; knocks down |

Frame counts are 60 Hz ticks, from the fighter files (`Assets/Fighters/<Name>/<id>.fighter.json`). In training, F3 shows the boxes and the frame advantage of every hit.
