# Quickstart: Build, test and validate the Battleship game

**Feature**: `001-battleship-game` | **Plan**: [plan.md](plan.md)

This guide proves the feature works end to end. Interfaces are defined in
[contracts/](contracts/), and entities and rules in [data-model.md](data-model.md).

## Prerequisites

| Tool | macOS | Windows |
|---|---|---|
| CMake | ≥ 3.16 | ≥ 3.16 |
| Compiler | Xcode 14+ (AppleClang) | Visual Studio 2019 or 2022 with "Desktop development with C++" |
| Python | 3.x (only for `download-deps.py`) | 3.x |
| Git | with submodules | with submodules |

The core and tests need only CMake and a C++14 compiler. No engine is required.

## 1. Core and application logic: build and run the tests (no engine)

```bash
cmake -S . -B build/tests -DBATTLESHIP_BUILD_GAME=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure -LE sim      # unit tests (fast)
ctest --test-dir build/tests --output-on-failure -L sim       # AI-vs-AI simulations
```

**Expected**: every test passes and the build shows zero warnings (warnings are errors).
This configuration must never fetch or need cocos2d-x (Principle I).

## 2. Engine setup (once)

```bash
git submodule update --init --recursive third_party/cocos2d-x    # pinned to cocos2d-x-4.0
python3 third_party/cocos2d-x/download-deps.py --remove-download yes
```

## 3. Build and run the game

**macOS** (x86_64; it runs under Rosetta 2 on Apple Silicon):
```bash
cmake -S . -B build/mac -GXcode -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build/mac --config Debug
open build/mac/bin/BattleShip/Debug/BattleShip.app
```

**Windows**:
```bat
cmake -S . -B build\win -G "Visual Studio 17 2022" -A Win32
cmake --build build\win --config Debug
build\win\bin\BattleShip\Debug\BattleShip.exe
```

## 4. Automated validation mapped to success criteria

| Check | How | Pass condition |
|---|---|---|
| SC-002 | `AutoPlacerBulkTest` (10,000 seeds, full + mixed) | 100% legal fleets, composition exact |
| SC-003 | `AutoPlacerTimingTest` | each `placeAll` < 1 s (target < 5 ms) |
| SC-004 | `sim` label: 100 seeded games Medium vs Easy, 100 Hard vs Medium | Medium win rate ≥ 60%; Hard wins > losses |
| SC-005 | `sim` label: 1,000 seeded games per difficulty | zero repeated or illegal computer actions |
| SC-006 | `LocalizationCompletenessTest` | identical key sets and placeholders in en/ru/vi |
| Principle III | `TargetViewTest` | unrevealed ship cells always read `Unknown` |

## 5. Manual acceptance walkthrough (the game build)

1. **US1 / SC-001**: launch → New Game → Easy → Auto-place all → Start Battle → click a target
   cell. Time this from launch; it must be under 60 s. Play to the end and check: a miss passes
   the turn, a hit gives another shot, a sunk ship's neighbours get faint marks, and
   "{name} wins!" appears.
2. **US2**: drag ships, try to touch diagonally (red preview + reason), right-click to rotate,
   drop on the dock to remove, Auto-place remaining, then crowd the board until "cannot fit"
   appears and check nothing moved.
3. **US3**: on Medium, after two aligned hits, the computer fires only at the line's ends. On
   Hard, the computer uses Help once within its first ~20 turns (watch for the cross effect).
4. **US4**: use Help in a corner, at an edge, and next to fired cells. Check the clipped cross,
   that the turn passes, and that the button shows "used".
5. **US5**: switch the language during the menu and the battle. All text changes immediately,
   including the computer's name ("Computer" / "Компьютер" / "Máy tính"). Enter a 25-character
   Cyrillic or Vietnamese name; it is capped at 20 with no clipping. Toggle sound and music,
   restart the app, and check the settings are kept.
6. **US6**: press Esc in battle → Stay (the state is unchanged, and the computer does not act
   while the dialog is open) → Esc → Leave → the menu appears. Esc on placement and game over
   leaves without a prompt.
7. **SC-009**: every shot result appears immediately, and the computer acts every ~0.8 s.

Repeat steps 1, 4 and 5 on both macOS and Windows.
