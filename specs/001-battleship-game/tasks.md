---

description: "Task list for the Single-Player Battleship Game (C++14 / cocos2d-x 4.0)"
---

# Tasks: Single-Player Battleship Game

**Input**: Design documents from `/specs/001-battleship-game/`

**Prerequisites**: [plan.md](plan.md), [spec.md](spec.md), [research.md](research.md),
[data-model.md](data-model.md), [contracts/](contracts/), [quickstart.md](quickstart.md)

**Tests**: REQUIRED. Constitution Principle II (NON-NEGOTIABLE) requires test-first
development for all non-rendering code. Every test task comes before the implementation task
it covers. Run it and confirm it **fails for the expected reason** before writing production
code. Then write the minimal code to pass, and refactor while the tests stay green.
Rendering-only code in `src/game/` (scenes, nodes, adapters) is exempt, but it must not make
game decisions.

**Organization**: Tasks are grouped by user story. US4 (Help) comes before US3 (Difficulty).
Both are P2, but the Hard AI uses the core Help rule.

**Revision 2026-10-20**: the spec changed Help from one use to **three uses per side**
(FR-026, FR-027, FR-030, FR-032, FR-035, FR-048). Phases 1–9 describe what was built for the
original single-use rule and are kept as history. **Phase 10** holds the follow-up tasks that
migrate the code; until it is done, phases 5 and 6 do not meet the current spec.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependency on an incomplete task)
- **[Story]**: The user story the task belongs to (US1–US6)

## Path Conventions

