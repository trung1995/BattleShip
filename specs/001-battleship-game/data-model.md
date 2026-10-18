# Data Model: Single-Player Battleship Game

**Feature**: `001-battleship-game` | **Date**: 2026-10-17 | **Plan**: [plan.md](plan.md)

All entities in sections 1–4 live in `battleship_core` (standard library only, no engine).
Settings and localization (section 5) live in `battleship_app`. Types are described in a
language-neutral way; the exact C++ signatures are in [contracts/core-api.md](contracts/core-api.md).

## 1. Value types

### Coord
| Field | Type | Rule |
|---|---|---|
| row | int | 0–9 |
| col | int | 0–9 |

- `isOnBoard()` is true when both values are in 0–9.
- Equality and ordering are by (row, col).
- Helpers: orthogonal neighbours (up to 4) and all neighbours including diagonals (up to 8),
  both clipped to the board.

### Orientation
`Horizontal | Vertical`

### ShipPlacement
| Field | Type | Rule |
|---|---|---|
| origin | Coord | top-left cell of the ship |
| length | int | 1–4 |
| orientation | Orientation | Horizontal grows along `col`, Vertical along `row` |

- `cells()` returns the `length` cells starting at `origin`. The placement is legal only if all
  of them are on the board (FR-005).

### Side
`Player | Computer`. The two sides are equal in the rules; only the session knows which one is
the human.

## 2. Board, ships and fleet

### FleetSpec (constant)
Lengths `[4, 3, 3, 2, 2, 2, 1, 1, 1, 1]`, giving 10 ships and 20 ship cells (FR-004). Each slot
has a stable `ShipId` from 0 to 9 in this order.

### Ship
| Field | Type | Notes |
|---|---|---|
| id | ShipId (0–9) | slot in FleetSpec |
| placement | ShipPlacement | |
| hits | bitset of `length` | which of its cells are hit |

**State** (derived): `Intact` (no hits), `Damaged` (some hits) or `Sunk` (all hits) (FR-021).

### Fleet
A collection of up to 10 Ships indexed by `ShipId`. Each slot is either empty or occupied.

- `isComplete()` is true when all 10 slots are occupied.
- `remainingCells()` is the number of unhit ship cells. When it reaches 0, that side has lost
  (FR-023).

### Board (owner's truth)
A 10 × 10 grid. Each cell holds:

| Field | Type | Notes |
|---|---|---|
| shipId | ShipId or none | which ship occupies the cell |
| shot | CellShotState | `Untouched`, `Miss`, `Hit` or `Marked` |

A cell is **fired upon** when its `shot` is anything other than `Untouched` (FR-018, FR-022).

### PlacementRules (validation)
A placement `p` of ship `s` on a fleet `F` is legal if and only if:
1. every cell of `p` is on the board (FR-005);
2. no cell of `p` is occupied by another ship in `F` (no overlap);
3. no cell of `p` is a side or corner neighbour of another ship in `F` (no touching, FR-006).

Rejections return one reason: `OutOfBounds`, `Overlaps` or `Touches`. The first failing rule
in that order wins, and the reason is shown to the player (FR-013).

### Shot record (public knowledge): TargetView
This is the read-only projection of a Board that the *opponent* may see, per cell:

| Board shot state | Ship state | Knowledge |
|---|---|---|
| Untouched | — | `Unknown` |
| Miss | — | `Miss` |
| Hit | ship not sunk | `Hit` |
| Hit | ship sunk | `Sunk` |
| Marked | — | `Marked` |

The AI only ever receives a `TargetView` (Principle III). The player's view of the computer's
board uses the same projection, so the computer's ships stay hidden (FR-016).

## 3. Shots and outcomes

### CellChange
| Field | Type |
|---|---|
| side | Side whose board changed |
| coord | Coord |
| newKnowledge | `Miss`, `Hit`, `Sunk` or `Marked` |

### ShotOutcome
| Field | Type | Notes |
|---|---|---|
| status | `Applied`, `RejectedNotYourTurn`, `RejectedAlreadyFired`, `RejectedOffBoard` or `RejectedWrongPhase` | rejected actions leave the state unchanged |
| result | `Miss`, `Hit` or `Sunk` | valid when Applied |
| sunkShip | ShipId or none | |
| changes | list of CellChange | the cell itself, the sunk ship's cells re-labelled `Sunk`, and the cells auto-marked around it |
| turnPassed | bool | true on a miss |
| gameOver | bool | true when this shot hit the last ship cell |

