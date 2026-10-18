# Contract: Localization resources and settings storage

## Language files

- **Location**: `Resources/i18n/en.lang`, `Resources/i18n/ru.lang` and `Resources/i18n/vi.lang`.
- **Encoding**: UTF-8 without BOM. Line endings are LF; CRLF is tolerated.

### Grammar
```text
file    := { line }
line    := blank | comment | entry
comment := ws* "#" any*
entry   := ws* key ws* "=" ws* value ws*      ; value is trimmed; may be empty only if intentional
key     := [a-z0-9_]+ { "." [a-z0-9_]+ }      ; e.g. battle.turn.player
value   := any UTF-8 text; "\n" inside the value is a newline; "{" ident "}" is a placeholder
```

### Rules
- Duplicate keys are a load error, reported in debug builds and covered by a test.
- Every key in `TextKeys.h` MUST exist in all three files, and no file may have extra keys.
  This is enforced by `LocalizationCompletenessTest` (SC-006, Principle IV).
- Placeholders MUST match across languages for the same key, also enforced by the test.
- Language display names are not translated: `lang.en = English`, `lang.ru = Русский` and
  `lang.vi = Tiếng Việt` are the same in every file.

### Required keys (initial catalogue)

| Key | en | ru | vi |
|---|---|---|---|
| `name.computer` | Computer | Компьютер | Máy tính |
| `name.player_default` | Player | Игрок | Người chơi |
| `battle.winner` | {name} wins! | {name} побеждает! | {name} chiến thắng! |

The full catalogue covers:
- the menu, difficulty pick, settings and placement screens, including the placement errors
  `placement.error.out_of_bounds`, `placement.error.overlaps`, `placement.error.touches`,
  `placement.error.cannot_fit` and `placement.error.fleet_incomplete`;
- the battle screen: turn banners, Help states, rejection messages, the exit dialog and
  game over.

It is written during implementation with the three translations together (constitution
merge rule).

## Settings storage keys (`ISettingsStore`)

| Key | Type | Default |
|---|---|---|
| `settings.sound` | bool | true |
| `settings.music` | bool | true |
| `settings.language` | string `en`/`ru`/`vi` | `en` |
| `settings.playerName` | string (UTF-8, ≤ 20 code points, trimmed) | `""`, which displays `name.player_default` |

Unknown or corrupt values fall back to the defaults without crashing.
