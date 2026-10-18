#include "GameServices.h"

#include <random>

#include "adapters/CocosAudioPlayer.h"
#include "adapters/FileUtilsResourceReader.h"
#include "adapters/SceneNavigator.h"
#include "adapters/UserDefaultSettingsStore.h"

GameServices::GameServices()
    : store_(std::make_unique<UserDefaultSettingsStore>()),
      reader_(std::make_unique<FileUtilsResourceReader>()), player_(std::make_unique<CocosAudioPlayer>()),
      random_(std::make_unique<bs::core::Mt19937Random>(std::random_device{}())) {
    settings_ = std::make_unique<bs::app::SettingsService>(*store_);
    localizer_ = std::make_unique<bs::app::Localizer>(*reader_, settings_->language());
    audio_ = std::make_unique<bs::app::AudioService>(*player_, *settings_);
    setup_ = std::make_unique<bs::app::GameSetup>(placer_, *random_);
    navigator_ = std::make_unique<SceneNavigator>(*this);
}

GameServices::~GameServices() = default;
