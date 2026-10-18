# Battleship

A single-player Battleship game for desktop (macOS and Windows), written in C++14 with
cocos2d-x 4.0. Play against the computer on 10 × 10 boards at three difficulty levels, arrange
your fleet by hand or automatically, use the once-per-game cross-shaped Help shot, and switch
between English, Russian and Vietnamese at any time.

## Build, test and run

See [specs/001-battleship-game/quickstart.md](specs/001-battleship-game/quickstart.md) for the
prerequisites and commands. In short, the rules and application logic build and test without
the engine:

```bash
cmake -S . -B build/tests -DBATTLESHIP_BUILD_GAME=OFF
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

## Layout

The code has three layers, and dependencies only point downwards (game → app → core):

| Layer | Folder | Depends on |
|---|---|---|
| Rules core: fleet, placement, turns, Help, computer opponents | `src/core` | C++ standard library only |
| Application logic: controllers, settings, translations, audio rules | `src/app` | core |
| Presentation: cocos2d-x scenes, drawing, engine adapters | `src/game` | app, cocos2d-x |

Tests live in `tests/` (`ctest -L sim` runs the computer-vs-computer simulations). Text for
all three languages is in `Resources/i18n/`. The design documents are in
[specs/001-battleship-game/](specs/001-battleship-game/), and the project's rules are in
[.specify/memory/constitution.md](.specify/memory/constitution.md).

## Third-party code and assets

- [cocos2d-x 4.0](https://github.com/cocos2d/cocos2d-x) (MIT), as a git submodule. Fixes for
  current toolchains are in `third_party/patches/` and are applied automatically.
- [GoogleTest 1.14.0](https://github.com/google/googletest) (BSD-3-Clause), fetched at build
  time for the tests.
- [Alegreya and Alegreya SC](https://github.com/huertatipografica/Alegreya) (SIL Open Font
  License 1.1); see `Resources/fonts/OFL.txt`.
- Sound effects and music are generated placeholders (CC0); see `Resources/audio/LICENSES.md`.
