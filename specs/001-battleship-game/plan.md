# Implementation Plan: Single-Player Battleship Game

**Branch**: `001-battleship-game` | **Date**: 2026-10-17 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `/specs/001-battleship-game/spec.md`

## Summary

This is a complete desktop Battleship game: one player against the computer on 10 × 10
boards, with:
- a 10-ship fleet with manual, automatic or mixed placement under the no-touch rule;
- an extra shot on a hit, and automatic marking around sunk ships;
- a once-per-game cross-shaped Help ability for each side;
- Easy, Medium and Hard AI;
- live EN/RU/VI localization and persistent settings;
- exit confirmation during a battle.

**Technical approach**: C++14 and cocos2d-x 4.0, built with CMake, in three layers:
1. a pure, deterministic **rules core** (`battleship_core`);
2. an engine-free **application layer** of controllers and services (`battleship_app`) that
   talks to the engine only through ports;
3. a thin **cocos2d-x presentation layer** of scenes and adapters.

The core and app layers are built test-first with GoogleTest. They build and test without the
engine, and AI-vs-AI simulations check the success criteria. See [research.md](research.md)
for the decisions.

## Technical Context

**Language/Version**: C++14 for all project targets (research R2)

**Primary Dependencies**:
- cocos2d-x 4.0, as a git submodule, presentation layer only (R1)
- GoogleTest/GoogleMock 1.14.0 via FetchContent, tests only (R3)
- Noto Sans fonts, a bundled asset (R10)

**Storage**: per-user key/value settings through `cocos2d::UserDefault` behind
`ISettingsStore` (R12). No game state is saved.

**Testing**:
- GoogleTest + GoogleMock, run by CTest.
- Label `sim` for the AI-vs-AI simulation suite.
- A manual walkthrough in [quickstart.md](quickstart.md).

**Target Platform**: desktop macOS (x86_64; Rosetta 2 on Apple Silicon) and Windows (Win32),
mouse input, landscape window at a 1280 × 720 design resolution (R1, R15; spec FR-047)

**Project Type**: desktop game application, with an engine-independent rules library

**Performance Goals**:
- 60 fps rendering.
- Every shot result rendered on the same frame (≤ 0.5 s, SC-009).
- `placeAll` under 1 s; target < 5 ms (SC-003).
- Computer acts every 0.8 s (FR-025).

**Constraints**:
- Fully offline: no network at runtime (Principle V).
- Deterministic core for a given seed (Principle I).
- Warnings are errors for project code (R5).
- Three complete translations before merge.

**Scale/Scope**:
- 6 screens (menu, difficulty overlay, settings, placement, battle, game-over overlay).
- About 60 localized strings.
- 3 AI strategies.
- Single local player.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Gate | Requirement | Pre-research | Post-design evidence |
|---|---|---|---|
| I. Rules engine independent | Core has no engine, rendering, audio, i18n or storage dependency; builds and tests alone; deterministic | ✅ Planned | ✅ `battleship_core` is std-only; `BATTLESHIP_BUILD_GAME=OFF` builds core + tests without cocos2d-x (R4); `IRandom` with a portable mapping (R6); [core-api.md](contracts/core-api.md) |
| II. TDD (all non-rendering code) | Tests first for core and app logic; required core coverage list; bulk tests for randomness; bug fixes start with a test | ✅ Planned | ✅ Controllers, settings, localization, audio policy and UTF-8 all live in `battleship_app` behind ports and are unit-testable; bulk/sim tests mapped in [quickstart.md §4](quickstart.md); scenes only render controller output |
| III. Fair computer | AI sees only public info; difficulty only from the strategy | ✅ Planned | ✅ Strategies receive `ITargetView` (Unknown/Miss/Hit/Sunk/Marked) built from the shot record, so the `Board` is unreachable by type; `TargetViewTest`; Hard = Medium + the FR-035 Help rule only (R8) |
| IV. Localization by default | No display strings in code; en/ru/vi complete; live switch; long text fits; Cyrillic/Vietnamese fonts | ✅ Planned | ✅ `TextKeys.h` + `.lang` files + completeness test (R9, [localization.md](contracts/localization.md)); `Localizer::subscribe` for live refresh; Noto Sans (R10); 20-code-point names (R11) |
| V. Simplicity, offline | No network/accounts; nothing beyond the spec; extra dependencies justified | ✅ Planned | ✅ No network code; the only dependencies are the constitution-listed engine and test framework, plus a font asset. No save/resume, statistics or multiplayer |
| VI. SOLID | SRP layering; OCP strategies; LSP contracts; ISP views; DI by constructor; no singletons in core | ✅ Planned | ✅ One strategy class per difficulty plus a factory, with a typed LSP test suite; `IFleetPlacer`; ports `ISettingsStore`, `IResourceReader`, `IAudioPlayer`, `INavigator`; controllers and labels depend on the service interfaces `ISettings`, `ILocalizer`, `IGameAudio`; cocos2d-x singletons only in `src/game/adapters/` ([app-api.md](contracts/app-api.md)) |
| Technology Stack | C++ (one standard ≥ 14), cocos2d-x 4.x, CMake, one test framework, RAII/smart pointers, warnings as errors | ✅ Planned | ✅ C++14 (R2), cocos2d-x 4.0 (R1), separate CMake targets (R4), GoogleTest (R3), `-Werror` / `/WX` (R5); ownership through `std::unique_ptr` and values; `create()`/`Ref` only in scenes |
| Player Experience | ≤ 0.5 s feedback; paced AI; reject bad input without state change; confirm exit; persist settings | ✅ Planned | ✅ R14 tick-driven pacing; outcome `status` with no state change on rejection; `ConfirmExit` mode; `SettingsService` persists on every change |

