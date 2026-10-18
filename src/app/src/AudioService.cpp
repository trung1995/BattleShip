#include "bs/app/AudioService.h"

namespace bs {
namespace app {

namespace {
MusicId musicFor(Screen screen) {
    return screen == Screen::Menu || screen == Screen::Settings ? MusicId::Menu : MusicId::Battle;
}
} // namespace

AudioService::AudioService(IAudioPlayer& player, const ISettings& settings)
    : player_(player), settings_(settings) {}

void AudioService::effect(SoundId id) {
    if (settings_.soundEnabled()) {
        player_.playEffect(id);
    }
}

void AudioService::onShot(bs::core::ShotResult result) {
    switch (result) {
    case bs::core::ShotResult::Miss:
        effect(SoundId::Miss);
        break;
    case bs::core::ShotResult::Hit:
        effect(SoundId::Hit);
        break;
    case bs::core::ShotResult::Sunk:
        effect(SoundId::Sunk);
        break;
    }
}

void AudioService::onHelp() { effect(SoundId::Help); }

void AudioService::onGameOver(bool playerWon) { effect(playerWon ? SoundId::Victory : SoundId::Defeat); }

void AudioService::onClick() { effect(SoundId::UiClick); }

void AudioService::startMusicFor(Screen screen) {
    const MusicId wanted = musicFor(screen);
    if (!settings_.musicEnabled() || (musicPlaying_ && playing_ == wanted)) {
        return;
    }
    player_.playMusic(wanted, true);
    musicPlaying_ = true;
    playing_ = wanted;
}

void AudioService::enterScreen(Screen screen) {
    hasScreen_ = true;
    screen_ = screen;
    startMusicFor(screen);
}

void AudioService::onSettingsChanged() {
    if (!settings_.musicEnabled()) {
        if (musicPlaying_) {
            player_.stopMusic();
            musicPlaying_ = false;
        }
    } else if (hasScreen_) {
        startMusicFor(screen_);
    }
}

} // namespace app
} // namespace bs
