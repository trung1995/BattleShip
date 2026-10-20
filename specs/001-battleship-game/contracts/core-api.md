# Contract: `battleship_core` public API

**Scope**: the public headers of the rules library (namespace `bs::core`). This library uses
only the C++14 standard library and must never include cocos2d-x (Principle I).
Entities are defined in [../data-model.md](../data-model.md). This document fixes signatures,
pre/postconditions and error behaviour. Tests are written against this contract first
(Principle II).

Headers live under `src/core/include/bs/core/`.

## Conventions

- No exceptions for rule violations. Every command returns an outcome with a `status`.
  Rejected commands MUST leave all state unchanged.
- Exceptions are reserved for programming errors (e.g., an out-of-range `ShipId`) and are
  documented as preconditions.
- No global or static mutable state. All randomness comes in through `IRandom&` (Principle VI).
- The same seed and the same calls MUST give the same results on every platform.

## Randomness — `Random.h`

```cpp
class IRandom {
public:
    virtual ~IRandom() = default;
    virtual int nextInt(int lo, int hi) = 0;            // uniform, inclusive; pre: lo <= hi
};
class Mt19937Random final : public IRandom {            // portable mapping, see research R6
public:
    explicit Mt19937Random(std::uint32_t seed);
    int nextInt(int lo, int hi) override;
};
template <class T> void shuffle(std::vector<T>&, IRandom&);   // Fisher–Yates via nextInt
```

## Geometry and fleet — `Coord.h`, `Fleet.h`

```cpp
struct Coord { int row; int col; bool isOnBoard() const; };
enum class Orientation { Horizontal, Vertical };
struct ShipPlacement { Coord origin; int length; Orientation orientation;
                       std::vector<Coord> cells() const; };
using ShipId = int;                                      // 0..9
constexpr int kBoardSize = 10;
constexpr int kHelpUsesPerGame = 3;                      // FR-026
const std::array<int, 10>& fleetSpec();                  // {4,3,3,2,2,2,1,1,1,1}

enum class PlacementError { None, OutOfBounds, Overlaps, Touches, ShipIdOutOfRange };

class Fleet {
public:
    PlacementError canPlace(ShipId, const ShipPlacement&) const;   // ignores the ship's own current slot
    PlacementError place(ShipId, const ShipPlacement&);            // replaces the slot if legal (move/rotate)
    void remove(ShipId);                                           // no-op if empty
    void clear();
    bool isPlaced(ShipId) const;
    bool isComplete() const;
    const ShipPlacement* placementOf(ShipId) const;                // nullptr if empty
    std::vector<ShipId> missingShips() const;
};
```

- **Rule check order**: `OutOfBounds` → `Overlaps` → `Touches` (data-model §2).
- `place` with an illegal placement returns the error and keeps the previous position of that
  ship (FR-013, US2 scenario 7).

## Auto-placement — `AutoPlacer.h`

```cpp
enum class AutoPlaceStatus { Ok, CannotFit };
struct AutoPlaceResult { AutoPlaceStatus status; Fleet fleet; };

class IFleetPlacer {                                     // OCP: alternative placers can be added
public:
    virtual ~IFleetPlacer() = default;
    virtual AutoPlaceResult placeAll(IRandom&) const = 0;
    virtual AutoPlaceResult placeRemaining(const Fleet& partial, IRandom&) const = 0;
};
class BacktrackingPlacer final : public IFleetPlacer { /* research R7 */ };
```

- **Postconditions**:
  - `placeAll` always returns `Ok` with a complete legal fleet.
  - `placeRemaining` returns either `Ok` with a complete legal fleet that keeps every ship of
    `partial` at its exact position (FR-009), or `CannotFit` with `fleet == partial` unchanged
    (FR-012).
- **Performance**: `placeAll` finishes in under 1 s (SC-003). The target is under 5 ms.

## Session — `GameSession.h`