These paths come from [plan.md](plan.md#project-structure):

| Path | Contents |
|---|---|
| `src/core/` | `battleship_core`, standard library only. **No cocos2d-x.** Headers in `src/core/include/bs/core/`, sources in `src/core/src/`, namespace `bs::core` |
| `src/app/` | `battleship_app`, standard library + core only. **No cocos2d-x.** Headers in `src/app/include/bs/app/`, sources in `src/app/src/`, namespace `bs::app` |
| `src/game/` | the cocos2d-x presentation layer (`BattleShip` executable) |
| `tests/core/`, `tests/app/`, `tests/fakes/` | the `battleship_tests` executable |
| `tests/sim/` | the `battleship_sim` executable, CTest label `sim` |
| `Resources/` | game assets, including `Resources/i18n/{en,ru,vi}.lang` |

**Standing rules for every task:**
- **Signatures**: as in [contracts/core-api.md](contracts/core-api.md) and
  [contracts/app-api.md](contracts/app-api.md).
- **Language**: C++14 only, with no `std::optional` or `std::variant`.
- **Warnings**: zero warnings (`-Werror` / `/WX`).
- **Memory**: no raw `new`/`delete` outside cocos2d-x `create()`.
- **Core**: no singletons or mutable globals in `src/core`.
- **Text**: every new user-facing string goes into `src/app/include/bs/app/TextKeys.h`
  **and** into all three `.lang` files in the same task (Principle IV).

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project skeleton and a build that compiles and runs an empty test suite, with and
without the engine.

- [X] T001 Create the directory skeleton from plan.md. It needs:
  - `src/core/include/bs/core/`, `src/core/src/`, `src/app/include/bs/app/`, `src/app/src/`
  - `src/game/{scenes,nodes,adapters,main/mac,main/win32}/`
  - `Resources/{i18n,fonts,audio}/`
  - `tests/{core,app,fakes,sim}/`, `cmake/`

  Then add `.gitignore` at the repo root, ignoring `build/`, `*.xcuserstate`, `xcuserdata/`,
  `.vs/` and `CMakeUserPresets.json`.
- [X] T002 Write the top-level `CMakeLists.txt`:
  - `cmake_minimum_required(VERSION 3.16)` and `project(BattleShip LANGUAGES CXX C)`.
  - `CMAKE_CXX_STANDARD 14`, `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF`.
  - Options `BATTLESHIP_BUILD_GAME` (default ON) and `BATTLESHIP_BUILD_TESTS` (default ON).
  - Include `cmake/Warnings.cmake`.
  - `add_subdirectory(src/core)` and `add_subdirectory(src/app)`.
  - `enable_testing()` and `add_subdirectory(tests)` when tests are on.
  - `add_subdirectory(src/game)` only when `BATTLESHIP_BUILD_GAME`.
- [X] T003 [P] Create `cmake/Warnings.cmake`, defining the INTERFACE target
  `battleship_warnings`:
  - Clang/AppleClang/GCC: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Werror`.
  - MSVC: `/W4 /WX /permissive- /utf-8`.
- [X] T004 [P] Create `cmake/GoogleTest.cmake`. It uses `FetchContent_Declare(googletest URL
  https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz URL_HASH SHA256=<hash>)`.
  Compute and record the real hash. Set `gtest_force_shared_crt ON`, `INSTALL_GTEST OFF` and
  `include(GoogleTest)`. Overriding with `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` must work for
  offline builds.
- [X] T005 Create `src/core/CMakeLists.txt` with the STATIC library `battleship_core` (start
  with `src/core/src/Version.cpp`). It has a PUBLIC include dir `src/core/include` and links
  PRIVATE `battleship_warnings`. It must have no engine include path.
- [X] T006 Create `src/app/CMakeLists.txt` with the STATIC library `battleship_app` (start
  with `src/app/src/Version.cpp`). It links PUBLIC `battleship_core` and PRIVATE
  `battleship_warnings`, and must have no engine include path.
- [X] T007 Create `tests/CMakeLists.txt`:
  - Include `cmake/GoogleTest.cmake`.
  - Executable `battleship_tests` built from globbed `tests/core/*.cpp` and `tests/app/*.cpp`,
    with include dir `tests/fakes`. It links `battleship_app`, `GTest::gtest_main`,
    `GTest::gmock` and `battleship_warnings`.
  - Compile definition `BATTLESHIP_RESOURCES_DIR="${CMAKE_SOURCE_DIR}/Resources"`.
  - `gtest_discover_tests(battleship_tests)`.
  - Executable `battleship_sim` built from `tests/sim/*.cpp`, discovered with
    `PROPERTIES LABELS sim`.
  - A placeholder `tests/core/SmokeTest.cpp` that asserts true.

  Verify `cmake -S . -B build/tests -DBATTLESHIP_BUILD_GAME=OFF && cmake --build build/tests
  && ctest --test-dir build/tests` passes with no cocos2d-x present.
- [X] T008 [P] Create `cmake/CheckNoEngineIncludes.cmake`. This `cmake -P` script scans every
  file under `src/core` and `src/app` for `cocos` (case-insensitive) in `#include` lines and
  fails if it finds any. Register it in `tests/CMakeLists.txt` as CTest `architecture.no_engine_in_core_or_app`.
  This enforces Principle I.
- [X] T009 Add the cocos2d-x submodule at `third_party/cocos2d-x`, pinned to tag `cocos2d-x-4.0`
  (`.gitmodules`). Run `python3 third_party/cocos2d-x/download-deps.py --remove-download yes`
  and confirm `third_party/cocos2d-x/external/` is populated.
- [X] T010 Create `src/game/CMakeLists.txt`, modelled on the cocos2d-x 4.0 `cpp-template`
  CMake:
  - Set `COCOS2DX_ROOT_PATH` to `third_party/cocos2d-x`, add its `cmake/Modules` to
    `CMAKE_MODULE_PATH`, `include(CocosBuildSet)`, and
    `add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)`.
  - Executable `BattleShip` from globbed `src/game/**/*.cpp`, plus `main/mac/main.mm` on APPLE
    and `main/win32/main.cpp` on WIN32.
  - Link `cocos2d` and `battleship_app`; add the engine include dirs as `SYSTEM`; apply
    `battleship_warnings` to project sources only.
  - Use `setup_cocos_app_config` and copy `Resources/` into the bundle or output directory.
  - On APPLE, default `CMAKE_OSX_ARCHITECTURES` to `x86_64`. On WIN32, `message(FATAL_ERROR)`
    unless `CMAKE_GENERATOR_PLATFORM` is `Win32` (research R1).
  - A placeholder `src/game/main/...` plus an empty `AppDelegate` must build an app that opens
    a blank 1280 × 720 window on macOS.
- [X] T011 [P] Add `Resources/fonts/NotoSans-Regular.ttf`, `Resources/fonts/NotoSans-Bold.ttf`
  and `Resources/fonts/OFL.txt` (Noto Sans, SIL Open Font License, with Latin + Cyrillic +
  Vietnamese coverage; research R10).
- [X] T012 [P] Add `.clang-format` at the repo root (based on LLVM, `ColumnLimit: 100`,
  `IndentWidth: 4`) and `.editorconfig` (UTF-8, LF, final newline).

**Checkpoint**: `BATTLESHIP_BUILD_GAME=OFF` builds and the tests pass. The game target opens
an empty window.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core value types, fleet rules, randomness, the app-layer ports and services
(localization, settings, audio), and the game shell. Every story needs these.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

### Interfaces and test doubles (no logic, so no test-first needed)

- [X] T013 [P] Create `src/core/include/bs/core/Random.h` with
  `class IRandom { virtual ~IRandom(); virtual int nextInt(int lo, int hi) = 0; }`, where the
  range is uniform and inclusive and the precondition is `lo <= hi`. Declare `class
  Mt19937Random final : public IRandom` and the template `shuffle(std::vector<T>&, IRandom&)`
  (Fisher–Yates via `nextInt`) per core-api.md.
- [X] T014 [P] Create `src/app/include/bs/app/Ports.h` with `ISettingsStore`,
  `IResourceReader`, `IAudioPlayer`, `enum class SoundId { Shot, Miss, Hit, Sunk, Help,
  Victory, Defeat, UiClick }`, `enum class MusicId { Menu, Battle }` and `INavigator`, copied
  verbatim from contracts/app-api.md.
- [X] T015 [P] Create the shared types and the service interfaces:
  - `src/core/include/bs/core/Types.h`: `enum class Side { Player, Computer }` and
    `enum class ShotResult { Miss, Hit, Sunk }`. The core and app layers both use these, and
    `Board.h` (US1) includes this header.
  - `src/app/include/bs/app/Services.h`: `Language`, `Screen`, `NameResult`, `SubscriptionId`,
    the opaque declaration `enum class TextKey : int;`, and the interfaces `ISettings`,
    `ILocalizer` and `IGameAudio`, copied verbatim from contracts/app-api.md (Principle VI).
- [X] T016 [P] Create `tests/fakes/ScriptedRandom.h`. It implements `IRandom`, returns queued
  values in order, falls back to `lo` when the queue is empty, and uses `ADD_FAILURE` if a
  queued value is outside `[lo, hi]`.
- [X] T017 [P] Create the test doubles for the app ports:
  - `tests/fakes/InMemorySettingsStore.h`: a map-backed `ISettingsStore` that counts
    `flush()` calls.
  - `tests/fakes/FakeResourceReader.h`: maps a path to contents, and `readText` returns false
    for unknown paths.
  - `tests/fakes/MockAudioPlayer.h`: a GoogleMock `IAudioPlayer`.
  - `tests/fakes/FakeLocalizer.h`: an `ILocalizer` whose `text` returns the key name (via
    `keyName`). It records `setLanguage` calls and notifies subscribers synchronously.
  - `tests/fakes/MockGameAudio.h`: a GoogleMock `IGameAudio`.

### Tests first (write, run, see them FAIL)

- [X] T018 [P] Write `tests/core/CoordTest.cpp`:
  - `Coord::isOnBoard` is true only for row and col in 0–9, with `kBoardSize == 10`.
  - Orthogonal neighbours: a corner gives 2, an edge 3, the centre 4.
  - All neighbours including diagonals: a corner gives 3, an edge 5, the centre 8.
  - `ShipPlacement::cells()`: Horizontal grows along `col`, Vertical along `row`, and the
    origin is the top-left cell.
- [X] T019 [P] Write `tests/core/Mt19937RandomTest.cpp`:
  - The same seed gives the same 1,000-value sequence.
  - `nextInt(lo, hi)` covers both `lo` and `hi` within 10,000 draws and never leaves the range.
  - `nextInt(5, 5) == 5`.
  - `shuffle` returns a permutation.
  - The mapping must not use `std::uniform_int_distribution`. Assert a golden first-10
    sequence for seed 42 with `nextInt(0, 99)`, computed from the research R6 mapping algorithm
    applied to the first raw `std::mt19937(42)` outputs.
  - Assert the standard check value: the 10,000th raw output of a default-seeded
    `std::mt19937` is `4123659995`.
- [X] T020 [P] Write `tests/core/FleetTest.cpp`:
  - `fleetSpec()` is `{4,3,3,2,2,2,1,1,1,1}`, with ShipId 0–9 in that order.
  - `place`, `remove`, `clear`, `isPlaced`, `isComplete` and `missingShips` work.
  - `canPlace` returns `OutOfBounds` when a cell is off the board.
  - It returns `Overlaps` on a shared cell, and `Touches` for a side neighbour **and** for a
    diagonal-only neighbour.
  - The check order is OutOfBounds → Overlaps → Touches (a placement that is both off-board
    and overlapping reports `OutOfBounds`).
  - Re-placing a ship (move or rotate) ignores that ship's own current cells.
  - An illegal `place` returns the error and keeps the ship's previous placement.
  - A ShipId outside 0–9 gives `ShipIdOutOfRange`.
  - Ships touching the board edge are legal.
- [X] T021 [P] Write `tests/app/Utf8Test.cpp`:
  - `codePointCount("Người chơi") == 10`, `codePointCount("Компьютер") == 9` and
    `codePointCount("") == 0`.
  - `truncateCodePoints(s, 20)` never splits a multibyte sequence, and leaves a string of
    20 or fewer code points unchanged.
  - `trim` removes ASCII whitespace and Unicode spaces (U+00A0, U+2003, U+3000) at both ends,
    keeps inner spaces, and turns a whitespace-only string into `""`.
- [X] T022 [P] Write `tests/app/StringTableTest.cpp`, covering the grammar in
  contracts/localization.md:
  - `key = value` with the value trimmed; blank lines and `#` comments are ignored.
  - Keys match `[a-z0-9_]+(\.[a-z0-9_]+)*`, and `\n` in a value becomes a newline.
  - CRLF is tolerated, and a leading UTF-8 BOM is stripped.
  - A duplicate key gives a load error naming the key and line.
  - A line without `=` gives a load error.
  - `placeholders(key)` returns the set of `{ident}` names.
- [X] T023 [P] Write `tests/app/LocalizerTest.cpp`, using `FakeResourceReader`:
  - Construction loads `i18n/en.lang`, and `text(key)` returns its value.
  - A missing key returns the key's name, not a crash.
  - `format(key, {{"name","Ann"}})` substitutes `{name}`.
  - `setLanguage(Ru)` loads `i18n/ru.lang` and calls every subscriber exactly once **before it
    returns**. After `unsubscribe`, that subscriber is not called.
  - `displayName(Player, settings)` returns `name.player_default` when `storedPlayerName()` is
    `""`, and otherwise the stored name.
  - `displayName(Computer, …)` returns `name.computer` in the current language.
- [X] T024 [P] Write `tests/app/SettingsServiceTest.cpp`, using `InMemorySettingsStore`:
  - **Defaults**: `soundEnabled == true`, `musicEnabled == true`, `language == En` and
    `storedPlayerName == ""`.
  - Each setter writes immediately to the keys `settings.sound`, `settings.music`,
    `settings.language` (`"en"|"ru"|"vi"`) and `settings.playerName`, and calls `flush()`.
  - A stored language `"fr"` or `""` reads as `En`.
  - `setPlayerName`:
    - trims the value;
    - turns an empty or whitespace-only name into `""`, reporting `reset`;
    - truncates more than 20 code points to exactly 20, reporting `truncated`;
    - keeps a 20-code-point Vietnamese name unchanged.
  - A new service on the same store reads back the persisted values.
- [X] T025 [P] Write `tests/app/AudioServiceTest.cpp`, using `MockAudioPlayer` and a real
  `SettingsService`:
  - `onShot(Miss|Hit|Sunk)` plays `Miss|Hit|Sunk`. `onHelp` plays `Help`.
    `onGameOver(true|false)` plays `Victory|Defeat`.
  - No effect plays when sound is off.
  - `enterScreen(Menu|Battle)` plays the matching music looped, unless music is off.
  - With music off, effects still play; with sound off, music still plays.
  - `onSettingsChanged()` stops music at once when it's turned off, and restarts the current
    screen's music when it's turned on.
- [X] T026 [P] Write `tests/app/LocalizationCompletenessTest.cpp`:
  - It reads `BATTLESHIP_RESOURCES_DIR "/i18n/{en,ru,vi}.lang"` from disk.
  - All three key sets equal the key list in `TextKeys.h` (`bs::app::allTextKeys()`), with no
    extras and none missing.
  - Placeholder sets are equal for every key across the languages.
  - `lang.en`, `lang.ru` and `lang.vi` have the same values in all files.
  - No value is empty.

### Implementation (make the tests above pass)

- [X] T027 [P] Create `src/core/include/bs/core/Coord.h` and `src/core/src/Coord.cpp` with
  `kBoardSize = 10`, `Coord{row,col}` (with `isOnBoard`, `==`, `<`), `orthogonalNeighbours`,
  `allNeighbours`, `Orientation`, and `ShipPlacement{origin,length,orientation}::cells()`.
  This makes T018 pass.
- [X] T028 [P] Create `src/core/src/Mt19937Random.cpp`. `Mt19937Random(std::uint32_t seed)`
  wraps `std::mt19937`, and `nextInt` uses rejection sampling on raw 32-bit output so results
  are identical across standard libraries. Add it to `src/core/CMakeLists.txt`. This makes
  T019 pass.
- [X] T029 Create `src/core/include/bs/core/Fleet.h` and `src/core/src/Fleet.cpp` with
  `ShipId`, `fleetSpec()`, `enum class PlacementError { None, OutOfBounds, Overlaps, Touches,
  ShipIdOutOfRange }` and `class Fleet`, as in core-api.md. The rules are the data-model §2
  PlacementRules, checked in the order OutOfBounds → Overlaps → Touches, and touching includes
  corners. This depends on T027 and makes T020 pass.
- [X] T030 [P] Create `src/app/include/bs/app/Utf8.h` and `src/app/src/Utf8.cpp` with
  `codePointCount`, `truncateCodePoints` and `trim` (Unicode whitespace). This makes T021
  pass.
- [X] T031 [P] Create `src/app/include/bs/app/StringTable.h` and
  `src/app/src/StringTable.cpp`. `StringTable::parse(const std::string&)` returns a result
  with a status, an error message and line number, and the map. It also provides `find(key)`
  and `placeholders(key)`. This makes T022 pass.
- [X] T032 Create `src/app/include/bs/app/TextKeys.h`, `src/app/src/TextKeys.cpp` and the
  initial `Resources/i18n/en.lang`, `ru.lang` and `vi.lang`.
  - **Code**: `enum class TextKey : int { … }` (the base type must match the opaque
    declaration in `Services.h`, T015), plus `const char* keyName(TextKey)` and
    `std::vector<TextKey> allTextKeys()`.
  - **Initial keys and values**:

    | Key | en | ru | vi |
    |---|---|---|---|
    | `name.computer` | Computer | Компьютер | Máy tính |
    | `name.player_default` | Player | Игрок | Người chơi |
    | `lang.en` / `lang.ru` / `lang.vi` | English / Русский / Tiếng Việt | same | same |
    | `menu.new_game` | New Game | Новая игра | Trò chơi mới |
    | `menu.settings` | Settings | Настройки | Cài đặt |
    | `menu.quit` | Quit | Выход | Thoát |
    | `common.back` | Back | Назад | Quay lại |
    | `app.title` | Battleship | Морской бой | Hải chiến |

  This makes T026 pass.
- [X] T033 Create `src/app/include/bs/app/Localizer.h` and `src/app/src/Localizer.cpp`, with
  implementing `ILocalizer` (`Language` comes from `Services.h`), with these members:
  - the constructor `(IResourceReader&, Language)`;
  - `setLanguage`, `language()`, `text(TextKey)` and `format(TextKey, args)`;
  - `subscribe` / `unsubscribe`;
  - `displayName(bs::core::Side, const ISettings&)`, with `Side` from `bs/core/Types.h`.

  This depends on T031, T032 and T034, and makes T023 pass.
- [X] T034 Create `src/app/include/bs/app/SettingsService.h` and
  `src/app/src/SettingsService.cpp`, implementing `ISettings` (`NameResult` comes from
  `Services.h`).
  - **Defaults**: "sound on, music on, language `en`, name `""` (displayed as
    `name.player_default`)".
  - **Validation**: the name is "trimmed; empty → reset; at most 20 code points".
  - **Language**: an "unknown value falls back to `en`".

  This depends on T030 and makes T024 pass.
