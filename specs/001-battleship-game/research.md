# Research: Single-Player Battleship Game

**Feature**: `001-battleship-game` | **Date**: 2026-10-17 | **Plan**: [plan.md](plan.md)

Each entry resolves an unknown from the plan's Technical Context or records a best-practice
decision for a mandated technology (constitution, Technology Stack).

## R1. cocos2d-x version and supported desktop targets

- **Decision**: cocos2d-x **4.0** (the latest official release), used as a git submodule at
  `third_party/cocos2d-x` pinned to tag `cocos2d-x-4.0` and built from source with CMake.
  Targets: **macOS** (Xcode generator, `x86_64` build; it runs natively on Intel and under
  Rosetta 2 on Apple Silicon) and **Windows** (Visual Studio 2019/2022 generator, `Win32`
  architecture).
- **Rationale**: 4.0 is the only 4.x release, and the constitution requires 4.x. It is
  CMake-based, which matches the constitution's build rule. Its prebuilt third-party libraries
  (`external/`) ship as macOS `x86_64` and Windows 32-bit only, so those architectures are the
  ones known to link.
- **Alternatives considered**:
  - *cocos2d-x 3.17*: not 4.x, and its build system is an older hybrid. Rejected.
  - *Native arm64 / Win64 builds*: they need the third-party libraries rebuilt, which is
    unsupported upstream. Deferred.
  - *Axmol (an actively maintained cocos2d-x fork with arm64/x64 and C++17/20)*: technically
    better, but it is a different engine, so it would need a constitution amendment. Recorded
    as the fallback if the 4.0 toolchain proves unworkable (see Risks in plan.md).

## R2. C++ standard

- **Decision**: **C++14** for every project target (`CMAKE_CXX_STANDARD 14`,
  `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF`).
- **Rationale**: The constitution requires one standard across the project, supported by the
  engine and its toolchains, at C++14 or newer. cocos2d-x 4.0 sources are C++11/14. Under
  C++17 they hit removed library features (e.g., `std::random_shuffle`) with newer libc++/MSVC
  STL, which would mean patching the engine. C++14 compiles cleanly on both toolchains.
- **Consequences**: no `std::optional`/`std::variant`. The core defines a small result type
  (an enum status plus a payload) instead, which fits the core's explicit-outcome style anyway.
- **Alternatives considered**: C++17. Rejected for now because it needs engine patches. It can
  be revisited with a constitution-compliant plan amendment.

## R3. Unit test framework

- **Decision**: **GoogleTest 1.14.0** (the last line supporting C++14), including GoogleMock
  for interaction checks on fakes. It is fetched with CMake `FetchContent` from a pinned
  release archive URL with a SHA256 hash. Tests are registered through `gtest_discover_tests`
  so CTest runs them.
- **Rationale**: It is the most widely used C++ test framework, has first-class CMake/CTest
  integration and death/param/typed tests, and GoogleMock supports constructor-injected fakes
  (Principle VI). Parameterized tests suit the edge/corner Help cases and the per-difficulty
  rules.
- **Alternatives considered**: Catch2 v3 needs C++14 too and is lighter, but its mocking
  story is weaker. doctest has no mocks. Both are viable, but GoogleTest was chosen once for
  the whole project (constitution: one framework).
- **Offline builds**: `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` can point at a local copy, so the
  first configure is the only step that needs network access.

## R4. Layering so the core and its tests build without the engine

- **Decision**: three CMake library targets plus executables:
  1. `battleship_core` holds the rules, placement, game session and AI. Standard library only.
  2. `battleship_app` holds the non-rendering application logic: controllers/presenters,
     settings service, localization, audio policy and UTF-8 utilities. It depends on
     `battleship_core` and the standard library only, and talks to the engine through
     interfaces.
  3. `BattleShip` is the game executable: cocos2d-x scenes, nodes and adapters that implement
     the `battleship_app` interfaces.
  4. `battleship_tests` is the GoogleTest executable for core + app, and `battleship_sim` is the
     AI-vs-AI simulation test (CTest label `sim`).

  The top-level option `BATTLESHIP_BUILD_GAME` (default `ON`) adds the engine and the game
  target. With it set to `OFF`, only 1, 2 and 4 are configured, so no engine checkout is needed.
- **Rationale**: This satisfies Principle I (the core never sees cocos2d-x) and Principle II
  (all non-rendering code is testable without the engine), and keeps the rendering layer thin.
- **Alternatives considered**: a single library with `#ifdef`s. Rejected because it breaks
  Principle I and makes the dependency direction unenforceable.

## R5. Compiler warnings policy

