# Controls and move lists

These are the default bindings. You can change them in **Options → Controls** (they are saved to `prefs.cfg`); the defaults live in [`ProjectSettings/InputManager.json`](../ProjectSettings/InputManager.json). The in-game **Move list** (pause menu) shows each side's moves with the keys that side currently uses.

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

Menus follow your bindings: if you move light punch to another key, that key confirms. On screens one player drives (title, Versus CPU and Training select, options), either player's keys work; on the 2 Players select each side moves its own cursor.

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

## System moves (both fighters)

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

Frame counts are 60 Hz ticks, from the fighter files (`Assets/Fighters/<Name>/<id>.fighter.json`). In training, F3 shows the boxes and the frame advantage of every hit.
