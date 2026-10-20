# Feature Specification: Single-Player Battleship Game

**Feature Branch**: `001-battleship-game`

**Created**: 2026-10-17

**Status**: Draft

**Input**: User description: "Complete Battleship game: player vs. computer on 10×10 boards with Easy/Medium/Hard difficulty, a 10-ship fleet, manual/automatic/mixed placement with no-touch rules, extra shot on hit, a once-per-game cross-shaped 'Help' ability for each side, settings (sound, music, language EN/RU/VI, player name), exit confirmation during battle, and winner display. Online multiplayer is out of scope."

## Clarifications

### Session 2026-10-17

- Q: When a ship sinks, should the game automatically mark the empty cells around it as "no ship here"? → A: Yes, for both sides; marked cells count as already fired upon (cannot be shot, skipped by Help) and are not counted as misses.
- Q: Should the computer's displayed name change with the language, or always be "Компьютер"? → A: Translated per language: "Computer" / "Компьютер" / "Máy tính", updating immediately on a language change.
- Q: On Medium and Hard, after the computer hits the same ship twice in a row, should it keep firing only along that line? → A: Yes; after one hit it tries the four orthogonal neighbours, and once two or more hits line up it fires only at the untried cells at either end of that line until the ship sinks.
- Q: How should Hard pick the turn and the target cell for its Help? → A: At game start it picks a random turn from its 1st–20th turn (waiting for the next turn with no wounded ship if busy); *superseded on 2026-10-20: now three distinct turns, one per Help use*; the centre is a random cell whose whole on-board cross is untouched, otherwise a cell whose cross has the most untouched cells.
- Q: What devices does the game run on, and do players use a mouse or touch? → A: Desktop only (Windows and macOS) with a mouse: drag to place, right-click or a button to rotate, landscape window.
- Q: Should the default player name be translated like the computer's name? → A: Yes: "Player" / "Игрок" / "Người chơi"; an empty name falls back to the default for the current language.

### Session 2026-10-20

- Q: How many times per game can Help be used? → A: Three times per game for the player, and three times per game for the computer on Hard (counted separately). This replaces the earlier once-per-game rule. Hard picks three distinct target turns from its 1st–20th turn; the centre rule is unchanged (see FR-035).
- Q: Can Help be used as the extra shot right after a hit, or only as the first action of a turn? → A: In place of any shot the participant is entitled to, including an extra shot after a hit; it then ends the turn.
- Q: Should the battle screen show how many Help uses the computer has left? → A: Yes on Hard: its remaining count (3/2/1/0) is shown next to its name; hidden on Easy and Medium.
- Q: When Help is used, are the cross cells revealed all at once or one after another? → A: All at once, within 0.5 seconds, followed by any sunk-ship marking.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Play a Full Battle Against the Computer (Priority: P1)

The player starts a new game, gets a valid fleet on their board (automatic placement is enough), and takes turns firing at the computer's hidden board. A hit gives the shooter another shot and a miss passes the turn. The game ends as soon as one side's whole fleet is destroyed, and the winner's name is shown.

**Why this priority**: This is the core game loop. Without it there is no game, and with only this story (plus automatic placement and Easy AI) the product is already playable.

**Independent Test**: Start a game with automatic placement on Easy, play to the end, and confirm that turn order, hit/miss/sunk feedback, extra shots and the winner screen all behave correctly.

**Acceptance Scenarios**:

1. **Given** a battle has started and it is the player's turn, **When** the player fires at a cell on the opponent's board that has not been fired upon, **Then** the cell is shown as a hit or a miss.
2. **Given** it is the player's turn, **When** the player's shot is a miss, **Then** the turn passes to the computer.
3. **Given** it is the player's turn, **When** the player's shot is a hit, **Then** the player fires again without the computer taking a turn.
4. **Given** a ship has had all its cells hit, **When** the last cell is hit, **Then** the ship is shown as sunk and every empty on-board cell touching it (sides and corners) is marked as "no ship here".
5. **Given** the opponent has one ship cell left, **When** that cell is hit, **Then** the game ends immediately and the shooter's name is shown as the winner.
6. **Given** it is the player's turn, **When** the player selects a cell that has already been fired upon, **Then** no shot is made and the turn does not change.
7. **Given** it is the computer's turn, **When** the computer fires, **Then** the player sees the result of each computer shot on their own board.

