<!--
Sync Impact Report
- Version change: 1.0.0 → 1.1.0 (MINOR: new principle + new section + expanded guidance;
  no principle removed or made incompatible)
- Modified principles:
  - I. Rules Engine Independent of Presentation: now explicitly forbids cocos2d-x in the core
    and requires the core to build and test as a separate library
  - II. Test-First Game Logic (NON-NEGOTIABLE) → II. Test-Driven Development (NON-NEGOTIABLE):
    TDD scope widened from the rules core to all non-rendering code
  - III, IV, V: unchanged
- Added principles: VI. SOLID Design
- Added sections: Technology Stack (C++, cocos2d-x, CMake, unit test framework)
- Modified sections: Development Workflow (build + test gate, reviewer SOLID/TDD check)
- Removed sections: none
- Templates: plan-template.md, spec-template.md and tasks-template.md read this file at runtime
  and need no edits; the plan's Constitution Check now covers Principles I–VI.
- Deferred TODOs: none (exact C++ standard, cocos2d-x version, test framework and target
  platforms are left to the implementation plan on purpose, within the limits set below)
-->

# BattleShip Constitution

## Core Principles

### I. Rules Engine Independent of Presentation

All game rules MUST live in a self-contained core: board and fleet, placement validation,
auto-placement, turns, shooting, the Help ability, win detection and AI targeting. This core
MUST NOT depend on rendering, input handling, audio, localization or storage, and MUST NOT
include any cocos2d-x header or link against cocos2d-x. The core MUST build as its own library
target and its tests MUST run without the engine. Given the same starting state, actions and
random seed, the core MUST produce the same results.

**Rationale**: A pure, deterministic core can be tested exhaustively, can drive automated
simulations (e.g., AI-vs-AI win-rate checks), and can later serve as the shared authority for
online multiplayer without a rewrite.

### II. Test-Driven Development (NON-NEGOTIABLE)

All production code MUST be written test-first: write a failing test, confirm it fails for the
expected reason, write the minimal code to pass, then refactor with the tests green
(Red-Green-Refactor). This applies to the rules core and to all non-rendering code outside it:
game flow and turn controllers, view-model/presenter logic, settings persistence, localization
lookup and input validation. Code that only draws, animates or plays sound through cocos2d-x
is exempt, but MUST stay a thin layer with no game decisions in it.

At minimum, the core tests MUST cover:
- fleet composition and the no-overlap/no-touch rule, including diagonals and board edges;
- full and mixed auto-placement, including the "remaining ships cannot fit" case;
- turn passing on a miss and the extra shot on a hit;
- the Help cross: edges, corners, cells already fired upon, no extra shot, and the win it can trigger;
- each difficulty's targeting rules, and that the computer never repeats a shot;
- ending the game immediately on the last hit.

Generators involving randomness MUST also have property-style or bulk tests, such as
validating thousands of auto-generated fleets. A bug fix MUST start with a test that
reproduces the bug.

**Rationale**: The game's quality comes down to exact rules; rule bugs are invisible until
a player hits them mid-game. Writing the tests first also forces the dependency seams that
Principle VI requires.

### III. Fair, Rule-Bound Computer Opponent

The computer MUST play under the same rules as the player and MUST decide using only the
information a human in its seat would have: its own shot history and results, and the public
board state. It MUST NOT read the positions of the player's ships that haven't been revealed.
Difficulty MUST come only from the defined targeting strategy for each level, never from
hidden advantages.

**Rationale**: A cheating AI ruins trust in the game and makes difficulty tuning meaningless.

### IV. Localization by Default

All user-facing text MUST come from translation resources; no display strings in code.
Every string MUST exist in English, Russian and Vietnamese before a feature counts as
complete, and changing the language MUST take effect without a restart. Layouts MUST handle
the longest translation, including player names up to the allowed length, without clipping.
Fonts MUST render Cyrillic and Vietnamese diacritics correctly.

**Rationale**: Three languages are a core requirement; retrofitting localization is costly
and error-prone.

### V. Simplicity and Offline-First Scope

The game MUST run fully offline for a single local player, with no accounts, servers or
network calls. Features outside the approved spec, including online multiplayer,
statistics and save/resume, MUST NOT be built until a spec approves them (YAGNI). New
dependencies beyond those in Technology Stack MUST be justified in the plan's Complexity
Tracking.

