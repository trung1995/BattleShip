#pragma once

#include "bs/app/Ports.h"

// IAudioPlayer over cocos2d::AudioEngine. Assets are MP3 (research R13).
class CocosAudioPlayer final : public bs::app::IAudioPlayer {
public:
    ~CocosAudioPlayer() override;

    void playEffect(bs::app::SoundId id) override;
    void playMusic(bs::app::MusicId id, bool loop) override;
    void stopMusic() override;

private:
    int musicAudioId_ = -1;
};
