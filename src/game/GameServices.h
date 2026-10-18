#pragma once

#include <memory>

#include "bs/app/AudioService.h"
#include "bs/app/GameSetup.h"
#include "bs/app/Localizer.h"
#include "bs/app/Ports.h"
#include "bs/app/SettingsService.h"
#include "bs/core/AutoPlacer.h"
#include "bs/core/GameSession.h"
#include "bs/core/Random.h"

// Composition root: owns the adapters and services. Scenes reach them only through the
// interface accessors (Principle VI).
class GameServices {
public:
    GameServices();
    ~GameServices();

    bs::app::ISettings& settings() { return *settings_; }
    bs::app::ILocalizer& localizer() { return *localizer_; }
    bs::app::IGameAudio& audio() { return *audio_; }
    bs::app::INavigator& navigator() { return *navigator_; }
    bs::core::IRandom& random() { return *random_; }
    const bs::core::IFleetPlacer& placer() const { return placer_; }
    bs::app::GameSetup& setup() { return *setup_; }

    // The game in progress, created when the placement screen opens. Scenes hold their own
    // reference, so a scene that is still fading out never sees its session destroyed.
    std::shared_ptr<bs::core::GameSession> currentSession;
    bs::core::Difficulty currentDifficulty = bs::core::Difficulty::Easy;

private:
    std::unique_ptr<bs::app::ISettingsStore> store_;
    std::unique_ptr<bs::app::IResourceReader> reader_;
    std::unique_ptr<bs::app::IAudioPlayer> player_;
    std::unique_ptr<bs::core::IRandom> random_;
    bs::core::BacktrackingPlacer placer_;
    std::unique_ptr<bs::app::SettingsService> settings_;
    std::unique_ptr<bs::app::Localizer> localizer_;
    std::unique_ptr<bs::app::AudioService> audio_;
    std::unique_ptr<bs::app::GameSetup> setup_;
    std::unique_ptr<bs::app::INavigator> navigator_;
};
