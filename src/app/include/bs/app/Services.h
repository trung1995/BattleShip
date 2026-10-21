#pragma once

#include <functional>
#include <map>
#include <string>

#include "bs/core/Types.h"

namespace bs {
namespace app {

enum class Language { En, Ru, Vi, Uk };
enum class Screen { Menu, Settings, Placement, Battle };

struct NameResult {
    std::string stored; // value now stored; "" means the default name is shown
    bool trimmed = false;
    bool truncated = false;
    bool reset = false;
};

using SubscriptionId = int;

enum class TextKey : int; // defined in TextKeys.h

// Service interfaces. Controllers, other services and scene labels depend on these, never on
// the concrete services (Principle VI, Dependency Inversion).

class ISettings {
public:
    virtual ~ISettings() = default;
    virtual bool soundEnabled() const = 0;
    virtual void setSoundEnabled(bool enabled) = 0;
    virtual bool musicEnabled() const = 0;
    virtual void setMusicEnabled(bool enabled) = 0;
    virtual Language language() const = 0;
    virtual void setLanguage(Language language) = 0;
    virtual std::string storedPlayerName() const = 0;
    virtual NameResult setPlayerName(const std::string& utf8) = 0;
};

class ILocalizer {
public:
    virtual ~ILocalizer() = default;
    virtual Language language() const = 0;
    virtual void setLanguage(Language language) = 0;
    virtual std::string text(TextKey key) const = 0;
    virtual std::string format(TextKey key,
                               const std::map<std::string, std::string>& args) const = 0;
    virtual SubscriptionId subscribe(std::function<void()> onChanged) = 0;
    virtual void unsubscribe(SubscriptionId id) = 0;
    virtual std::string displayName(bs::core::Side side, const ISettings& settings) const = 0;
};

class IGameAudio {
public:
    virtual ~IGameAudio() = default;
    virtual void onShot(bs::core::ShotResult result) = 0;
    virtual void onHelp() = 0;
    virtual void onGameOver(bool playerWon) = 0;
    virtual void onClick() = 0;
    virtual void enterScreen(Screen screen) = 0;
    virtual void onSettingsChanged() = 0;
};

} // namespace app
} // namespace bs