- **Decision**: a `battleship_warnings` INTERFACE target applied to project targets only:
  - Clang/AppleClang/GCC: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Werror`.
  - MSVC: `/W4 /WX /permissive- /utf-8`.

  cocos2d-x include directories are added as `SYSTEM` so engine headers don't trigger
  warnings, and engine targets keep their own flags.
- **Rationale**: This follows the constitution ("treated as errors for project code, not for
  third-party code"). `/utf-8` is required because the source and resource literals contain
  Cyrillic and Vietnamese text.

## R6. Randomness and determinism

- **Decision**: an `IRandom` interface in the core (`nextInt(lo, hi)` inclusive, plus
  `shuffle` of index vectors). The production implementation `Mt19937Random` wraps
  `std::mt19937` with an explicit seed and uses its own uniform-integer mapping, not
  `std::uniform_int_distribution`, whose output differs between standard libraries. Tests use
  scripted or seeded fakes.
- **Mapping algorithm** (normative): for `nextInt(lo, hi)`, let `range = (uint64)(hi − lo) + 1`.
  If `range == 2³²`, return `lo + engine()`. Otherwise let `limit = 2³² − (2³² mod range)`,
  repeat `x = engine()` until `x < limit`, and return `lo + (int)(x mod range)`. `engine` is
  `std::mt19937`, whose raw output the C++ standard fully defines, so this mapping gives
  identical results on every platform.
- **Rationale**: Principle I requires identical results for the same seed on every platform,
  and Principle VI requires randomness behind an abstraction.

## R7. Auto-placement algorithm

- **Decision**: a **randomized complete backtracking search**. Ships are placed largest-first.
  For each ship, all legal positions (both orientations) are enumerated against the current
  occupancy-plus-halo mask and tried in shuffled order, recursing until the fleet is complete.
  - "Auto-place all" starts from an empty board.
  - "Auto-place remaining" starts from the hand-placed ships and fixes them in place.

  Because the search is complete, a "none found" answer is proof that the remaining ships
  cannot fit (FR-012), never a random false negative. A node budget (e.g., 2,000,000 visits)
  guards the pathological case, and hitting the budget is reported as "cannot fit".
- **Rationale**: The 10 × 10 board and 10 ships keep the search tiny. A full fleet is found in
  microseconds on an empty board, well within SC-003 (< 1 s). Bulk tests over 10,000 seeds
  cover SC-002.
- **Alternatives considered**:
  - *Random restarts only*: fast, but can falsely report "cannot fit" for mixed placement.
    Rejected.
  - *Precomputed layouts*: fail the randomness expectation and cannot extend partial layouts.
    Rejected.

## R8. Computer opponent design (fair by construction)

- **Decision**:
  - **Interface**: an `ITargetingStrategy` interface with `Action chooseAction(const
    ITargetView&, IRandom&)`, where `Action` is a normal shot or Help at a coordinate.
    `ITargetView` is a read-only view of what the shooter knows about the opponent's board:
    each cell is `Unknown | Miss | Hit | Sunk | Marked`. It is built from the session's public
    shot record, never from the opponent's `Board`, so the strategy *cannot* see unrevealed
    ships (Principle III, enforced by types and ISP).
  - **Easy (`RandomStrategy`)**: picks uniformly at random among `Unknown` cells.
  - **Medium (`HuntTargetStrategy`)**: derives "wounded ships" as orthogonally-connected
    clusters of `Hit` cells, oldest first. Because ships never touch, each cluster is exactly
    one ship.
    - A cluster with 1 cell: target a random `Unknown` orthogonal neighbour.
    - A cluster with ≥ 2 cells: target a random `Unknown` cell at either end of the line.
    - No clusters: random, as in Easy.
  - **Hard (`HardStrategy`)**: a decorator over `HuntTargetStrategy` plus a `HelpPlanner`.
    - The planner draws the target turn in `[1, 20]` at construction.
    - On the first action of a computer turn with index ≥ target and no wounded cluster, it
      returns Help.
    - The centre is chosen at random among cells whose whole on-board cross is `Unknown`, or
      failing that among cells with the most `Unknown` cross cells (FR-035).
  - **Turn counting**: the session counts how many times the turn has passed to the computer.
- **Rationale**: Each difficulty is its own class behind one interface (Open/Closed), and Hard
  reuses Medium by composition. The behaviour is deterministic with a seeded `IRandom`, so the
  1,000-game SC-005 and 100-game SC-004 simulations are reproducible.

## R9. Localization resources and live switching

- **Decision**:
  - **Files**: one UTF-8 file per language, `Resources/i18n/en.lang`, `ru.lang` and
    `vi.lang`, in a flat `key = value` format with `#` comments and `{name}` placeholders.
  - **Parsing**: the `battleship_app` `StringTable` parses them, and a `Localizer` service
    exposes `text(key, args)` and a change-notification subscription.
  - **File access**: the engine adapter reads the files through `FileUtils` and passes the
    contents in as a string, so the parser stays engine-free and testable.
  - **Live switching**: scenes subscribe and re-apply text when the language changes
    (FR-040).
  - **Completeness**: a unit test asserts that all three files contain the same key set and
    that every key used in code exists. Keys live in one header (`TextKeys.h`), and the test
    walks that list (SC-006).
- **Rationale**: This adds no new dependency (Principle V), is trivially parseable and
  diff-friendly, and keeps a single source of truth for keys.