- [X] T035 Create `src/app/include/bs/app/AudioService.h` and `src/app/src/AudioService.cpp`
  implementing `IGameAudio`, with the constructor `(IAudioPlayer&, const ISettings&)`
  (`Screen` comes from `Services.h`). The Menu and Settings screens
  play `MusicId::Menu`, and Placement and Battle play `MusicId::Battle`. The methods are those
  in app-api.md. This depends on T034 and makes T025 pass.

### Game shell (rendering layer, so TDD is not required; keep it thin)

- [X] T036 [P] Create the engine adapters:
  - `src/game/adapters/UserDefaultSettingsStore.{h,cpp}`: `ISettingsStore` over
    `cocos2d::UserDefault`.
  - `src/game/adapters/FileUtilsResourceReader.{h,cpp}`: `IResourceReader` over
    `FileUtils::getStringFromFile`, which returns false on an empty or missing file.
- [X] T037 [P] Create `src/game/adapters/CocosAudioPlayer.{h,cpp}`: `IAudioPlayer` over
  `cocos2d::experimental::AudioEngine`, mapping `SoundId` and `MusicId` to
  `audio/<name>.mp3` and keeping the current music audio ID. Add CC0 placeholder assets
  `Resources/audio/{shot,miss,hit,sunk,help,victory,defeat,click,menu,battle}.mp3` (MP3 is the
  only format both engine audio backends decode; research R13), and record their sources and
  licences in `Resources/audio/LICENSES.md`.
- [X] T038 Create `src/game/GameServices.{h,cpp}`, the composition root. It owns these with
  `std::unique_ptr`:
  - the adapters;
  - `Mt19937Random`, seeded from `std::random_device`;
  - `SettingsService`, `Localizer` (initial language from settings) and `AudioService`;
  - the `SceneNavigator`.

  Scenes receive `GameServices&` and use only its interface accessors (`ISettings&`,
  `ILocalizer&`, `IGameAudio&`). No scene calls a cocos2d-x singleton for things a port
  covers.
- [X] T039 Create `src/game/adapters/SceneNavigator.{h,cpp}`. It implements `INavigator` with
  `Director::replaceScene` and `TransitionFade(0.2)`. `showSettings`, `showPlacement` and
  `showBattle` log "not yet available" and stay on the menu until the stories add them.
- [X] T040 Create `src/game/AppDelegate.{h,cpp}`, `src/game/main/mac/main.mm`, its
  `Info.plist`, and `src/game/main/win32/main.cpp`.
  - `GLViewImpl::createWithRect` at 1280 × 720, resizable.
  - `setDesignResolutionSize(1280, 720, ResolutionPolicy::SHOW_ALL)`.
  - The window title comes from the `app.title` key. Subscribe to `ILocalizer` and, when the
    language changes, set the new title with
    `glfwSetWindowTitle(static_cast<GLViewImpl*>(glview)->getWindow(), …)` (FR-040).
  - Build `GameServices`, then show `MenuScene`.
  - On `applicationDidEnterBackground`, pause audio.
- [X] T041 [P] Create `src/game/nodes/LocalizedLabel.{h,cpp}`. It is a `cocos2d::Node`
  wrapping `Label::createWithTTF("fonts/NotoSans-Regular.ttf", …)`, or `-Bold` when asked. It
  is bound to a `TextKey` plus an optional args provider `std::function<std::map<…>()>`.
  - It takes an `ILocalizer&`, subscribes in `onEnter`, unsubscribes in `onExit`, and re-applies its
    text when the language changes (FR-040).
  - It supports a maximum width with `Label::Overflow::SHRINK`, so the longest translation
    never clips.
- [X] T042 Create `src/game/scenes/MenuScene.{h,cpp}`:
  - The title is `app.title`. The buttons are `menu.new_game`, `menu.settings` and
    `menu.quit`, each a `LocalizedLabel` on a `ui::Button`.
  - Quit calls `Director::end()`.
  - New Game and Settings call `INavigator` (they are wired up in later stories).
  - On enter, it calls `AudioService::enterScreen(Menu)`.
  - A button press plays `onClick`.

**Checkpoint**: `ctest -LE sim` passes, including the architecture check. The game launches to
a localized main menu with music.

---

## Phase 3: User Story 1 - Play a Full Battle Against the Computer (Priority: P1) 🎯 MVP

**Goal**: New Game → auto-placed fleet → battle against the Easy computer → hit/miss/sunk
feedback, extra shot on a hit, marking around sunk ships → winner screen.

**Independent Test**: Start a game with automatic placement on Easy and play to the end.
Check the turn order, the hit/miss/sunk feedback, the extra shots, the automatic marks around
sunk ships, and the winner screen (spec US1, quickstart §5 step 1).

### Tests for User Story 1 (write first, confirm they FAIL)

- [X] T043 [P] [US1] Write `tests/core/BoardTest.cpp`, for a `Board` built from a complete
  `Fleet`:
  - `shoot(c)` on an empty cell gives `Miss`. On a ship cell it gives `Hit`, and `Sunk` when
    it hits the last cell of that ship.
  - After a sink, every `Untouched` side **and corner** neighbour of the ship becomes
    `Marked`. Cells already Miss or Hit stay as they are, and marking is clipped at the board
    edges.
  - Marked cells are not counted as misses.
  - `isFiredUpon` is true for Miss, Hit and Marked.
  - `remainingCells()` goes from 20 to 0.
  - `shipState(id)` reports Intact, Damaged or Sunk.
- [X] T044 [P] [US1] Write `tests/core/TargetViewTest.cpp`. The projection follows data-model
  §2:
  - Untouched → `Unknown`, **including unrevealed ship cells** (Principle III).
  - Miss → `Miss`; Hit on an unsunk ship → `Hit`; Hit on a sunk ship → `Sunk`;
    Marked → `Marked`.
  - `GameSession::viewOf(Computer)` never returns anything other than `Unknown` for any
    untouched computer ship cell.
- [X] T045 [P] [US1] Write `tests/core/GameSessionTurnTest.cpp`, for a session with a known
  computer fleet:
  - It starts in phase `Placement` with `currentTurn == Player`.
  - `startBattle()` returns `FleetIncomplete` with 9 ships and `Started` with 10. A second
    call returns `WrongPhase`.
  - Each `fire` rejection leaves the session unchanged (compare the views before and after):
    `RejectedWrongPhase` before battle, `RejectedNotYourTurn`, `RejectedOffBoard`, and
    `RejectedAlreadyFired` on Miss, Hit **and Marked** cells.
  - A Miss gives `turnPassed == true` and switches the side. A Hit or Sunk keeps the turn.
  - `turnsTaken(side)` increments each time the turn passes to that side (FR-035 counting).
  - The shot on the last ship cell sets `gameOver`, phase `Finished` and `winner == shooter`,
    with no further shot accepted (FR-023).
  - `changes` contains the shot cell, the sunk ship's cells re-labelled `Sunk`, and every
    auto-`Marked` cell.
- [X] T046 [P] [US1] Write `tests/core/AutoPlacerTest.cpp`:
  - `BacktrackingPlacer::placeAll(rng)` returns `Ok` with a complete fleet that passes the
    PlacementRules check for every ship.
  - The same seed gives the same fleet, and different seeds give different fleets in at least
    95 of 100 cases.
  - A timing test: 100 calls each finish under 1 s (SC-003), with the median under 5 ms.
- [X] T047 [P] [US1] Write `tests/core/AutoPlacerBulkTest.cpp` for `placeAll` over 10,000
  seeds: 100% complete, exact `fleetSpec` composition, no overlap or touch (SC-002, full
  placement part).
- [X] T048 [P] [US1] Write `tests/core/RandomStrategyTest.cpp` and
  `tests/core/StrategyContractTest.cpp`.
  - **StrategyContractTest**: a typed test that is instantiated only with `RandomStrategy`
    for now. It checks that the strategy always returns `Kind::Shot` at an `Unknown` cell,
    never `Help`, and never repeats a cell when 100 shots are applied to a fresh view.
  - **RandomStrategyTest**: with `ScriptedRandom`, the chosen cell is the n-th `Unknown` cell
    in row-major order, and earlier Hit cells do not influence the choice (FR-033).
- [X] T049 [P] [US1] Write `tests/app/GameSetupTest.cpp`, using `BacktrackingPlacer` and a
  seeded `Mt19937Random`:
  - `newSession(d)` returns a session in phase `Placement` with difficulty `d`.
  - The computer fleet is complete and legal, and the player fleet is empty.
  - The same seed gives the same computer fleet.
  - A `PlacementController` built on `session->playerFleet()` changes the session's own
    player fleet: after `autoPlaceAll()`, `session->startBattle()` returns `Started`.
