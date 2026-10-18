#include "adapters/CocosAudioPlayer.h"

#include "audio/include/AudioEngine.h"

namespace {
const char* fileFor(bs::app::SoundId id) {
    using bs::app::SoundId;
    switch (id) {
    case SoundId::Shot: return "audio/shot.mp3";
    case SoundId::Miss: return "audio/miss.mp3";
    case SoundId::Hit: return "audio/hit.mp3";
    case SoundId::Sunk: return "audio/sunk.mp3";
    case SoundId::Help: return "audio/help.mp3";
    case SoundId::Victory: return "audio/victory.mp3";
    case SoundId::Defeat: return "audio/defeat.mp3";
    case SoundId::UiClick: return "audio/click.mp3";
    }
    return "audio/click.mp3";
}

const char* fileFor(bs::app::MusicId id) {
    return id == bs::app::MusicId::Menu ? "audio/menu.mp3" : "audio/battle.mp3";
}
} // namespace

CocosAudioPlayer::~CocosAudioPlayer() { cocos2d::AudioEngine::end(); }

void CocosAudioPlayer::playEffect(bs::app::SoundId id) { cocos2d::AudioEngine::play2d(fileFor(id)); }

void CocosAudioPlayer::playMusic(bs::app::MusicId id, bool loop) {
    stopMusic();
    musicAudioId_ = cocos2d::AudioEngine::play2d(fileFor(id), loop, 0.5f);
}

void CocosAudioPlayer::stopMusic() {
    if (musicAudioId_ != cocos2d::AudioEngine::INVALID_AUDIO_ID) {
        cocos2d::AudioEngine::stop(musicAudioId_);
        musicAudioId_ = cocos2d::AudioEngine::INVALID_AUDIO_ID;
    }
}
