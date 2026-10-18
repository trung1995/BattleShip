#pragma once

#include <string>

#include "bs/core/Types.h"

namespace bs {
namespace app {

// Ports: what the application layer requires from the engine. The cocos2d-x game target
// implements them with thin adapters in src/game/adapters/.

class ISettingsStore {
public:
    virtual ~ISettingsStore() = default;
    virtual bool getBool(const std::string& key, bool fallback) const = 0;
    virtual std::string getString(const std::string& key, const std::string& fallback) const = 0;
    virtual void setBool(const std::string& key, bool value) = 0;
    virtual void setString(const std::string& key, const std::string& value) = 0;
    virtual void flush() = 0;
};

class IResourceReader {
public:
    virtual ~IResourceReader() = default;
    // Reads a UTF-8 text resource. Returns false if it is missing or unreadable.
    virtual bool readText(const std::string& path, std::string& out) const = 0;
};

enum class SoundId { Shot, Miss, Hit, Sunk, Help, Victory, Defeat, UiClick };
enum class MusicId { Menu, Battle };

class IAudioPlayer {
public:
    virtual ~IAudioPlayer() = default;
    virtual void playEffect(SoundId id) = 0;
    virtual void playMusic(MusicId id, bool loop) = 0;
    virtual void stopMusic() = 0;
};

class INavigator {
public:
    virtual ~INavigator() = default;
    virtual void showMenu() = 0;
    virtual void showSettings() = 0;
    virtual void showPlacement(bs::core::Difficulty difficulty) = 0;
    virtual void showBattle() = 0;
    virtual void showNewGame() = 0; // the menu with the difficulty picker open
};

} // namespace app
} // namespace bs