- [X] T050 [P] [US1] Write `tests/app/PlacementControllerTest.cpp` (US1 part):
  - `autoPlaceAll()` fills all 10 ships.
  - `canStartBattle()` is false with fewer than 10 ships and true with 10.
  - `dockShips()` is empty after `autoPlaceAll()`.
- [X] T051 [P] [US1] Write `tests/app/BattleControllerTest.cpp` (US1 part), using a seeded
  session, `RandomStrategy`, `ScriptedRandom` and `MockGameAudio`:
  - It starts in mode `AwaitingPlayer`.
  - `onPlayerFire` on a miss returns `Applied` with the changes and moves to
    `ComputerActing`. On a hit it stays in `AwaitingPlayer`.
  - `tick(0.79)` performs no computer action. `tick(0.01)` then performs exactly one, and a
    single `tick(5.0)` still performs only one.
  - A computer hit stays in `ComputerActing` for the next 0.8 s, and a computer miss returns
    to `AwaitingPlayer`.
  - `onPlayerFire` during `ComputerActing` returns `RejectedNotYourTurn` with no changes.
  - An already-fired cell returns `RejectedAlreadyFired` with TextKey
    `battle.reject.already_fired`.
  - The last hit moves to `GameOver`.
  - `turnIndicator()` reflects the current side.
  - `MockGameAudio` receives `onShot` for each result and `onGameOver(playerWon)`.
- [X] T052 [P] [US1] Write `tests/sim/AiSimulationTest.cpp` (Easy part, label `sim`):
  - A headless loop runs `GameSession` with two strategies, both sides firing via
    `viewOf(opponent)`.
  - 1,000 seeded Easy-vs-Easy games produce zero rejected actions, and every game ends with
    one winner (SC-005, Easy part).

### Implementation for User Story 1

- [X] T053 [US1] Create `src/core/include/bs/core/Board.h` and `src/core/src/Board.cpp`:
  - `enum class CellShotState { Untouched, Miss, Hit, Marked }`. `ShotResult` comes from
    `Types.h`.
  - `Board(const Fleet&)`, `shoot(Coord)` (which returns the result, the sunk ship ID and the
    changed cells), `markAround(ShipId)`, `isFiredUpon`, `shotState`, `shipState` and
    `remainingCells`.
  - Marking happens on every sink, unless the caller defers it (needed by Help in US4: add a
    `bool deferMarking` parameter, default false).

  This depends on T029 and makes T043 pass.
- [X] T054 [US1] Create the session types and the public view:
  - `src/core/include/bs/core/GameSession.h`: `Difficulty`, `Phase`, `Knowledge`,
    `ITargetView`, `CellChange`, `ActionStatus`, `ShotOutcome` and `StartStatus`, as in
    core-api.md.
  - `src/core/src/TargetView.cpp`: a concrete `TargetView : ITargetView` over a `const Board&`.

  This makes T044 pass.
- [X] T055 [US1] Implement `GameSession` in `src/core/src/GameSession.cpp`:
  - the constructor `(Difficulty, Fleet computerFleet)`;
  - `playerFleet()`, which throws `std::logic_error` when phase != Placement;
  - `startBattle`, `fire` with the full rejection table, `phase`, `currentTurn`, `winner`,
    `turnsTaken`, `viewOf`, `ownFleet` and `difficulty`;
  - `helpAvailable` returns true for now, and `useHelp` is added in US4.

  This depends on T053 and T054, and makes T045 pass.
- [X] T056 [US1] Create `src/core/include/bs/core/AutoPlacer.h` and
  `src/core/src/BacktrackingPlacer.cpp`:
  - `AutoPlaceStatus`, `AutoPlaceResult` and the `IFleetPlacer` interface.
  - `BacktrackingPlacer::placeAll`: a randomized complete backtracking search, largest ship
    first, with legal positions in both orientations shuffled via `IRandom`, and a node budget
    of 2,000,000 (research R7).
  - Declare `placeRemaining` so that it returns `CannotFit` until US2.

  This makes T046 and T047 pass.
- [X] T057 [US1] Create `src/core/include/bs/core/Ai.h`, `src/core/src/RandomStrategy.cpp`
  and `src/core/src/StrategyFactory.cpp`:
  - `AiAction`, the `ITargetingStrategy` interface and `RandomStrategy`, as in core-api.md.
  - `makeStrategy(Difficulty, IRandom&)` returns a `RandomStrategy` for Easy, and throws
    `std::logic_error("not implemented")` for Medium and Hard until US3.

  This makes T048 and T052 pass.
- [X] T058 [US1] Create `src/app/include/bs/app/GameSetup.h` and `src/app/src/GameSetup.cpp`,
  with the constructor `(const IFleetPlacer&, IRandom&)` and `newSession(Difficulty)`. It
  places the computer fleet with `placeAll` and returns
  `std::make_unique<GameSession>(difficulty, fleet)`. This makes T049 pass.
- [X] T059 [US1] Create `src/app/include/bs/app/PlacementController.h` and
  `src/app/src/PlacementController.cpp`, with the constructor `(Fleet&, const IFleetPlacer&,
  IRandom&)`, `autoPlaceAll()`, `canStartBattle()` and `dockShips()`. The fleet passed in is always `session.playerFleet()`. This makes T050 pass.
- [X] T060 [US1] Create `src/app/include/bs/app/BattleController.h` and
  `src/app/src/BattleController.cpp`:
  - `enum class Mode { AwaitingPlayer, PlayerHelpTargeting, ComputerActing, ConfirmExit,
    GameOver }`.
  - `ActionFeedback{ActionStatus status; bs::app::TextKey message; bool hasMessage;
    std::vector<CellChange> changes;}` and `TurnIndicator`.
  - The constructor, as in app-api.md, taking `IGameAudio&`, with
    `computerDelaySeconds = 0.8`.
  - `onPlayerFire`, `tick` and `turnIndicator`.

  This makes T051 pass.
- [X] T061 [US1] Add the US1 strings to `TextKeys.h` and all three `.lang` files:
  - `placement.title`, `placement.auto_all`, `placement.start`,
    `placement.error.fleet_incomplete`
  - `battle.turn.player`, `battle.turn.computer` (with `{name}`), `battle.board.own`,
    `battle.board.target`
  - `battle.reject.already_fired`, `battle.reject.not_your_turn`
  - `battle.winner` (`{name} wins!` / `{name} побеждает!` / `{name} chiến thắng!`)
  - `gameover.new_game`, `gameover.menu`

  T026 must stay green.
- [X] T062 [P] [US1] Create `src/game/nodes/BoardNode.{h,cpp}`. It draws a 10 × 10 grid with
  `DrawNode`, with the numerals 1–10 labelling both the columns and the rows (generated in
  code; they read the same in every language). It renders the
  `Knowledge` states:
  - Unknown: blank. Miss: dot. Hit: cross.
  - Sunk: ship outline with a darker tint.
  - Marked: a faint small dot, visually distinct from Miss.

  It can optionally draw its own ships from a `const Fleet&`, maps a mouse position to a
  `Coord` (`cellAt(Vec2) -> bool + Coord`), and applies a `std::vector<CellChange>` as an
  incremental redraw in the same frame (SC-009).
- [X] T063 [US1] Create `src/game/scenes/PlacementScene.{h,cpp}` (MVP form):
  - The player `BoardNode` and the buttons `placement.auto_all`, `placement.start` and
    `common.back`.
  - On enter, it calls `GameSetup::newSession(difficulty)`, stores the session in
    `GameServices::currentSession`, and builds `PlacementController` on
    `session.playerFleet()`.
  - Start calls `canStartBattle()`. If it's false, it shows `placement.error.fleet_incomplete`.
    If it's true, it calls `session.startBattle()` and navigates to Battle.
  - Back and Esc go to the menu with no confirmation.
  - Implement `SceneNavigator::showPlacement`, with `MenuScene` New Game →
    `showPlacement(Difficulty::Easy)`.
  - Add `std::unique_ptr<GameSession> currentSession` and a `GameSetup` to `GameServices`.
- [X] T064 [US1] Create `src/game/scenes/BattleScene.{h,cpp}`:
  - The own `BoardNode` sits on the left and the target `BoardNode` (a `viewOf(Computer)`
    projection only) on the right.
  - A turn banner shows `battle.turn.player` or `battle.turn.computer` (FR-024), and name
    labels come from `ILocalizer::displayName`.
  - A left-click on the target board calls `onPlayerFire`. A rejection message shows briefly.
  - `scheduleUpdate()` makes `update(dt)` call `controller.tick(dt)`, and the returned changes
    are applied in the same frame.
  - On `GameOver`, it shows `GameOverLayer`, and it calls `enterScreen(Battle)`.
  - Implement `SceneNavigator::showBattle`.
- [X] T065 [US1] Create `src/game/scenes/GameOverLayer.{h,cpp}`. It is a modal overlay
  (swallowing touches) over the final battle state. It shows `battle.winner` with `{name}` set
  to the winner's display name (FR-045), and the buttons `gameover.new_game` (→
  `showPlacement(Easy)`; US3 changes this) and `gameover.menu` (FR-046), with no confirmation.
- [ ] T066 [US1] Run quickstart §5 step 1 on macOS: launch → New Game → Auto-place all →
  Start → play to the end, timing launch to first shot (under 60 s, SC-001). Fix any defect
  test-first (Principle II).

**Checkpoint**: The MVP is playable end to end against Easy, and all unit and `sim` tests pass.

---

## Phase 4: User Story 2 - Arrange the Fleet Manually, Automatically or Both (Priority: P1)

**Goal**: Mouse drag-and-drop placement with a live legality preview, rotation, removal,
"auto-place remaining", and a placement lock once the battle starts.