### HelpOutcome
| Field | Type | Notes |
|---|---|---|
| status | `Applied`, `RejectedNotYourTurn`, `RejectedAlreadyUsed`, `RejectedNoEffect`, `RejectedOffBoard` or `RejectedWrongPhase` | `NoEffect` means every on-board cross cell was already fired upon (FR-029) |
| affected | list of Coord | on-board, previously untouched cross cells, in the order resolved |
| sunkShips | list of ShipId | |
| changes | list of CellChange | |
| gameOver | bool | |

The turn always passes after Help unless the game is over (FR-030, FR-031).

### Rules applied to every shot (normal or Help cell)
1. Set the cell to `Miss` or `Hit`.
2. If the hit sinks the ship, mark every `Untouched` side or corner neighbour of that ship as
   `Marked` (FR-022). When Help resolves several cells, all cross cells are resolved first and
   the marking happens afterwards, so that cells in the cross are recorded as their real
   result (Edge Cases).
3. If the opponent's `remainingCells()` reaches 0, the game ends immediately, whatever extra
   shot was pending (FR-023).

## 4. Game session

### Participant
| Field | Type | Notes |
|---|---|---|
| side | Side | |
| board | Board | |
| fleet | Fleet | |
| helpAvailable | bool | starts true and becomes false after use (FR-026) |
| turnsTaken | int | how many times the turn passed to this side (FR-035 counting) |

Display names are not stored in the core. The app layer resolves them: the player's name comes
from Settings, and the computer's name from the localized key `name.computer` (FR-042).

### GameSession
| Field | Type | Notes |
|---|---|---|
| difficulty | `Easy`, `Medium` or `Hard` | fixed at creation (FR-001) |
| player, computer | Participant | |
| phase | `Placement`, `Battle` or `Finished` | |
| currentTurn | Side | starts as `Player` (Assumptions) |
| winner | Side or none | set when phase becomes Finished |

### Phase state machine
```text
            startBattle() [player fleet complete & legal]
Placement ────────────────────────────────────────────────▶ Battle
    │  (computer fleet auto-generated at creation)            │
    │                                                         │ last ship cell hit
    │                                                         ▼ (normal shot or Help)
    └── placement edits allowed only here (FR-015)        Finished (winner set)
```

- `startBattle()` fails with `FleetIncomplete` when the fleet has fewer than 10 ships (FR-014).
- All placement commands are rejected outside `Placement`.
- `fire` and `useHelp` are rejected outside `Battle`.

### Turn state machine (inside Battle)
```text
          normal shot = Miss           ┌────────────┐
 ┌──────── Player's turn ─────────────▶│ Computer's │
 │  Hit/Sunk: same side shoots again   │   turn     │
 │  Help: turn passes                  └─────┬──────┘
 └─────────────────────────────────────────── ┘ Miss or Help
```

- Each switch to a side increments that side's `turnsTaken`.
- A shot that ends the game ends it immediately; no extra shot is given.

## 5. Application-layer data (`battleship_app`)

### Settings
| Field | Type | Default | Validation | Store key |
|---|---|---|---|---|
| soundEnabled | bool | true | — | `settings.sound` |
| musicEnabled | bool | true | — | `settings.music` |
| language | `en`, `ru` or `vi` | `en` | unknown value falls back to `en` | `settings.language` |
| playerName | UTF-8 string | "Player"¹ | trimmed; empty → default; at most 20 code points | `settings.playerName` |

¹ The *stored* value is empty until the player sets a name, and the default name is shown
through the localized key `name.player_default`: "Player" / "Игрок" / "Người chơi" (FR-041).
Because the default is resolved at display time, it follows a language change immediately.

Settings persist on every change (FR-043) and are read once at startup.

### Localization
| Item | Type | Notes |
|---|---|---|
| Language | `en`, `ru` or `vi` | |
| StringTable | map from key to UTF-8 template | loaded from `i18n/<lang>.lang` |
| Template args | `{name}` placeholders | e.g., `battle.winner = {name} wins!` |

Invariant: all three tables contain exactly the key set declared in `TextKeys`, enforced by a
test (SC-006).

### UI session state (BattleController)
| Field | Type | Notes |
|---|---|---|
| mode | `AwaitingPlayer`, `PlayerHelpTargeting`, `ComputerActing`, `ConfirmExit` or `GameOver` | |
| computerDelay | seconds, default 0.8 | time until the next computer action |
| pendingChanges | list of CellChange | handed to the view for rendering |

`ConfirmExit` freezes the battle: `tick` does nothing, and cancelling returns to the previous
mode unchanged (FR-044).