---

### User Story 2 - Arrange the Fleet Manually, Automatically or Both (Priority: P1)

Before the battle the player arranges their 10 ships: one 4-cell ship, two 3-cell ships, three 2-cell ships and four 1-cell ships. They can place each ship by hand, horizontally or vertically. They can also auto-fill only the ships still missing (mixed placement) or have the whole fleet auto-placed. Ships can be moved or removed until the battle starts.

**Why this priority**: Every battle depends on a legal fleet, and the placement rules (no overlapping, no touching) are basic to the game.

**Independent Test**: On the placement screen, place some ships by hand, auto-fill the rest, move and remove ships, try illegal positions, and confirm the battle can start only with a complete legal fleet.

**Acceptance Scenarios**:

1. **Given** the placement screen, **When** the player places a ship horizontally or vertically entirely within the board and not touching any other ship (diagonals included), **Then** the ship is placed.
2. **Given** a ship is already placed, **When** the player tries to place another ship so that it overlaps or touches it (diagonally included), **Then** the placement is rejected and the player sees why.
3. **Given** a ship would extend beyond the board edge, **When** the player tries to place it, **Then** the placement is rejected.
4. **Given** some ships are placed by hand, **When** the player chooses "auto-place remaining", **Then** only the missing ships are added and none of the hand-placed ships move.
5. **Given** any arrangement, including an empty board, **When** the player chooses "auto-place all", **Then** the whole fleet is replaced with a new, complete, legal arrangement.
6. **Given** the hand-placed ships leave no legal room for the remaining ships, **When** the player chooses "auto-place remaining", **Then** the game tells the player that the remaining ships cannot fit, leaves the current arrangement unchanged, and lets the player move or remove ships.
7. **Given** a placed ship, **When** the player moves it, rotates it or removes it, **Then** the board updates and the same placement rules apply.
8. **Given** the fleet is incomplete, **When** the player tries to start the battle, **Then** the battle does not start and the player sees that ships are still missing.
9. **Given** the battle has started, **When** the player tries to move, remove or add a ship, **Then** nothing changes.

---

### User Story 3 - Choose a Difficulty Level (Priority: P2)

Before starting a game the player chooses Easy, Medium or Hard. Each level makes the computer behave differently, so the challenge gets harder from level to level.

**Why this priority**: Difficulty levels give the game variety and replay value. A battle is still playable with Easy alone, so this comes after the core loop.

**Independent Test**: Play several games at each level, or watch the computer's shot sequences, and confirm each level follows its targeting rules.

**Acceptance Scenarios**:

1. **Given** Easy is selected, **When** the computer takes a shot, **Then** it picks a random cell it has not yet fired upon, and earlier hits do not influence the choice.
2. **Given** Medium is selected and the computer has no wounded ship (hit but not yet sunk) to finish, **When** it takes a shot, **Then** it picks a random cell it has not fired upon.
3. **Given** Medium is selected and the computer has hit a ship that is not yet sunk, **When** it takes its next shot, **Then** it targets an unshot cell directly next to the hit cell (above, below, left or right) if that ship has only one hit, or an unshot cell at either end of the line of hits if it has two or more.
4. **Given** Medium is selected and the computer has just sunk the ship it was finishing off, **When** it takes its next shot, **Then** it returns to random targeting, unless another wounded ship is still being tracked.
5. **Given** Hard is selected, **When** the computer plays, **Then** it uses the Medium targeting rules and uses its Help ability up to three times, on the turns defined in FR-035, each centred on a cell chosen as defined in FR-035.
6. **Given** Easy or Medium is selected, **When** the game is played, **Then** the computer never uses the Help ability.