**Rationale**: A small, focused scope ships sooner and stays maintainable; the isolation in
Principle I already keeps the path to multiplayer open.

### VI. SOLID Design

All code MUST follow the SOLID principles, applied as these concrete rules:
- **Single Responsibility**: each class has one reason to change. Rules, AI, game flow,
  presentation, audio, localization and persistence MUST be separate classes.
- **Open/Closed**: variation points MUST be extended by adding implementations, not by editing
  `switch`/`if` chains. Each difficulty level MUST be its own implementation of a shared
  targeting-strategy interface; placement strategies follow the same pattern.
- **Liskov Substitution**: every implementation of an interface MUST honour its documented
  contract, so that any difficulty strategy, random source or storage backend can be swapped
  without changing callers or breaking their tests.
- **Interface Segregation**: interfaces MUST be small and role-specific (e.g., a read-only
  board view for the AI, separate from the mutating board API), so that no client depends on
  methods it does not use.
- **Dependency Inversion**: high-level logic MUST depend on abstractions and receive them
  through constructor injection. Randomness, time, storage, audio and localization MUST sit
  behind interfaces so that tests can substitute fakes. The core MUST NOT use singletons or
  mutable global state; cocos2d-x singletons (e.g., `Director`, `UserDefault`, `AudioEngine`)
  MAY be used only inside presentation-layer adapters.

**Rationale**: These rules keep the rules core testable in isolation (Principles I and II),
make the AI's information limits enforceable through types (Principle III), and let new
difficulties or a future network opponent be added without rewriting existing code.

## Technology Stack

- **Language**: C++. The whole project MUST compile under a single C++ standard supported by
  the chosen cocos2d-x version and its platform toolchains (C++14 or newer); the plan fixes the
  exact standard. Compiler warnings MUST be enabled at a high level and treated as errors for
  project code (not for third-party code).
- **Engine**: cocos2d-x (4.x) is the only rendering, input, scene, audio and platform layer.
  It MUST be used only in the presentation layer and in adapters that implement core-facing
  interfaces.
- **Build**: CMake. The rules core, the game application and the test executable(s) MUST be
  separate targets; the core and its tests MUST build without the engine.
- **Testing**: one C++ unit test framework (e.g., GoogleTest or Catch2), chosen in the plan
  and used project-wide. Tests MUST run from the command line through CMake/CTest.
- **Memory and resources**: project code MUST use RAII and standard smart pointers for ownership;
  raw `new`/`delete` is forbidden outside cocos2d-x's own `create()`/`Ref` reference-counting
  idiom in the presentation layer. Raw pointers MAY be used only as non-owning references.
- **Target platforms**: decided in the plan; every supported platform MUST build from the same
  CMake project.

## Player Experience Standards

- Every shot result (player or computer) MUST appear on screen within 0.5 seconds; the
  computer's shots MUST be paced so the player can follow each one.
- Input that breaks a rule, such as an illegal placement, a repeated shot, or a click during the
  opponent's turn, MUST be rejected without changing state and, where the player caused it,
  with a clear explanation.
- Actions that would lose progress, such as leaving a battle in progress, MUST ask for confirmation.
- Settings (sound, music, language, player name) MUST persist between sessions on the device.

## Development Workflow

- Work follows the Spec Kit flow: specify → clarify → plan → tasks → implement. A spec MUST
  have no open `[NEEDS CLARIFICATION]` markers before planning.
- Each plan MUST pass the Constitution Check against Principles I–VI and the Technology Stack
  before design, and again after it; any violation MUST be recorded and justified in
  Complexity Tracking.
- Tasks MUST be ordered so that each test task comes before the implementation task it covers.
- A change can be merged only when the project builds without warnings, all automated tests
  pass, and every new user-facing string has all three translations.
- Review MUST confirm that new code came with tests written first and that it follows the
  SOLID rules in Principle VI.

## Governance

This constitution overrides other project practices. Amendments are made with
`/speckit-constitution`, MUST include a Sync Impact Report, and MUST be versioned
semantically:
- MAJOR: removing or redefining a principle in a backward-incompatible way;
- MINOR: adding a principle or section, or materially expanding guidance;
- PATCH: clarifications and wording fixes.

Every plan and review MUST check compliance with this document. Runtime development
guidance belongs in the agent guidance file (e.g., `CLAUDE.md`), not here.

**Version**: 1.1.0 | **Ratified**: 2026-10-17 | **Last Amended**: 2026-10-17
