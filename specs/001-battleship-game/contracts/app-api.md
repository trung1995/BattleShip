# Contract: `battleship_app` (application logic and engine ports)

**Scope**: the non-rendering layer (namespace `bs::app`) between the core and the cocos2d-x
scenes. It depends on `battleship_core` and the standard library only. It has two kinds of
interface:

- **Ports**, which this layer *requires*. The cocos2d-x game target implements them with thin
  adapters.
- **Controllers and services**, which this layer *provides*. Scenes call them and render what
  they return.

Everything here is unit-tested with fakes (Principle II). Headers live under
`src/app/include/bs/app/`.

## Ports (implemented by engine adapters in `src/game/adapters/`)

```cpp
class ISettingsStore {                     // adapter: UserDefaultSettingsStore (cocos2d::UserDefault)
public:
    virtual ~ISettingsStore() = default;
    virtual bool        getBool(const std::string& key, bool fallback) const = 0;
    virtual std::string getString(const std::string& key, const std::string& fallback) const = 0;
    virtual void setBool(const std::string& key, bool) = 0;
    virtual void setString(const std::string& key, const std::string&) = 0;
    virtual void flush() = 0;
};

class IResourceReader {                    // adapter: FileUtilsResourceReader (cocos2d::FileUtils)
public:
    virtual ~IResourceReader() = default;
    virtual bool readText(const std::string& path, std::string& out) const = 0;  // UTF-8
};

enum class SoundId { Shot, Miss, Hit, Sunk, Help, Victory, Defeat, UiClick };
enum class MusicId { Menu, Battle };
class IAudioPlayer {                       // adapter: CocosAudioPlayer (cocos2d::experimental::AudioEngine)
public:
    virtual ~IAudioPlayer() = default;
    virtual void playEffect(SoundId) = 0;
    virtual void playMusic(MusicId, bool loop) = 0;
    virtual void stopMusic() = 0;
};

class INavigator {                         // adapter: SceneNavigator (cocos2d::Director)
public:
    virtual ~INavigator() = default;
    virtual void showMenu() = 0;
    virtual void showSettings() = 0;
    virtual void showPlacement(bs::core::Difficulty) = 0;
    virtual void showBattle() = 0;
    virtual void showNewGame() = 0;   // the menu with the difficulty picker open (Game Over → New Game)
};
```

## Service interfaces (`Services.h`)

Controllers, other services and scene labels depend on these interfaces, never on the concrete
services (Principle VI, Dependency Inversion). Tests substitute `FakeLocalizer` and
`MockGameAudio`.

```cpp
enum class Language { En, Ru, Vi };
enum class Screen { Menu, Settings, Placement, Battle };
struct NameResult { std::string stored; bool trimmed; bool truncated; bool reset; };
using SubscriptionId = int;
enum class TextKey : int;                 // opaque declaration; defined in TextKeys.h

class ISettings {                         // implemented by SettingsService
public:
    virtual ~ISettings() = default;
    virtual bool soundEnabled() const = 0;      virtual void setSoundEnabled(bool) = 0;
    virtual bool musicEnabled() const = 0;      virtual void setMusicEnabled(bool) = 0;
    virtual Language language() const = 0;      virtual void setLanguage(Language) = 0;
    virtual std::string storedPlayerName() const = 0;
    virtual NameResult setPlayerName(const std::string& utf8) = 0;
};

class ILocalizer {                        // implemented by Localizer
public:
    virtual ~ILocalizer() = default;
    virtual Language language() const = 0;
    virtual void setLanguage(Language) = 0;
    virtual std::string text(TextKey) const = 0;
    virtual std::string format(TextKey, const std::map<std::string, std::string>& args) const = 0;
    virtual SubscriptionId subscribe(std::function<void()> onChanged) = 0;
    virtual void unsubscribe(SubscriptionId) = 0;
    virtual std::string displayName(bs::core::Side, const ISettings&) const = 0;
};

class IGameAudio {                        // implemented by AudioService
public:
    virtual ~IGameAudio() = default;
    virtual void onShot(bs::core::ShotResult) = 0;
    virtual void onHelp() = 0;
    virtual void onGameOver(bool playerWon) = 0;
    virtual void onClick() = 0;
    virtual void enterScreen(Screen) = 0;
    virtual void onSettingsChanged() = 0;
};
```

`bs::core::Side` and `bs::core::ShotResult` come from `src/core/include/bs/core/Types.h`.

## Services

### `SettingsService` (implements `ISettings`)
```cpp
SettingsService(ISettingsStore&);
bool soundEnabled() const;  void setSoundEnabled(bool);
bool musicEnabled() const;  void setMusicEnabled(bool);
Language language() const;  void setLanguage(Language);         // Language { En, Ru, Vi }
std::string storedPlayerName() const;                           // "" if never set or reset
NameResult  setPlayerName(const std::string& utf8);             // trims; empty → reset; >20 code points → truncated
```

- Every setter persists immediately (FR-043).
- `NameResult` reports the stored value and whether it was trimmed, truncated or reset.
- The input field itself also caps entry at 20 code points (Edge Cases). Truncation in the
  service is a safety net.