**Result**: PASS before and after design. No violations, so Complexity Tracking is empty.

## Project Structure

### Documentation (this feature)

```text
specs/001-battleship-game/
├── plan.md              # This file
├── research.md          # Phase 0: decisions R1–R15
├── data-model.md        # Phase 1: entities, rules, state machines
├── quickstart.md        # Phase 1: build/test/validation guide
├── contracts/
│   ├── core-api.md      # battleship_core public API
│   ├── app-api.md       # battleship_app ports, services, controllers
│   ├── ui-screens.md    # screens, mouse mappings, transitions
│   └── localization.md  # .lang format, key catalogue, settings keys
├── checklists/requirements.md
└── tasks.md             # Phase 2 (/speckit-tasks — not created here)
```

### Source Code (repository root)

```text
CMakeLists.txt                     # options: BATTLESHIP_BUILD_GAME (ON), BATTLESHIP_BUILD_TESTS (ON)
cmake/
├── Warnings.cmake                 # battleship_warnings INTERFACE target (R5)
└── GoogleTest.cmake               # FetchContent, pinned 1.14.0 (R3)
src/
├── core/                          # battleship_core — std only, NO cocos2d-x
│   ├── include/bs/core/           # Types.h Coord.h Fleet.h Board.h AutoPlacer.h GameSession.h Ai.h Random.h
│   └── src/                       # Fleet.cpp Board.cpp BacktrackingPlacer.cpp GameSession.cpp
│                                  # TargetView.cpp RandomStrategy.cpp HuntTargetStrategy.cpp
│                                  # HardStrategy.cpp HelpPlanner.cpp Mt19937Random.cpp
├── app/                           # battleship_app — std + core only, NO cocos2d-x
│   ├── include/bs/app/            # Ports.h Services.h SettingsService.h Localizer.h TextKeys.h AudioService.h
│   │                              # Utf8.h StringTable.h GameSetup.h PlacementController.h BattleController.h
│   │                              # SettingsController.h
│   └── src/
└── game/                          # BattleShip executable — cocos2d-x presentation (thin)
    ├── AppDelegate.cpp/.h
    ├── main/                      # mac/ (main.mm, Info.plist) and win32/ (main.cpp)
    ├── scenes/                    # MenuScene SettingsScene PlacementScene BattleScene GameOverLayer ConfirmDialog
    ├── nodes/                     # BoardNode ShipNode DockNode HelpPreviewNode
    └── adapters/                  # UserDefaultSettingsStore FileUtilsResourceReader CocosAudioPlayer SceneNavigator
Resources/
├── i18n/                          # en.lang ru.lang vi.lang
├── fonts/                         # NotoSans-Regular.ttf NotoSans-Bold.ttf (+ OFL.txt)
└── audio/                         # *.mp3 effects and music (boards and ships are drawn with DrawNode)
tests/
├── core/                          # CoordTest Mt19937RandomTest FleetTest BoardTest AutoPlacerTest AutoPlacerBulkTest
│                                  # GameSessionTurnTest HelpTest TargetViewTest FairnessCompileTest
│                                  # StrategyContractTest (typed) RandomStrategyTest HuntTargetStrategyTest HardStrategyTest
├── app/                           # SettingsServiceTest LocalizerTest LocalizationCompletenessTest Utf8Test StringTableTest
│                                  # AudioServiceTest GameSetupTest PlacementControllerTest BattleControllerTest SettingsControllerTest
├── fakes/                         # ScriptedRandom InMemorySettingsStore FakeResourceReader MockAudioPlayer
│                                  # FakeLocalizer MockGameAudio
└── sim/                           # AiSimulationTest (CTest label "sim": SC-004, SC-005)
third_party/
└── cocos2d-x/                     # git submodule @ cocos2d-x-4.0 (only used when BATTLESHIP_BUILD_GAME=ON)
```

**Structure Decision**: a single CMake project with three library/executable layers
(`src/core`, `src/app`, `src/game`) and one `tests/` tree. The dependency direction is strictly
`game → app → core`, and the CMake target links enforce it. `src/core` and `src/app` never
include cocos2d-x, so the core and app tests build and run with `BATTLESHIP_BUILD_GAME=OFF`.

## Risks & Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| cocos2d-x 4.0 is no longer maintained, and its third-party libraries are x86_64-only on macOS and 32-bit on Windows | No native Apple Silicon or Win64 builds; possible breakage with the newest Xcode/MSVC | Build x86_64 (Rosetta) and Win32 (R1). Keep the engine only in `src/game`, so a switch to Axmol (a cocos2d-x fork) would touch only that layer. Such a switch needs a constitution amendment |
| Hard may not beat Medium by enough to pass SC-004 over 100 games | Fails a success criterion | Run the `sim` suite early (right after the strategies exist). If it fails, raise it via `/speckit-clarify` before tuning anything outside the spec |

## Complexity Tracking

No constitution violations, so nothing to justify.
