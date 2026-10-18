#pragma once

#include <gmock/gmock.h>

#include "bs/app/Services.h"

class MockGameAudio : public bs::app::IGameAudio {
public:
    MOCK_METHOD(void, onShot, (bs::core::ShotResult result), (override));
    MOCK_METHOD(void, onHelp, (), (override));
    MOCK_METHOD(void, onGameOver, (bool playerWon), (override));
    MOCK_METHOD(void, onClick, (), (override));
    MOCK_METHOD(void, enterScreen, (bs::app::Screen screen), (override));
    MOCK_METHOD(void, onSettingsChanged, (), (override));
};