### `Localizer` (implements `ILocalizer`)
```cpp
Localizer(IResourceReader&, Language initial);
void setLanguage(Language);                                     // reloads table, notifies subscribers
std::string text(TextKey) const;                                // falls back to the key name if missing (never crash)
std::string format(TextKey, const std::map<std::string, std::string>& args) const;   // {name}
SubscriptionId subscribe(std::function<void()> onChanged);  void unsubscribe(SubscriptionId);
std::string displayName(bs::core::Side, const ISettings&) const;  // player name or name.player_default; name.computer
```

- A language change is applied synchronously, and every subscriber is notified before
  `setLanguage` returns (FR-040).

### `AudioService` (implements `IGameAudio`)
```cpp
AudioService(IAudioPlayer&, const ISettings&);
void onShot(bs::core::ShotResult);  void onHelp();  void onGameOver(bool playerWon);  void onClick();
void enterScreen(Screen);                                       // picks music; respects toggle
void onSettingsChanged();                                       // stops or starts music immediately
```

- Effects are suppressed when sound is off, and music is suppressed when music is off. The two
  toggles are independent (US5 scenario 2).

### `Utf8`
`std::size_t codePointCount(const std::string&)`, `std::string truncateCodePoints(const
std::string&, std::size_t)`, and `std::string trim(const std::string&)` (Unicode whitespace).

### `GameSetup`
```cpp
GameSetup(const bs::core::IFleetPlacer&, bs::core::IRandom&);
std::unique_ptr<bs::core::GameSession> newSession(bs::core::Difficulty);   // computer fleet auto-placed here
```

- Called when the placement screen opens. The returned session is in phase `Placement` with an
  empty player fleet and a complete legal computer fleet (data-model §4).
- `PlacementController` is built on `session.playerFleet()`, so the player's fleet never needs
  to be copied into the session.

## Controllers

### `SettingsController`
```cpp
SettingsController(ISettings&, ILocalizer&, IGameAudio&);
void toggleSound();  void toggleMusic();                 // persist + IGameAudio::onSettingsChanged()
void selectLanguage(Language);                           // ISettings::setLanguage + ILocalizer::setLanguage
std::string filterNameInput(const std::string& utf8) const;   // caps live input at 20 code points
NameResult commitName(const std::string& utf8);
std::vector<std::pair<Language, TextKey>> languages() const;  // En/lang.en, Ru/lang.ru, Vi/lang.vi
```

### `PlacementController`
```cpp
PlacementController(bs::core::Fleet& fleet, const bs::core::IFleetPlacer&, bs::core::IRandom&);   // fleet = session.playerFleet()
void beginDragFromDock(bs::core::ShipId);  void beginDragPlaced(bs::core::ShipId);
Preview hover(bs::core::Coord cell);         // cells + legal flag + PlacementError reason
void rotate();                               // during a drag: flips the preview; otherwise rotates the selected ship in place if legal
DropResult drop(bs::core::Coord cell);       // Placed | Rejected(reason) | ReturnedToDock
void dropOnDock();                           // removes a placed ship (US2 scenario 7)
void select(bs::core::ShipId);               // selects a placed ship for rotate() outside a drag
void cancelDrag();
AutoResult autoPlaceRemaining();             // Ok | CannotFit (layout unchanged, FR-012)
void autoPlaceAll();
bool canStartBattle() const;                 // fleet complete (FR-014)
std::vector<bs::core::ShipId> dockShips() const;
```

### `BattleController`
```cpp
BattleController(bs::core::GameSession&, std::unique_ptr<bs::core::ITargetingStrategy>,
                 bs::core::IRandom&, IGameAudio&, double computerDelaySeconds = 0.8);
Mode mode() const;                           // AwaitingPlayer | PlayerHelpTargeting | ComputerActing | ConfirmExit | GameOver
ActionFeedback onPlayerFire(bs::core::Coord);    // rejected when mode != AwaitingPlayer (Edge Cases)
void armHelp();  void disarmHelp();              // enters or leaves PlayerHelpTargeting; no-op if no uses left.
                                                 // Allowed in AwaitingPlayer, including on an extra shot after a hit (FR-027)
ActionFeedback onPlayerHelp(bs::core::Coord);    // in PlayerHelpTargeting only
std::vector<bs::core::CellChange> tick(double dtSeconds);   // performs at most one computer action per delay
void requestExit();  void cancelExit();  bool confirmExit();     // FR-044; confirmExit() returns true to navigate
TurnIndicator turnIndicator() const;             // whose turn, for FR-024
bool playerHelpAvailable() const;                // helpUsesLeft(Player) > 0
int  playerHelpUsesLeft() const;                 // FR-032: 3, 2, 1 or 0 on the Help button
bool showComputerHelpCounter() const;            // true only on Hard (FR-032)
int  computerHelpUsesLeft() const;               // FR-032: shown next to the computer's name on Hard
```

- `ActionFeedback` carries the `ActionStatus`, a `TextKey` explaining any rejection the player
  caused, and the `CellChange` list to render in the same frame (SC-009). For Help, from either
  side, the list holds the whole cross plus any sunk-ship marking and is rendered as one batch
  (FR-048); `tick` returns a computer Help the same way.
- The strategy is called with `session.helpUsesLeft(Computer)`, never a bool.
- During `ConfirmExit`, `tick` performs nothing and returns no changes, so cancelling resumes
  the exact state (US6 scenario 2).
- Exit requests from the placement and game-over screens do not use this controller and
  navigate without confirmation (US6 scenario 4).
