# Contract: Screens and player interactions

**Scope**: the user-facing contract of the desktop game (FR-047): screens, the elements on
each, mouse and keyboard mappings, and transitions. The design resolution is 1280 × 720,
landscape, letterboxed (`SHOW_ALL`). All text comes from [localization.md](localization.md)
keys, so no literal strings are shown (Principle IV). The only exception is the board's
coordinate labels: both axes use the numerals 1–10, which read the same in every language and
are generated in code.

## Screen flow

```text
            ┌──────────── Settings ◀──────┐
            ▼                             │
Launch ─▶ Main Menu ── New Game ─▶ Difficulty pick ─▶ Placement ─▶ Battle ─▶ Game Over
            ▲                                            │  Back (no confirm)  │    │
            │                                            ▼                     │    ├─ New Game ─▶ Difficulty pick
            └────────────────────────────────────── Main Menu ◀── confirm ─────┘    └─ Main Menu
                                                               (Battle exit needs confirmation, FR-044)
```

> **Redesign v2 "Blood & Black Water" (2026-10-20)**: the visual reference is the claude.ai/design
> project "Battleship pirate redesign", file `Battleship Pirate Mockups v2`. Storm backdrops,
> tilted torn parchment sheets, blood-red primary buttons, tar secondary, gold "safe" and Help;
> Playfair Display SC for headings and buttons. The battle screen adds a gold "Hit! Fire again"
> banner, a Help column (button, use pips, "n of 3 left", Cancel), a red "{computer} unleashes
> Help!" banner with a result bar, and a cross strike that flares all cells at once (FR-048).
> A developer screenshot tour (`BS_SCREENSHOTS=<folder>`, `src/game/tools/ScreenshotTour`)
> renders every screen in all three languages for the layout pass.

## Main Menu
- **Buttons**: New Game, Settings, Quit.
- **Music**: Menu track (if music is on).

## Difficulty pick (overlay on the menu)
- **Buttons**: Easy, Medium, Hard, Back. Choosing a level opens Placement with that difficulty
  (FR-001).

## Settings
| Element | Behaviour |
|---|---|
| Sound effects toggle | applies immediately and persists |
| Music toggle | starts or stops music immediately and persists |
| Language selector (English / Русский / Tiếng Việt) | all visible text updates without restart (FR-040); language names are always shown in their own language |
| Player name field | at most 20 characters entered; the trimmed value is saved on confirm; empty → default name (FR-041) |
| Back | returns to the menu |

## Placement
| Element | Behaviour |
|---|---|
| Player board (10 × 10, columns and rows both labelled 1–10) | drop target |
| Dock | ships not yet placed, grouped by length with remaining counts |
| Left-drag a dock ship or placed ship | shows a preview on hover: green = legal, red = illegal with a reason tooltip (OutOfBounds, Overlaps, Touches) (FR-007, FR-013) |
| Right-click (during a drag or on a placed ship) / Rotate button | toggles orientation |
| Drop on the board | places the ship if legal; otherwise rejects it, shows the reason, and leaves the ship where it was |
| Drop on the dock | removes the ship |
| Auto-place remaining | fills only the missing ships; on failure shows "remaining ships cannot fit" and nothing moves (FR-009, FR-012) |
| Auto-place all | replaces the fleet (FR-010) |
| Start Battle | enabled only when the fleet is complete; clicking while disabled shows "ships still missing" (FR-014) |
| Back / Esc | returns to the menu, no confirmation |

## Battle
| Element | Behaviour |
|---|---|
| Own board (left) | the player's ships with the computer's results |
| Target board (right) | the computer's board as a `TargetView`; unrevealed ships are never drawn (FR-016) |
| Turn banner | "Your turn" / "{computer} is firing…" (FR-024) |
| Name labels | player display name and computer display name (FR-042). On Hard only, a Help counter (`battle.computer_help`, e.g. "Help: 2") sits next to the computer's name and updates after each computer Help (FR-032) |
| Left-click a target cell | normal shot; ignored during the computer's turn; a fired or marked cell is rejected with a brief message (Edge Cases) |
| Help button | shows the remaining uses, e.g. "Help (2)", or a disabled "used up" state at 0 (FR-032). Enabled on the player's turn, including an extra shot after a hit (FR-027). Click → targeting mode: a cross preview follows the mouse over the target board, clipped at edges and showing already-fired cells as skipped. Left-click to use, right-click or Esc to cancel. The whole cross is revealed at once, then any sunk-ship marking (FR-048) |
| Cell visuals | Unknown, Miss (dot), Hit (flame/cross), Sunk (ship outline + darker tint), Marked (faint dot, visually distinct from a miss) |
| Computer pacing | one action every 0.8 s, each rendered before the next (FR-025) |
| Menu button / Esc | opens the confirmation dialog: Leave battle? [Stay] [Leave] (FR-044) |

## Game Over (overlay on the final battle state)
- **Content**: "{name} wins!" (FR-045), victory or defeat sound.
- **Buttons**: New Game (→ Difficulty pick), Main Menu (FR-046). Leaving needs no
  confirmation.

## Keyboard shortcuts (optional conveniences; every action is mouse-reachable)
- `Esc`: back, cancel Help targeting, or open the exit confirmation (context-dependent).
- `R`: rotate during placement.