---

### User Story 4 - Use the "Help" Ability (Priority: P2)

Up to three times per game, on their turn, the player can use Help instead of a normal shot. They pick a target cell, and the ability hits that cell plus its neighbours above, below, left and right. This ends the player's turn even if something is hit.

**Why this priority**: Help is the game's distinctive extra mechanic. It adds strategy but isn't needed for a basic game.

**Independent Test**: In a battle, use Help in the middle of the board, at an edge and in a corner, and near cells already fired upon. Check which cells are affected, that the remaining-uses counter goes 3 → 2 → 1 → 0, that the ability can't be used a fourth time, and that the turn passes after each use.

**Acceptance Scenarios**:

1. **Given** it is the player's turn and the player has at least one Help use left, **When** the player uses Help on a cell, **Then** that cell and its four orthogonal neighbours are each resolved as a hit or a miss, and the player's remaining Help uses decrease by one.
2. **Given** the chosen cell is at an edge or in a corner, **When** Help is used, **Then** neighbours outside the board are skipped and only cells on the board are affected.
3. **Given** some cells in the cross have already been fired upon, **When** Help is used, **Then** those cells are skipped and their state does not change.
4. **Given** Help hits one or more ship cells, **When** it resolves, **Then** the turn still passes to the opponent; no extra shot is given.
5. **Given** Help destroys the opponent's last remaining ship cells, **When** it resolves, **Then** the game ends immediately and the user of the ability is declared the winner.
6. **Given** the player has used Help once or twice, **When** their next turn comes, **Then** Help is still available and the interface shows how many uses are left (2 or 1).
7. **Given** the player has already used Help three times in this game, **When** they try to use it again, **Then** it is unavailable and is clearly shown as used up.
8. **Given** the player's normal shot was a hit and they have Help uses left, **When** they use Help for their extra shot, **Then** the cross resolves, one use is consumed and the turn passes to the opponent.
9. **Given** the computer has used some or all of its own Help uses (Hard), **When** the player checks their ability, **Then** the player's remaining uses are unchanged; the two counters are separate, and the computer's counter beside its name shows its own remaining uses.

---

### User Story 5 - Adjust Settings (Priority: P3)

From the menu the player can turn sound effects and music on or off, change the language (English, Russian, Vietnamese) and set their player name, which is shown during and after games.

**Why this priority**: These settings make the game more comfortable and accessible but don't affect the rules.

**Independent Test**: Change each setting, check that it takes effect right away, then restart the game and check that the settings were kept.

**Acceptance Scenarios**:

1. **Given** the settings screen, **When** the player switches the language, **Then** all interface text appears in the chosen language without restarting.
2. **Given** sound effects are turned off, **When** game events happen (shots, hits, sinkings), **Then** no sound effects play; music works the same way, independently.
3. **Given** the player has never set a name, **When** a game is played, **Then** the player is shown with the default name for the current language ("Player" in English).
4. **Given** the player enters a new name, **When** they win a game, **Then** that name is shown as the winner.
5. **Given** the player leaves the name empty or only whitespace, **When** they save, **Then** the name falls back to the default name for the current language.
6. **Given** settings were changed, **When** the game is closed and reopened, **Then** the changed settings are still in effect.

---

### User Story 6 - Leave a Battle Safely (Priority: P3)

If the player tries to go back to the menu during a battle, the game asks them to confirm so they don't lose the game by accident.

**Why this priority**: This protects the player's progress. It's a small safeguard.

**Independent Test**: During a battle, ask to return to the menu, cancel, and check the battle continues unchanged; ask again, confirm, and check the menu appears.

**Acceptance Scenarios**:

