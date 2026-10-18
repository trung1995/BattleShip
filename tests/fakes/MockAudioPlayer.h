#pragma once

#include <gmock/gmock.h>

#include "bs/app/Ports.h"

class MockAudioPlayer : public bs::app::IAudioPlayer {
public:
    MOCK_METHOD(void, playEffect, (bs::app::SoundId id), (override));
    MOCK_METHOD(void, playMusic, (bs::app::MusicId id, bool loop), (override));
    MOCK_METHOD(void, stopMusic, (), (override));
};