```cpp
enum class Side { Player, Computer };
enum class Difficulty { Easy, Medium, Hard };
enum class Phase { Placement, Battle, Finished };
enum class Knowledge { Unknown, Miss, Hit, Sunk, Marked };

class ITargetView {                                      // ISP: read-only, public info only
public:
    virtual ~ITargetView() = default;
    virtual Knowledge at(Coord) const = 0;
};

struct CellChange { Side board; Coord coord; Knowledge now; };

enum class ActionStatus { Applied, RejectedWrongPhase, RejectedNotYourTurn, RejectedOffBoard,
                          RejectedAlreadyFired, RejectedAlreadyUsed, RejectedNoEffect };
enum class ShotResult { Miss, Hit, Sunk };

struct ShotOutcome { ActionStatus status; ShotResult result; int sunkShip /* -1 none */;
                     std::vector<CellChange> changes; bool turnPassed; bool gameOver; };
struct HelpOutcome { ActionStatus status; std::vector<Coord> affected; std::vector<ShipId> sunkShips;
                     std::vector<CellChange> changes; bool gameOver; };

enum class StartStatus { Started, FleetIncomplete, WrongPhase };

class GameSession {
public:
    GameSession(Difficulty, Fleet computerFleet);        // computer fleet produced by an IFleetPlacer
    // Placement phase (player only)
    Fleet& playerFleet();                                // mutable only while phase == Placement
    StartStatus startBattle();
    // Battle phase
    ShotOutcome fire(Side shooter, Coord target);
    HelpOutcome useHelp(Side user, Coord centre);
    // Queries
    Phase phase() const;  Side currentTurn() const;  Side winner() const;  // winner(): pre phase==Finished
    int  helpUsesLeft(Side) const;                       // 0..kHelpUsesPerGame (FR-026, FR-032)
    bool helpAvailable(Side) const;                      // helpUsesLeft(side) > 0
    int turnsTaken(Side) const;                          // passes-of-turn count (FR-035)
    const ITargetView& viewOf(Side targetBoardOwner) const;   // public projection
    const Fleet& ownFleet(Side) const;                   // for rendering one's own board only
    Difficulty difficulty() const;
};
```

### `fire` behaviour

| Precondition violated | status |
|---|---|
| `phase != Battle` | `RejectedWrongPhase` |
| `shooter != currentTurn` | `RejectedNotYourTurn` |
| `!target.isOnBoard()` | `RejectedOffBoard` |
| target already Miss, Hit or Marked | `RejectedAlreadyFired` |

When applied:
- **Miss**: `turnPassed = true`.
- **Hit**: `turnPassed = false` (FR-019).
- **Sunk**: auto-marks the neighbours (FR-022).
- **Last ship cell**: `gameOver = true`, `phase = Finished`, `winner = shooter`, and no extra
  shot (FR-023).

### `useHelp` behaviour

The wrong-phase, wrong-turn and off-board checks are the same as for `fire`. Help is accepted on
the first action of a turn and on any extra shot after a hit (FR-027). In addition:
- If `helpUsesLeft(user) == 0`: `RejectedAlreadyUsed` (all three uses spent, FR-026).
- If every on-board cross cell is already fired upon: `RejectedNoEffect`, and the help is
  **not** consumed (FR-029, Edge Cases).

When applied:
1. Untouched on-board cross cells are resolved in the order centre, up, down, left, right
   (FR-027, FR-028).
2. Marking around sunk ships happens after all of them are resolved.
3. `helpUsesLeft(user)` decreases by one and the turn passes (FR-030), unless the game is over
   (FR-031). The full `changes` list is returned at once so the view renders it as one batch
   (FR-048).

### `viewOf(side)` guarantee

It never reveals `Untouched` ship cells. Tests assert that an unrevealed ship cell reads
`Unknown` (Principle III).

## Computer opponent — `Ai.h`

```cpp
struct AiAction { enum class Kind { Shot, Help } kind; Coord target; };

class ITargetingStrategy {                               // OCP: one class per difficulty
public:
    virtual ~ITargetingStrategy() = default;
    // Called once per computer action. firstActionOfTurn is true on the first action after the turn passed.
    // turnIndex is turnsTaken(Computer). Must return a legal action for the given view.
    virtual AiAction choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                            int helpUsesLeft, IRandom& rng) = 0;
};
class RandomStrategy;       // Easy    — FR-033
class HuntTargetStrategy;   // Medium  — FR-034 (line-following, research R8)
class HardStrategy;         // Hard    — FR-035; wraps HuntTargetStrategy + HelpPlanner

class HelpPlanner {         // FR-035, research R8
public:
    explicit HelpPlanner(IRandom&);                       // draws kHelpUsesPerGame distinct turns in 1..20
    const std::vector<int>& targetTurns() const;          // sorted ascending
    bool shouldUse(int turnIndex, bool firstActionOfTurn, int helpUsesLeft, bool woundedShip) const;
};

std::unique_ptr<ITargetingStrategy> makeStrategy(Difficulty, IRandom&);   // factory
```

- **Contract (Liskov)**: every strategy MUST return a `Shot` at an `Unknown` cell, or a `Help`
  whose cross contains at least one `Unknown` cell, and only when `helpUsesLeft > 0`.
  Easy and Medium MUST never return `Help` (FR-036). This is checked by one typed test suite
  run against all three strategies, which also checks Hard never returns more than three Helps
  per game.
- **`HelpPlanner::shouldUse`** is true only when `firstActionOfTurn`, `helpUsesLeft > 0`,
  `!woundedShip`, and `count(targetTurns ≤ turnIndex) > kHelpUsesPerGame − helpUsesLeft`
  (at least one scheduled use is due). The three turns are drawn with a partial Fisher–Yates
  over 1..20 through `IRandom::nextInt`, so the schedule is seed-deterministic.