**Independent Test**: Place some ships by hand, auto-fill the rest, move, rotate and remove
ships, and try illegal positions. The battle starts only with a complete legal fleet
(spec US2, quickstart §5 step 2).

### Tests for User Story 2 (write first, confirm they FAIL)

- [X] T067 [P] [US2] Extend `tests/core/AutoPlacerTest.cpp` with `placeRemaining`:
  - With 0–9 hand-placed ships it returns `Ok`, a complete legal fleet, and every hand-placed
    ship at its **exact** original placement (FR-009).
  - For a hand-built layout that leaves no room for the missing ships (construct it and prove
    it with an exhaustive check in the test), it returns `CannotFit` with `fleet` equal to the
    input (FR-012).
  - A complete input returns `Ok` unchanged.
- [X] T068 [P] [US2] Extend `tests/core/AutoPlacerBulkTest.cpp` with mixed placement over
  10,000 seeds. For each, take a `placeAll` fleet, remove a random subset, run
  `placeRemaining`, and assert `Ok`, a legal and complete fleet, and the kept ships unchanged
  (SC-002, mixed part).
- [X] T069 [P] [US2] Extend `tests/core/GameSessionTurnTest.cpp`: after `startBattle()`,
  `playerFleet()` throws `std::logic_error` (FR-015, US2 scenario 9).
- [X] T070 [P] [US2] Extend `tests/app/PlacementControllerTest.cpp` with drag and drop:
  - `beginDragFromDock(id)` then `hover(c)` returns a `Preview` with the ship's cells, a
    `legal` flag and a `PlacementError` reason (OutOfBounds, Overlaps or Touches, including
    diagonals).
  - `rotate()` during a drag flips the preview orientation.
  - `drop(c)` on a legal cell returns `Placed`. On an illegal cell it returns
    `Rejected(reason)` and the ship stays in the dock, or at its previous position when it was
    dragged from the board.
  - `beginDragPlaced(id)` followed by `dropOnDock()` removes the ship.
  - `rotate()` with a placed ship selected and no drag rotates it in place if legal, and
    otherwise returns `Rejected(reason)` with no change.
  - `cancelDrag()` restores the previous state.
  - `autoPlaceRemaining()` returns `Ok` and only adds ships, or returns `CannotFit` with the
    layout unchanged.
  - `dockShips()` lists the unplaced ShipIds grouped by length.

### Implementation for User Story 2

- [X] T071 [US2] Implement `BacktrackingPlacer::placeRemaining` in
  `src/core/src/BacktrackingPlacer.cpp`. It seeds the search with the partial fleet's ships
  fixed and searches only the missing ShipIds, largest first. Exhausting the search or the
  budget returns `CannotFit` with the input fleet. This makes T067 and T068 pass.
- [X] T072 [US2] Confirm `GameSession::playerFleet()` throws `std::logic_error` outside
  `Placement` in `src/core/src/GameSession.cpp`, adjusting it if T055 did not do this. This
  makes T069 pass.
- [X] T073 [US2] Extend `src/app/include/bs/app/PlacementController.h` and
  `src/app/src/PlacementController.cpp` with `Preview`, `DropResult`, `AutoResult`,
  `beginDragFromDock`, `beginDragPlaced`, `hover`, `rotate`, `drop`, `dropOnDock`,
  `cancelDrag`, `select(ShipId)` and `autoPlaceRemaining`, as in app-api.md. This makes T070
  pass.
- [X] T074 [US2] Add the US2 strings to `TextKeys.h` and all three `.lang` files:
  - `placement.auto_remaining`, `placement.rotate`, `placement.dock`
  - `placement.dock.count` (`×{n}` in all three languages; it's a key so a translator can
    change the format later)
  - `placement.error.out_of_bounds`, `placement.error.overlaps`, `placement.error.touches`,
    `placement.error.cannot_fit`
  - `placement.hint` (drag, right-click to rotate)
- [X] T075 [P] [US2] Create `src/game/nodes/DockNode.{h,cpp}` and
  `src/game/nodes/ShipNode.{h,cpp}`. The dock shows the unplaced ships grouped by length, each
  group with a count from `ILocalizer::format(placement.dock.count, {{"n", …}})` shown in a
  `LocalizedLabel`. `ShipNode` is a draggable ship visual in either orientation. Neither makes
  decisions; both report mouse events to the scene.
- [X] T076 [US2] Extend `src/game/scenes/PlacementScene.{h,cpp}` to full placement:
  - **Mouse**: `EventListenerMouse` maps left-button down on a dock or placed ship to
    `beginDrag…`, move to `hover` (a green or red preview, plus a tooltip from the
    `placement.error.*` key on illegal), up to `drop` or `dropOnDock`, and right-click to
    `rotate`.
  - **Buttons**: `placement.rotate` and `placement.auto_remaining`. On `CannotFit` it shows
    `placement.error.cannot_fit` and nothing moves.
  - **Keyboard**: `R` rotates.
  - Start stays disabled-looking until the fleet is complete, and clicking it shows
    `placement.error.fleet_incomplete` (FR-014).
- [ ] T077 [US2] Run quickstart §5 step 2 on macOS. Fix any defect test-first.

**Checkpoint**: US1 and US2 both work. A battle can start from a hand-made, mixed or
fully automatic fleet.

---

## Phase 5: User Story 4 - Use the "Help" Ability (Priority: P2)

**Goal**: Each side has one cross-shaped Help per game. It replaces a shot, ends the turn,
skips off-board and fired cells, and can win the game.

> **Superseded in part (2026-10-20)**: Help now has three uses per side, may replace an extra
> shot after a hit, and shows a remaining-uses counter. See Phase 10 (T117, T118, T121, T123,
> T125–T127).

**Independent Test**: Use Help in the centre, at an edge, in a corner and next to fired cells.
Check the affected cells, that the turn passes, that a second use is impossible, and that the
button shows "used" (spec US4, quickstart §5 step 4).

### Tests for User Story 4 (write first, confirm they FAIL)

- [X] T078 [P] [US4] Write `tests/core/HelpTest.cpp` (parameterized where useful):
  - In the centre, 5 cells are affected, in the order centre, up, down, left, right.
  - At an edge 4 are affected, and in a corner 3 (FR-027, FR-028).
  - Cells already Miss, Hit or Marked are skipped and their state is unchanged.
  - When all on-board cross cells are fired upon, it returns `RejectedNoEffect` and the help
    is **not** consumed (FR-029).
  - A second use returns `RejectedAlreadyUsed`.
  - The player and computer helps are independent (US4 scenario 7).
  - The turn passes even when the help hits (FR-030).
  - It is rejected when not the user's turn or not in Battle.
  - When the help sinks a ship, the cross cells are resolved first and the marking happens
    afterwards. A cross cell next to the sunk ship keeps its real Miss or Hit result.
  - A help that destroys the last ship cells ends the game with the user as winner, and no
    turn pass happens (FR-031).
  - `helpAvailable(side)` becomes false after a use.
- [X] T079 [P] [US4] Extend `tests/app/BattleControllerTest.cpp` with Help:
  - `armHelp()` enters `PlayerHelpTargeting` only from `AwaitingPlayer` and only if the help
    is available. `disarmHelp()` returns to `AwaitingPlayer`.
  - `onPlayerHelp(c)` applies and moves to `ComputerActing`, or to `GameOver` if it wins.
  - `RejectedNoEffect` stays in targeting mode with TextKey `battle.reject.help_no_effect`.
  - `playerHelpAvailable()` is false after a use, and `armHelp()` is then a no-op.
  - `onPlayerFire` is rejected while targeting.
  - `MockGameAudio` receives `onHelp`.

### Implementation for User Story 4

- [X] T080 [US4] Add `HelpOutcome` to `src/core/include/bs/core/GameSession.h` and implement
  `GameSession::useHelp` and real `helpAvailable` in `src/core/src/GameSession.cpp`, using
  `Board::shoot(c, /*deferMarking*/ true)` and then `markAround` for each ship sunk by the
  cross. This makes T078 pass.
- [X] T081 [US4] Extend `src/app/src/BattleController.cpp` and
  `src/app/include/bs/app/BattleController.h` with `armHelp`, `disarmHelp`, `onPlayerHelp`
  and `playerHelpAvailable`. This makes T079 pass.
- [X] T082 [US4] Add the US4 strings to `TextKeys.h` and all three `.lang` files:
  `battle.help`, `battle.help.used`, `battle.help.hint` (click a target cell, right-click or
  Esc to cancel) and `battle.reject.help_no_effect`.
- [X] T083 [P] [US4] Create `src/game/nodes/HelpPreviewNode.{h,cpp}`. It is a cross overlay
  that follows the hovered target cell, is clipped at the board edges, and draws
  already-fired cells as "skipped" (hatched). It is pure drawing and gets its cells from the
  core `Coord` helpers.
- [X] T084 [US4] Extend `src/game/scenes/BattleScene.{h,cpp}`:
  - **Help button**: shows `battle.help` when available, and `battle.help.used` disabled when
    used (FR-032).
  - **Targeting**: clicking the button calls `armHelp()` and shows `HelpPreviewNode` with the
    `battle.help.hint` text. A left-click calls `onPlayerHelp`, and right-click or Esc calls
    `disarmHelp()`.
  - All cross changes render in the same frame.
- [ ] T085 [US4] Run quickstart §5 step 4 on macOS. Fix any defect test-first.

**Checkpoint**: The player's Help works in every edge case, and the core Help rule is ready
for the Hard AI.

---

## Phase 6: User Story 3 - Choose a Difficulty Level (Priority: P2)

**Goal**: Easy, Medium and Hard computer opponents, chosen before each game. Medium finishes
wounded ships along the line, and Hard adds one well-aimed Help.

> **Superseded in part (2026-10-20)**: Hard now uses Help three times, on three distinct
> target turns in 1–20. See Phase 10 (T119, T120, T122, T124, T128).