1. **Given** a battle is in progress, **When** the player chooses to return to the menu, **Then** a confirmation prompt appears.
2. **Given** the confirmation prompt is shown, **When** the player cancels, **Then** the battle continues exactly where it was.
3. **Given** the confirmation prompt is shown, **When** the player confirms, **Then** the battle is abandoned and the main menu is shown.
4. **Given** the player is on the placement screen or the game-over screen, **When** they return to the menu, **Then** no confirmation is asked.

---

### Edge Cases

- The player clicks the opponent's board while it is the computer's turn: the input is ignored.
- The player selects an already-fired cell as a normal shot: it is rejected and the turn does not change.
- The player picks a Help centre whose five cells (or fewer, at the edge) have all been fired upon already: the selection is rejected, so the ability isn't wasted for no effect.
- The game ends before Hard AI reaches one or more of its chosen Help turns (or before it has a free turn for a deferred use): the remaining uses simply go unused. The same applies to any Help uses the player has not spent.
- Hard AI's Help hits several ships at once: each newly hit but not yet sunk ship is tracked for finishing off on later turns.
- With mixed placement, the hand-placed ships leave too little room: the player is told, nothing moves, and the player can still edit the layout.
- A ship is sunk by a Help shot rather than a normal shot: it is shown as sunk, and its surrounding cells are marked, exactly as after a normal shot.
- Help sinks a ship whose surrounding cells include other cells of the same cross: those cells are resolved first, then the remaining untouched neighbours are marked.
- The player selects a cell marked "no ship here": it is rejected like any already-fired cell and the turn does not change.
- The player changes the language while a battle is in progress, if settings are reachable then: the text updates and the battle state is unaffected.
- The player enters a very long name: input is limited to 20 characters so the name fits in the interface.
- All of one side's ship cells are hit by the final normal shot that would otherwise grant an extra shot: the game ends immediately; no extra shot is offered.

## Requirements *(mandatory)*

### Functional Requirements

**Game setup & modes**

- **FR-001**: The game MUST let the player start a single-player game against the computer and choose a difficulty: Easy, Medium or Hard.
- **FR-002**: The game MUST NOT offer online or local multiplayer in this release.
- **FR-047**: The game MUST run on desktop computers with Windows and macOS, played with a mouse in a landscape window. Every action MUST be possible with the mouse alone. Mobile devices and touch input are out of scope for this release.

**Board & fleet**

- **FR-003**: Each participant MUST have their own 10 × 10 board.
- **FR-004**: Each fleet MUST contain exactly 10 ships: one 4-cell, two 3-cell, three 2-cell and four 1-cell ships.
- **FR-005**: Each ship MUST occupy a straight line of cells, either horizontal or vertical, entirely within the board.
- **FR-006**: Ships MUST NOT overlap and MUST NOT touch one another on any side or corner; at least one empty cell must separate any two ships.

**Placement**

- **FR-007**: The player MUST be able to place each ship by hand by dragging it onto the board with the mouse, and choose its orientation with a right-click or an on-screen rotate button. A preview MUST show where the ship would land and whether that position is legal before it is dropped.
- **FR-008**: The player MUST be able to move, rotate or remove any placed ship before the battle starts.
- **FR-009**: The game MUST provide "auto-place remaining", which adds only the ships not yet placed and never moves or removes ships already placed.
- **FR-010**: The game MUST provide "auto-place all", which replaces the whole fleet with a new, complete, legal arrangement.
- **FR-011**: Every automatic arrangement MUST satisfy FR-005 and FR-006.
- **FR-012**: When "auto-place remaining" cannot fit the missing ships around the current layout, the game MUST tell the player, leave the layout unchanged and let the player keep editing.
- **FR-013**: The game MUST reject illegal manual placements and show the player why.
- **FR-014**: The battle MUST NOT start until the player's fleet is complete and legal.
- **FR-015**: Ship placement and editing MUST be unavailable once the battle starts.
- **FR-016**: The computer's fleet MUST be generated automatically under the same rules and MUST stay hidden from the player, except for cells the player has fired upon.

**Turns & shooting**

