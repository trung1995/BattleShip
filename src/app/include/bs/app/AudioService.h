#pragma once

#include "bs/app/Ports.h"
#include "bs/app/Services.h"

namespace bs {
namespace app {

// Maps game events to sounds and applies the sound-effects and music toggles independently.
class AudioService final : public IGameAudio {
public:
    AudioService(IAudioPlayer& player, const ISettings& settings);

    void onShot(bs::core::ShotResult result) override;
    void onHelp() override;
    void onGameOver(bool playerWon) override;
    void onClick() override;
    void enterScreen(Screen screen) override;
    void onSettingsChanged() override;

private:
    void effect(SoundId id);
    void startMusicFor(Screen screen);

    IAudioPlayer& player_;
    const ISettings& settings_;
    bool hasScreen_ = false;
    Screen screen_ = Screen::Menu;
    bool musicPlaying_ = false;
    MusicId playing_ = MusicId::Menu;
};

} // namespace app
} // namespace bs