**Independent Test**: Watch the computer's shots at each level. Medium fires only at the ends
of a line once two hits are aligned, and Hard uses Help once within its first ~20 turns. The
`sim` suite meets SC-004 and SC-005 (spec US3, quickstart §4 and §5 step 3).

### Tests for User Story 3 (write first, confirm they FAIL)

- [X] T086 [P] [US3] Write `tests/core/HuntTargetStrategyTest.cpp`, covering FR-034 and
  research R8:
  - With no Hit cells it picks a random `Unknown` cell.
  - With one isolated Hit it picks only among that cell's `Unknown` orthogonal neighbours.
  - With two or more aligned Hits it picks only the `Unknown` cell at either end of the line,
    **never one beside the line**, and when one end is blocked (edge, Miss or Marked) it picks
    the other.
  - After the cluster becomes `Sunk`, it returns to random.
  - With several wounded clusters, it finishes the one whose first Hit it observed earliest
    (tracked from its own shot history across calls).
  - `Sunk` and `Marked` cells are never targeted.
- [X] T087 [P] [US3] Write `tests/core/HardStrategyTest.cpp`, covering FR-035:
  - **Target turn**: drawn once at construction with `nextInt(1, 20)` (assert with
    `ScriptedRandom`).
  - **When Help is returned**: only when `firstActionOfTurn`, `turnIndex >= target`, no
    wounded cluster exists, and `helpAvailable`.
  - **Waiting**: while a wounded cluster exists on the target turn, it shoots (Medium rules)
    and uses Help on the first later turn with no wounded cluster.
  - **Centre**: chosen uniformly among cells whose entire on-board cross is `Unknown`. If
    there are none, among the cells with the most `Unknown` cross cells.
  - **Delegation**: never Help when `helpAvailable == false`. All other actions match
    `HuntTargetStrategy` given the same random stream.
- [X] T088 [P] [US3] Extend `tests/core/StrategyContractTest.cpp` to also instantiate
  `HuntTargetStrategy` and `HardStrategy`. Every action must be a `Shot` at `Unknown`, or a
  `Help` with at least one `Unknown` cross cell and only when `helpAvailable`. Easy and Medium
  must never return Help (FR-036). Add a `makeStrategy` test that checks the dynamic type for
  each `Difficulty`.
- [X] T089 [P] [US3] Extend `tests/app/BattleControllerTest.cpp`: when the computer's
  strategy returns `Help`, `tick` applies `session.useHelp`, returns its changes, calls
  `IGameAudio::onHelp`, and passes the turn to the player (or `GameOver`).
- [X] T090 [P] [US3] Extend `tests/sim/AiSimulationTest.cpp`, using fixed seeds 1–N:
  - 1,000 games for each of Easy, Medium and Hard produce zero rejected actions and zero
    repeated cells (SC-005).
  - Over 100 Medium-vs-Easy games, Medium wins ≥ 60%.
  - Over 100 Hard-vs-Medium games, Hard wins more than it loses (SC-004).
  - Alternate which side shoots first between games. Print the win rates.
  - If the SC-004 assertion fails, stop and raise it via `/speckit-clarify`. Do not tune
    behaviour outside FR-034/FR-035.

### Implementation for User Story 3

- [X] T091 [US3] Create `src/core/src/HuntTargetStrategy.cpp` and declare it in
  `src/core/include/bs/core/Ai.h`. It does cluster detection over `Hit` cells, keeps the
  first-observed order of hits from its own history, and targets single-hit neighbours or
  line ends. This makes T086 pass.
- [X] T092 [US3] Create `src/core/src/HelpPlanner.cpp` and `src/core/src/HardStrategy.cpp`
  and declare them in `Ai.h`. `HelpPlanner` holds the target turn drawn from `[1, 20]` and
  picks the centre. `HardStrategy` is a decorator over `HuntTargetStrategy` plus
  `HelpPlanner`. Then update `makeStrategy` in `src/core/src/StrategyFactory.cpp` for Medium
  and Hard. This makes T087 and T088 pass.
- [X] T093 [US3] Extend `src/app/src/BattleController.cpp` to execute `AiAction::Help` with
  `firstActionOfTurn` and `turnIndex = session.turnsTaken(Computer)`. This makes T089 pass.
- [X] T094 [US3] Run `ctest -L sim` and confirm T090 passes. Record the observed win rates in
  the commit message.
- [X] T095 [US3] Add the US3 strings to `TextKeys.h` and all three `.lang` files:
  `difficulty.title`, `difficulty.easy`, `difficulty.medium` and `difficulty.hard`.
- [X] T096 [US3] Create `src/game/scenes/DifficultyOverlay.{h,cpp}`. It is a modal over
  `MenuScene` with the buttons Easy, Medium, Hard and `common.back`, and choosing a level
  calls `showPlacement(difficulty)` (FR-001). Re-wire `MenuScene` New Game and the
  `GameOverLayer` `gameover.new_game` button to open this overlay (GameOver goes to the menu
  with the overlay open). `PlacementScene` and `BattleScene` pass the difficulty through to
  `GameSession` and `makeStrategy`.
- [ ] T097 [US3] Run quickstart §5 step 3 on macOS. Fix any defect test-first.

**Checkpoint**: All gameplay stories (US1–US4) work, and the `sim` suite passes.

---

## Phase 7: User Story 5 - Adjust Settings (Priority: P3)

**Goal**: Sound and music toggles, a live EN/RU/VI language switch, and the player name, all
persisted across restarts.

**Independent Test**: Change each setting, see the effect immediately, restart the game, and
check the settings are kept (spec US5, quickstart §5 step 5).

### Tests for User Story 5 (write first, confirm they FAIL)

- [X] T098 [P] [US5] Write `tests/app/SettingsControllerTest.cpp`, using a real
  `SettingsService` over `InMemorySettingsStore`, plus `FakeLocalizer` and `MockGameAudio`:
  - `toggleSound()` and `toggleMusic()` flip the setting, persist it, and call
    `IGameAudio::onSettingsChanged()`.
  - `selectLanguage(Ru)` stores `Ru` in the settings and calls
    `ILocalizer::setLanguage(Ru)`.
  - `filterNameInput(text)` caps live input at 20 code points.
  - `commitName("  ")` stores `""`, and the display becomes `name.player_default`.
  - `commitName` of 25 Cyrillic characters stores the first 20.
  - `languages()` returns En, Ru, Vi with the display keys `lang.en`, `lang.ru`, `lang.vi`.

### Implementation for User Story 5

- [X] T099 [US5] Create `src/app/include/bs/app/SettingsController.h` and
  `src/app/src/SettingsController.cpp`, with the constructor `(ISettings&, ILocalizer&,
  IGameAudio&)` and the operations from T098, as in app-api.md. This makes T098 pass.
- [X] T100 [US5] Add the US5 strings to `TextKeys.h` and all three `.lang` files:
  `settings.title`, `settings.sound`, `settings.music`, `settings.language`,
  `settings.player_name`, `settings.on`, `settings.off` and
  `settings.name_placeholder`.
- [X] T101 [US5] Create `src/game/scenes/SettingsScene.{h,cpp}`:
  - Toggles for sound and music.
  - A language selector of three buttons labelled `lang.en`, `lang.ru` and `lang.vi`, with the
    current one highlighted.
  - A `ui::TextField` for the name, with input passed through
    `SettingsController::filterNameInput` and committed on Enter, focus loss or Back.
  - The `common.back` button.
  - Every label is a `LocalizedLabel`, so the whole screen re-renders immediately on a
    language change (FR-040).
  - Implement `SceneNavigator::showSettings` and enable the `MenuScene` Settings button.
- [X] T102 [US5] In `src/game/AppDelegate.cpp`, at startup apply the persisted language to
  `Localizer` and start the menu music only if `musicEnabled`. Confirm persistence across an
  app restart (FR-043).
- [ ] T103 [US5] Run quickstart §5 step 5 on macOS, including a 20-character name of wide
  glyphs ("ШШШ…", "WWW…", "ẨẨẨ…") on the battle and game-over screens with no clipping. Fix
  any defect test-first.

**Checkpoint**: Settings work and persist, and every screen re-localizes live.

---

## Phase 8: User Story 6 - Leave a Battle Safely (Priority: P3)

**Goal**: Leaving an in-progress battle needs confirmation. Cancelling resumes the exact
state, and the computer never acts while the dialog is open.

**Independent Test**: In a battle, ask to leave, cancel, and check nothing changed. Ask again,
confirm, and the menu appears. Placement and game over leave without a prompt (spec US6,
quickstart §5 step 6).

### Tests for User Story 6 (write first, confirm they FAIL)

- [X] T104 [P] [US6] Extend `tests/app/BattleControllerTest.cpp` with exit:
  - `requestExit()` from `AwaitingPlayer`, `PlayerHelpTargeting` or `ComputerActing` enters
    `ConfirmExit`.
  - While in `ConfirmExit`, `tick(10.0)` performs no computer action and returns no changes,
    and `onPlayerFire` and `onPlayerHelp` are rejected.
  - `cancelExit()` restores the exact previous mode, the session state, and the remaining
    computer delay.
  - `confirmExit()` returns true.
  - `requestExit()` in `GameOver` does nothing (game over leaves without confirmation,
    US6 scenario 4).

### Implementation for User Story 6

- [X] T105 [US6] Extend `src/app/include/bs/app/BattleController.h` and
  `src/app/src/BattleController.cpp` with `requestExit`, `cancelExit` and `confirmExit`,
  saving the previous mode and the accumulated delay. This makes T104 pass.
- [X] T106 [US6] Add the US6 strings to `TextKeys.h` and all three `.lang` files:
  `exit.title` (Leave battle?), `exit.message` (the current game will be lost), `exit.stay`,
  `exit.leave` and `battle.menu`.