- **FR-017**: Participants MUST take turns firing at a single cell on the opponent's board.
- **FR-018**: A participant MUST NOT be able to fire at a cell they have already fired upon.
- **FR-019**: A miss MUST pass the turn to the opponent; a hit from a normal shot MUST give the same participant another shot.
- **FR-020**: The result of every shot (miss, hit, sunk) MUST be shown on the relevant board for both the player's and the computer's shots.
- **FR-021**: A ship MUST be shown as sunk once all its cells have been hit.
- **FR-022**: When a ship is sunk, by a normal shot or by Help, every on-board cell touching it (sides and corners) that has not been fired upon MUST be marked automatically as "no ship here". This applies equally to both participants. Marked cells MUST count as already fired upon for FR-018, FR-028, FR-029 and FR-037, and MUST NOT be counted or shown as misses.
- **FR-023**: The game MUST end immediately when every cell of a participant's fleet has been hit, whatever turn or extra shot was pending.
- **FR-024**: The player MUST be able to tell clearly whose turn it is at all times.
- **FR-025**: The computer's shots MUST happen at a pace the player can follow, with each shot visible before the next one.

**Help ability**

- **FR-026**: Each participant MUST have exactly three Help uses per game, tracked separately for the player and the computer. Unused uses do not carry over to the next game.
- **FR-027**: Using Help MUST replace a normal shot on the user's turn — either the first shot or any extra shot earned by a hit — and MUST affect the chosen cell plus its orthogonal neighbours (up, down, left, right), up to five cells.
- **FR-028**: Cells in the cross that are outside the board, or have already been fired upon, MUST be skipped.
- **FR-029**: The chosen centre MUST be a cell on the board; the game MUST reject a centre where every affected cell has already been fired upon.
- **FR-030**: Each use of Help MUST consume one of the user's remaining uses and end the user's turn, even if it scores hits; therefore Help can be used at most once per turn. When no uses remain, Help MUST be unavailable.
- **FR-031**: If Help destroys the opponent's last remaining ship cell, the game MUST end immediately with its user as the winner.
- **FR-048**: All cells affected by a Help use MUST be revealed together as one action, not one by one; any "no ship here" marking around ships it sinks MUST appear right after.
- **FR-032**: The interface MUST show how many Help uses the player has left (3, 2, 1 or 0) and whether Help is currently available. On Hard, it MUST also show the computer's remaining Help uses (3, 2, 1 or 0) next to the computer's name; on Easy and Medium no computer Help counter is shown.

**Computer opponent**

- **FR-033**: On Easy, the computer MUST pick each shot at random from the cells it has not fired upon.
- **FR-034**: On Medium, the computer MUST fire at random until it hits a ship. While a hit ship is not yet sunk, it MUST target it as follows: with one hit, a random unshot orthogonal neighbour of that cell; with two or more hits in a line, a random unshot cell at either end of that line, never a cell beside the line. After sinking it, the computer MUST return to random targeting, unless another hit-but-unsunk ship is still being tracked.
- **FR-035**: On Hard, the computer MUST use the Medium strategy and MUST use Help three times per game, as follows. A computer turn is counted each time the turn passes to the computer. At game start the computer MUST pick three distinct target turns uniformly at random from its 1st to 20th turn. For each target turn, it MUST use Help as the first action of that turn, or, if it is then finishing a wounded ship, of its next turn with no wounded ship. Since Help ends the turn, at most one Help is used per turn: if a deferred use lands on a turn already taken by another use, it moves to the next free turn with no wounded ship. For each use, the centre MUST be chosen at random among cells whose entire on-board cross is untouched; if there are none, at random among cells whose cross has the most untouched cells.
- **FR-036**: On Easy and Medium, the computer MUST NOT use Help.
- **FR-037**: The computer MUST never fire at a cell it has already fired upon.

**Interface & settings**