- **Alternatives considered**:
  - *JSON via the RapidJSON bundled in cocos2d-x*: would make the parser depend on an engine
    header. Rejected.
  - *gettext .po*: would need a new dependency and tooling. Rejected.

## R10. Fonts for Latin, Cyrillic and Vietnamese

- **Decision**: **Noto Sans** (Regular + Bold, SIL Open Font License) as TTF in
  `Resources/fonts/`, rendered with `Label::createWithTTF`. Its glyph coverage includes
  Vietnamese stacked diacritics (e.g., "Máy tính", "Người chơi") and Cyrillic.
- **Rationale**: Principle IV requires correct rendering of all three scripts. One font family
  covers all of them, and the OFL licence permits bundling.
- **Validation**: a manual walkthrough per language (SC-006), plus a layout check at 20
  characters of the widest glyphs ("Ш" / "W" / "Ẩ").

## R11. Player name length and UTF-8

- **Decision**: name length is counted in **Unicode code points** after trimming ASCII and
  Unicode whitespace. The limit is 20. `battleship_app` provides a small `Utf8` utility for
  decoding, counting and truncating, so the rule is testable without the engine.
- **Rationale**: Russian and Vietnamese characters take 2–3 bytes in UTF-8, so a byte limit
  would wrongly cut names short (FR-041).
- **Alternatives considered**: grapheme clusters. Rejected because they would need ICU
  (a heavy dependency). Desktop text input produces precomposed (NFC) Vietnamese, so code
  points and visible characters match in practice.

## R12. Settings persistence

- **Decision**: an `ISettingsStore` interface (get/set bool and string by key) in
  `battleship_app`. The production adapter `UserDefaultSettingsStore` wraps
  `cocos2d::UserDefault`, and tests use `InMemorySettingsStore`. `SettingsService` owns the
  defaults (sound on, music on, language `en`, name "Player") and validation.
- **Rationale**: `UserDefault` is the engine's standard per-user store on macOS and Windows
  (FR-043). The interface keeps the cocos2d-x singleton inside the adapter (Principle VI).

## R13. Audio

- **Decision**: an `IAudioPlayer` interface (`playEffect(id)`, `playMusic(id, loop)`,
  `stopMusic()`). The production adapter uses `cocos2d::experimental::AudioEngine`.
  `AudioService` in `battleship_app` maps game events to sound IDs and applies the
  sound-effects and music toggles independently, and its behaviour is unit-tested with a mock
  player. Assets are OGG on both platforms.
- **Rationale**: the toggle logic is testable without the engine (US5 scenario 2), and
  `AudioEngine` is the 4.0 audio API on desktop.

## R14. Pacing the computer and the 0.5-second feedback target

- **Decision**: the `BattleController` (in `battleship_app`) is a state machine advanced by
  explicit calls: `onPlayerFire`, `onPlayerHelp` and `tick(elapsedSeconds)`. During the
  computer's turn, `tick` accumulates time and performs one computer action every
  **0.8 s** (a configurable constant, inside the spec's 0.5–1.5 s range). The cocos2d-x scene
  calls `tick` from its `update(dt)`.
  - Every action returns a list of cell changes that the view renders on the same frame.
  - Input during the computer's turn is rejected by the controller (Edge Cases).
- **Rationale**: Time sits behind the `tick` argument, so pacing and turn logic are fully
  unit-testable with no real clock (Principles II and VI). Rendering on the same frame meets
  SC-009 (≤ 0.5 s) with a large margin.

## R15. Mouse interaction for placement (FR-007, FR-047)

- **Decision**: `PlacementController` (`battleship_app`) exposes `beginDrag(shipId | dockSlot)`,
  `hover(cell)`, `rotate()`, `drop(cell)`, `cancelDrag()`, `remove(shipId)`, and the
  auto-place commands.
  - `hover` returns a preview: the cells the ship would cover, plus a legal/illegal flag and
    the rejection reason (out of bounds, overlap or touching).
  - **Adapter mapping**: `EventListenerMouse` maps left-button drag to these calls,
    right-click to `rotate`, and an on-screen rotate button also calls `rotate`. Dropping a
    placed ship back on the dock removes it.
- **Rationale**: All placement decisions are testable, and the scene only maps pixels to cells.
- **Window**: a fixed design resolution of **1280 × 720** with `ResolutionPolicy::SHOW_ALL`.
  The window is resizable and letterboxed, and always landscape.

## Resolved unknowns summary

| Unknown | Resolution |
|---|---|
| cocos2d-x version and architectures | R1: 4.0; macOS x86_64, Windows Win32 |
| C++ standard | R2: C++14 |
| Test framework | R3: GoogleTest 1.14.0 + GoogleMock via FetchContent |
| How the core builds without the engine | R4: `BATTLESHIP_BUILD_GAME=OFF` |
| Localization format | R9: per-language `key = value` UTF-8 files |
| Fonts | R10: Noto Sans |
| Name length unit | R11: Unicode code points |
| Persistence | R12: `UserDefault` behind `ISettingsStore` |
| Pacing / timing | R14: `tick(dt)`, 0.8 s per computer action |