- [X] T107 [US6] Create `src/game/scenes/ConfirmDialog.{h,cpp}`. It is a reusable modal
  (a dimmed background that swallows touches) taking title and message TextKeys and two
  button TextKeys plus callbacks.
- [X] T108 [US6] Extend `src/game/scenes/BattleScene.{h,cpp}`:
  - The `battle.menu` button and Esc (when not in Help targeting) call `requestExit()` and
    show `ConfirmDialog`.
  - Stay calls `cancelExit()`. Leave calls `confirmExit()`, then `showMenu()`, and discards
    the session (FR-044).
  - Confirm that `PlacementScene` Back/Esc and the `GameOverLayer` buttons navigate without a
    prompt.
- [ ] T109 [US6] Run quickstart §5 step 6 on macOS. Fix any defect test-first.

**Checkpoint**: All six user stories work on macOS.

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Cross-platform verification, layout robustness, and final validation against the
constitution and the success criteria.

- [ ] T110 Build and run on Windows: `cmake -S . -B build\win -G "Visual Studio 17 2022" -A
  Win32`. Fix any MSVC `/W4 /WX` warnings in project code without suppressing them globally.
  Confirm that `ctest -C Debug` passes and that UTF-8 text renders (because of `/utf-8`).
- [ ] T111 [P] Do a layout pass on every scene in `src/game/scenes/` in all three languages.
  Find the longest string per key (usually ru or vi), confirm no `LocalizedLabel` clips at
  1280 × 720 or in a resized window, and adjust the max widths and layouts.
- [X] T112 [P] Write a Principle III code audit as `tests/core/FairnessCompileTest.cpp`, a
  test that documents and asserts (through `static_assert` on the signatures) that
  `ITargetingStrategy::choose` takes only `const ITargetView&` and has no `Board`/`Fleet`
  parameter.
- [ ] T113 [P] Run the full suite with `ctest --output-on-failure` (unit, `sim` and
  architecture) on macOS and Windows. Confirm zero warnings in a clean build on both.
- [X] T114 [P] Add `README.md` at the repo root: a one-paragraph description, links to
  `specs/001-battleship-game/quickstart.md` for the build/test/run commands, the layer
  overview (core → app → game), and licence notes for Noto Sans and the audio assets.
- [ ] T115 Run the whole of quickstart.md §4 and §5 on macOS and on Windows. Tick each
  SC-001–SC-009 check, and file any failure as a test-first fix.
- [X] T116 Do a constitution compliance review of the full diff:
  - Principles I–VI: no engine in core/app, tests written first, fair AI, no literal display
    strings in `src/game` (grep for quoted non-key literals passed to labels), offline only,
    SOLID rules.
  - Technology Stack: C++14, smart pointers, no raw owning pointers.
  - Record the result in the PR description.

---

## Phase 10: Help Revision — Three Uses per Side (2026-10-20)

**Purpose**: migrate the single-use Help built in phases 5 and 6 to the revised spec:
- three uses per side (FR-026);
- Help may replace the first shot or any extra shot after a hit (FR-027);
- each use consumes one use and ends the turn (FR-030);
- the player's counter, plus the computer's counter on Hard only (FR-032);
- Hard draws three distinct target turns (FR-035);
- the whole cross is revealed as one batch (FR-048).

The design is in [data-model.md](data-model.md) §3–4,
[contracts/core-api.md](contracts/core-api.md) (`kHelpUsesPerGame`, `helpUsesLeft`,
`HelpPlanner`), [contracts/app-api.md](contracts/app-api.md) (`BattleController` counters),
[contracts/ui-screens.md](contracts/ui-screens.md) and
[contracts/localization.md](contracts/localization.md). The rule in research R8 decides when a
scheduled use is *due*.

**Independent Test**: In one game, the player uses Help three times (once as an extra shot
after a hit). The button goes "Help (3)" → "(2)" → "(1)" → "Help used up", and a fourth use
is impossible. On Hard, the computer's counter beside its name goes 3 → 0 over its first ~20
turns, never more than one Help per turn. On Easy and Medium no computer counter is shown.

### Tests for the revision (update first, confirm they FAIL)

- [X] T117 [P] [US4] Update `tests/core/HelpTest.cpp`. Replace the single-use assertions
  (around the current `helpAvailable` checks and the `RejectedAlreadyUsed` check) with:
  - `helpUsesLeft(Player)` and `helpUsesLeft(Computer)` both start at `kHelpUsesPerGame` (3).
  - Each applied Help decreases the user's count by exactly 1. The opponent's count does not
    change (US4 scenario 9).
  - `helpAvailable(side)` is true at 3, 2 and 1, and false only at 0.
  - The fourth use returns `RejectedAlreadyUsed` and leaves the state unchanged.
  - `RejectedNoEffect` (every on-board cross cell already fired upon) does **not** decrease
    the count (FR-029).
  - **Help as an extra shot**: the player fires a `Hit` (turn kept), then `useHelp` is
    `Applied`, the count decreases, and the turn passes to the computer (FR-027, US4
    scenario 8).
  - A rejected `useHelp` (wrong turn, wrong phase, off board) never changes the count.
- [X] T118 [P] [US4] Extend `tests/app/BattleControllerTest.cpp`:
  - `playerHelpUsesLeft()` goes 3 → 2 → 1 → 0 across three applied player Helps.
    `playerHelpAvailable()` is false only at 0, and `armHelp()` is then a no-op.
  - After the player's shot is a `Hit` (mode stays `AwaitingPlayer`), `armHelp()` enters
    `PlayerHelpTargeting`, and `onPlayerHelp(c)` applies and moves to `ComputerActing`.
  - `showComputerHelpCounter()` is true for `Difficulty::Hard` and false for Easy and Medium.
  - `computerHelpUsesLeft()` mirrors `session.helpUsesLeft(Computer)` after a computer Help
    performed through `tick`.
  - The `ActionFeedback` of a player Help, and the return value of `tick` for a computer Help,
    hold **all** cross cells plus any sunk-ship and marking changes in one list (FR-048). A
    Help that sinks a ship next to the cross returns the cross results first and the `Marked`
    cells after them.
  - The strategy is called with `int helpUsesLeft` equal to `session.helpUsesLeft(Computer)`
    (assert through a recording fake strategy).
- [X] T119 [P] [US3] Rewrite the Help-timing part of `tests/core/HardStrategyTest.cpp` for
  FR-035 and research R8:
  - **Schedule**: `HelpPlanner(rng)` draws three **distinct** turns in `[1, 20]` with a partial
    Fisher–Yates through `IRandom::nextInt`. Assert the exact `nextInt` calls with
    `ScriptedRandom`, and that `targetTurns()` is sorted ascending. A bulk check over 10,000
    seeds finds no duplicates and nothing outside 1–20.
  - **Due rule**: `shouldUse(turnIndex, firstActionOfTurn, helpUsesLeft, woundedShip)` is true
    only when `firstActionOfTurn`, `helpUsesLeft > 0`, `!woundedShip`, and
    `count(targetTurns ≤ turnIndex) > 3 − helpUsesLeft`.
  - **Deferral**: with targets {3, 4, 10}, a wounded ship on turns 3–5 gives Help on turn 6
    (uses left 3 → 2), Help again on turn 7 (2 → 1), no Help on turns 8–9, and Help on
    turn 10.
  - **One per turn**: never Help on a non-first action. With all three targets due at once,
    the Helps land on three consecutive free turns.
  - **Limit**: never Help when `helpUsesLeft == 0`, even if a target turn is reached.
  - Keep the existing centre-selection and delegation cases. Update their `choose` calls to the
    `int helpUsesLeft` parameter.
- [X] T120 [P] [US3] Update `tests/core/StrategyContractTest.cpp`:
  - The simulated loop passes `int helpUsesLeft`, starting at 3 and decreasing on each
    returned Help.
  - It asserts that Hard returns at most 3 Helps per game, and that each Help is the first
    action of its turn.
  - Easy and Medium still never return Help (FR-036).
- [X] T121 [P] [US3] Extend `tests/sim/AiSimulationTest.cpp`:
  - Pass `session.helpUsesLeft(shooter)` to `choose` instead of `helpAvailable(shooter)`.
  - In every Hard game, assert that the Hard side applies at most 3 Helps, on distinct
    computer turns.
  - Keep the SC-004 and SC-005 assertions unchanged.
- [X] T122 [P] [US3] Update the `static_assert` in `tests/core/FairnessCompileTest.cpp` to the
  new `ITargetingStrategy::choose(const ITargetView&, int, bool, int, IRandom&)` signature.
  It must still have no `Board`/`Fleet` parameter (Principle III).

### Implementation for the revision

- [X] T123 [US4] Core counter:
  - In `src/core/include/bs/core/Coord.h`, add `constexpr int kHelpUsesPerGame = 3;` next to
    `kBoardSize`.
  - In `src/core/include/bs/core/GameSession.h`, replace `std::array<bool, 2> helpUsed_` with
    `std::array<int, 2> helpUsesLeft_{{kHelpUsesPerGame, kHelpUsesPerGame}}`, and add
    `int helpUsesLeft(Side) const`.
  - In `src/core/src/GameSession.cpp`:
    - `useHelp` returns `RejectedAlreadyUsed` when the count is 0 and decrements it only on
      `Applied`;
    - `helpAvailable` returns `helpUsesLeft_[i] > 0`;
    - confirm that `useHelp` is accepted while the user keeps the turn after a hit.

  This makes T117 pass.