- **FR-038**: The game MUST offer settings for sound effects (on/off), music (on/off), language and player name.
- **FR-039**: Languages offered MUST be English, Russian and Vietnamese, and all interface text MUST be available in each.
- **FR-040**: A language change MUST take effect immediately without restarting.
- **FR-041**: The default player name MUST be translated per language: "Player" (English), "Игрок" (Russian), "Người chơi" (Vietnamese). An empty or whitespace-only name MUST fall back to this default, and names MUST be limited to 20 characters.
- **FR-042**: The computer's display name MUST be translated per language: "Computer" (English), "Компьютер" (Russian), "Máy tính" (Vietnamese). It MUST update immediately when the language changes, including on the battle and game-over screens.
- **FR-043**: Settings MUST persist between sessions on the same device.
- **FR-044**: When the player tries to return to the menu during a battle, the game MUST ask for confirmation. Cancelling MUST resume the battle unchanged; confirming MUST abandon it.

**End of game**

- **FR-045**: When the game ends, the winner's name MUST be displayed.
- **FR-046**: From the game-over screen the player MUST be able to start a new game or return to the main menu.

### Key Entities

- **Participant**: One side of a game, either the human player or the computer. Has a display name, a board, a fleet, a count of remaining Help uses (starts at 3), and for the computer a difficulty level.
- **Board**: A 10 × 10 grid of cells belonging to one participant. Each cell is empty or part of a ship, and is either untouched, a miss, a hit, or marked "no ship here" (automatically, next to a sunk ship). Miss, hit and marked cells all count as fired upon.
- **Ship**: A straight run of 1–4 cells with an orientation. Its state is intact, damaged (some cells hit) or sunk (all cells hit).
- **Fleet**: The set of 10 ships belonging to one participant, with a fixed composition (1×4, 2×3, 3×2, 4×1).
- **Game Session**: One battle from placement to game over. Tracks the difficulty, both participants, whose turn it is, the phase (placement / battle / finished) and the winner.
- **Settings**: Player preferences kept across sessions: sound on/off, music on/off, language, player name.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A new player can go from launching the game to firing their first shot in under 60 seconds using automatic placement.
- **SC-002**: 100% of fleets produced by automatic placement, whether full or mixed, satisfy the fleet composition and no-touch rules, checked over at least 10,000 generated arrangements.
- **SC-003**: "Auto-place all" produces a complete fleet in under 1 second, every time.
- **SC-004**: Over at least 100 simulated games per level, Medium wins against Easy at least 60% of the time, and Hard wins against Medium more often than it loses.
- **SC-005**: The computer never fires at the same cell twice and never makes an illegal move, across at least 1,000 simulated games.
- **SC-006**: Every interface text is translated in all three languages, with no untranslated or missing strings found in a full walkthrough of each language.
- **SC-007**: No battle is lost to an accidental exit: 100% of attempts to leave an in-progress battle show a confirmation first.
- **SC-008**: In playtesting, at least 90% of first-time players correctly predict which cells Help will affect after one use.
- **SC-009**: Every shot result (player or computer) appears on screen within 0.5 seconds of the shot; for Help, all affected cells appear together within 0.5 seconds of its use.

## Assumptions

- The game targets a single local player on one Windows or macOS desktop computer. No accounts, internet connection or server are needed.
- The player takes the first shot in every battle.
- Abandoning a battle discards it; saving and resuming a battle in progress is out of scope.
- Sound and music settings are simple on/off toggles; volume sliders are not required.
- The default language is English on first launch.
- Hard AI's three random Help moments are chosen independently each game; whether or not it hits, Help results feed into the Medium-style "finish the wounded ship" tracking.
- The Medium/Hard targeting rule considers only orthogonal neighbours (up/down/left/right) of hit cells, and only the line's ends once two hits are aligned, since ships are straight.
- No statistics, leaderboards, achievements or game history are kept.
- The computer's shots are animated or paced so the player can follow them, roughly 0.5–1.5 seconds per shot.
- Online multiplayer is a possible future addition; this release doesn't need to prepare for it beyond keeping the two participants as separate, equal sides of the rules.