- [X] T124 [US3] Strategy signature and planner:
  - In `src/core/include/bs/core/Ai.h`, change `bool helpAvailable` to `int helpUsesLeft` in
    `ITargetingStrategy::choose` and in `RandomStrategy`, `HuntTargetStrategy` and
    `HardStrategy`. Update `src/core/src/RandomStrategy.cpp` and
    `src/core/src/HuntTargetStrategy.cpp`.
  - In `HelpPlanner`, replace `int targetTurn_` and `targetTurn()` with
    `std::vector<int> targetTurns_` and `const std::vector<int>& targetTurns() const`. Remove
    `HardStrategy::helpTurn()`, or make it return `targetTurns()`.
  - In `src/core/src/HelpPlanner.cpp`:
    - the constructor draws 3 distinct turns from 1..`kLatestTurn` by partial Fisher–Yates
      (`nextInt(i, 19)` swaps over the array 1..20), then sorts them;
    - `shouldUse` implements the due rule from T119;
    - `HardStrategy::choose` passes `helpUsesLeft` through.

  This makes T119, T120 and T122 pass. Then update `tests/sim/AiSimulationTest.cpp` callers
  as T121 requires.
- [X] T125 [US4] App counters, in `src/app/include/bs/app/BattleController.h` and
  `src/app/src/BattleController.cpp`:
  - Add `int playerHelpUsesLeft() const`, `int computerHelpUsesLeft() const` and
    `bool showComputerHelpCounter() const` (`session.difficulty() == Difficulty::Hard`).
  - Pass `session_.helpUsesLeft(Side::Computer)` to the strategy.
  - Make sure `armHelp()` is allowed in `AwaitingPlayer` after a hit.

  This makes T118 pass.
- [X] T126 [US4] Strings, with `{count}` placeholders, in
  `src/app/include/bs/app/TextKeys.h` and all three `.lang` files, verbatim from
  [contracts/localization.md](contracts/localization.md):

  | Key | en | ru | vi |
  |---|---|---|---|
  | `battle.help` | Help ({count}) | Помощь ({count}) | Trợ giúp ({count}) |
  | `battle.help.used` | Help used up | Помощь израсходована | Đã hết trợ giúp |
  | `battle.computer_help` (new key `BattleComputerHelp`) | Help: {count} | Помощь: {count} | Trợ giúp: {count} |

  `LocalizationCompletenessTest` must pass with matching placeholders.
- [X] T127 [US4] Update `src/game/scenes/BattleScene.{h,cpp}` (rendering only, no game
  decisions):
  - **Help button**: shows `format(BattleHelp, {count: playerHelpUsesLeft()})` while uses
    remain, and disabled `battle.help.used` at 0. It refreshes after every player action and
    on a language change.
  - **Computer counter**: on Hard only (`showComputerHelpCounter()`), a `LocalizedLabel`
    beside the computer's name shows `battle.computer_help` with `computerHelpUsesLeft()`. It
    refreshes after every `tick` that applied a computer Help, and on a language change.
  - **One batch**: each Help change list (player or computer) is applied to the `BoardNode`
    in one pass on one frame, with no per-cell delay (FR-048).
  - **Layout**: check that the counter fits beside the longest computer name ("Компьютер") at
    1280 × 720.
- [X] T128 [US3] Run `ctest -L sim` with the three-use Hard and confirm SC-004 (Hard wins
  more than it loses against Medium) and SC-005 still pass. Record the new win rates in the
  commit message. If SC-004 fails, stop and raise it via `/speckit-clarify`; do not tune
  outside FR-034/FR-035.
- [ ] T129 [US4] Run `ctest -LE sim` with zero warnings. Then redo quickstart §5 steps 3 and 4,
  which were rewritten for three uses, on macOS. This also closes T085 and T097. Fix any
  defect test-first.

**Checkpoint**: Help matches the 2026-10-20 spec for both sides, and the full test suite is
green.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: no dependencies. T009–T010 (engine) can proceed in parallel with
  T003–T008.
- **Foundational (Phase 2)**: depends on Setup. It **blocks all user stories**.
- **US1 (Phase 3)**: depends on Foundational. It is the MVP.
- **US2 (Phase 4)**: depends on Foundational. It extends US1's `PlacementScene`,
  `PlacementController` and `BacktrackingPlacer` files, so do it after US1.
- **US4 (Phase 5)**: depends on US1 (`GameSession`, `Board`, `BattleController`,
  `BattleScene`).
- **US3 (Phase 6)**: depends on US1, and on **US4's core Help** (T080) because Hard uses it.
- **US5 (Phase 7)**: depends on Foundational only (it needs the services). It can run in
  parallel with US2–US4 when staffed.
- **US6 (Phase 8)**: depends on US1 (`BattleController`, `BattleScene`).
- **Polish (Phase 9)**: depends on all stories.
- **Help revision (Phase 10)**: depends on phases 5 and 6, which are already built. Inside
  it: T117–T122 first (all `[P]`). Then T123 → T124 (the signature change touches the files
  T123 builds on) → T125 → T126 → T127. T128 and T129 come last. Do Phase 10 before closing
  T085, T097, T113 and T115, which validate Help.

### Story completion order

```text
Setup → Foundational ─┬─▶ US1 (MVP) ─┬─▶ US2
                      │              ├─▶ US4 ─▶ US3
                      │              └─▶ US6
                      └─▶ US5 (independent of gameplay stories)
                                         all ─▶ Polish
                      US4 + US3 (built) ─▶ Phase 10 Help revision ─▶ T113/T115
```

### Within each user story

1. The tests (`[P]` where they are separate files) are written and **fail**.
2. Core types and rules.
3. App controllers.
4. Strings in all three languages.
5. Rendering nodes and scenes.
6. The quickstart walkthrough.

### Parallel opportunities

- **Setup**: T003, T004, T008, T011 and T012 in parallel, with T009 alongside.
- **Foundational**: the interfaces T013–T017 in parallel. All tests T018–T026 in parallel.
  The implementations T027, T028, T030 and T031 in parallel, and the adapters T036, T037 and
  T041 in parallel.
- **US1**: tests T043–T052 all in parallel. T062 (`BoardNode`) in parallel with the
  core/app implementation.
- **US2**: tests T067–T070 in parallel. T075 in parallel with T071–T073.
- **US4**: T078 and T079 in parallel. T083 in parallel with T080–T081.
- **US3**: T086–T090 in parallel.
- **Phase 10**: tests T117–T122 in parallel. T126 (strings) can run in parallel with
  T123–T125.
- **Across stories**: US5 can be built by a second developer while US2–US4 proceed.

---

## Parallel Example: User Story 1

```bash
# All US1 tests together (each must FAIL first):
Task: "T043 BoardTest in tests/core/BoardTest.cpp"
Task: "T044 TargetViewTest in tests/core/TargetViewTest.cpp"
Task: "T045 GameSessionTurnTest in tests/core/GameSessionTurnTest.cpp"
Task: "T046 AutoPlacerTest in tests/core/AutoPlacerTest.cpp"
Task: "T047 AutoPlacerBulkTest in tests/core/AutoPlacerBulkTest.cpp"
Task: "T048 RandomStrategyTest + StrategyContractTest in tests/core/"
Task: "T049 GameSetupTest in tests/app/GameSetupTest.cpp"
Task: "T050 PlacementControllerTest in tests/app/PlacementControllerTest.cpp"
Task: "T051 BattleControllerTest in tests/app/BattleControllerTest.cpp"
Task: "T052 AiSimulationTest (Easy) in tests/sim/AiSimulationTest.cpp"

# The rendering node in parallel with the core work:
Task: "T062 BoardNode in src/game/nodes/BoardNode.{h,cpp}"
```

## Parallel Example: User Story 3

```bash
Task: "T086 HuntTargetStrategyTest in tests/core/HuntTargetStrategyTest.cpp"
Task: "T087 HardStrategyTest in tests/core/HardStrategyTest.cpp"
Task: "T088 StrategyContractTest extension in tests/core/StrategyContractTest.cpp"
Task: "T089 BattleControllerTest (computer Help) in tests/app/BattleControllerTest.cpp"
Task: "T090 AiSimulationTest (SC-004/SC-005) in tests/sim/AiSimulationTest.cpp"
```

## Parallel Example: Phase 10 (Help revision)

```bash
Task: "T117 HelpTest three uses in tests/core/HelpTest.cpp"
Task: "T118 BattleControllerTest counters in tests/app/BattleControllerTest.cpp"
Task: "T119 HardStrategyTest schedule in tests/core/HardStrategyTest.cpp"
Task: "T120 StrategyContractTest in tests/core/StrategyContractTest.cpp"
Task: "T121 AiSimulationTest in tests/sim/AiSimulationTest.cpp"
Task: "T122 FairnessCompileTest in tests/core/FairnessCompileTest.cpp"
```

---

## Implementation Strategy

### MVP first (User Story 1 only)

1. Phase 1 Setup, then Phase 2 Foundational.
2. Phase 3 US1: automatic placement, Easy AI, battle, game over.
3. **Stop and validate**: run `ctest` (unit + `sim`) and quickstart §5 step 1. That gives a
   playable game.

### Incremental delivery

| Step | Adds |
|---|---|
| + US2 | manual and mixed placement (completes the P1 scope) |
| + US4 | Help for the player |
| + US3 | difficulty levels, Medium/Hard AI, SC-004 simulation (completes the P2 scope) |
| + US5 | settings and live language switch |
| + US6 | exit confirmation (completes the P3 scope) |
| Polish | Windows verification, layout, full quickstart |
| Phase 10 | three Help uses per side, counters, Hard's three-turn schedule (2026-10-20 spec) |

Each step leaves all earlier stories working, and the full test suite must stay green.

### Parallel team strategy

1. The whole team does Setup + Foundational.
2. Developer A: US1 → US2 → US4 → US3. Developer B: US5, then US6 (after US1 lands), then
   the Windows build.

---

## Notes

- `[P]` means a different file with no dependency on an incomplete task.
- Write the tests first, watch them fail, then implement. A test that never failed proves
  nothing.
- Commit after each task, or after each test+implementation pair, with
  `ctest -LE sim` green.
- Every new user-facing string goes into all three `.lang` files in the same commit.
- Stop at any checkpoint to validate a story on its own.
