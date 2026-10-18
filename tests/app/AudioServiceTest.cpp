#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "InMemorySettingsStore.h"
#include "MockAudioPlayer.h"
#include "bs/app/AudioService.h"
#include "bs/app/SettingsService.h"

using namespace bs::app;
using bs::core::ShotResult;
using ::testing::_;
using ::testing::StrictMock;

class AudioServiceTest : public ::testing::Test {
protected:
    InMemorySettingsStore store;
    SettingsService settings{store};
    StrictMock<MockAudioPlayer> player;
    AudioService audio{player, settings};
};

TEST_F(AudioServiceTest, GameEventsPlayTheirEffects) {
    EXPECT_CALL(player, playEffect(SoundId::Miss));
    EXPECT_CALL(player, playEffect(SoundId::Hit));
    EXPECT_CALL(player, playEffect(SoundId::Sunk));
    EXPECT_CALL(player, playEffect(SoundId::Help));
    EXPECT_CALL(player, playEffect(SoundId::Victory));
    EXPECT_CALL(player, playEffect(SoundId::Defeat));
    EXPECT_CALL(player, playEffect(SoundId::UiClick));
    audio.onShot(ShotResult::Miss);
    audio.onShot(ShotResult::Hit);
    audio.onShot(ShotResult::Sunk);
    audio.onHelp();
    audio.onGameOver(true);
    audio.onGameOver(false);
    audio.onClick();
}

TEST_F(AudioServiceTest, NoEffectsWhenSoundIsOff) {
    settings.setSoundEnabled(false);
    audio.onShot(ShotResult::Hit);
    audio.onHelp();
    audio.onGameOver(true);
    audio.onClick(); // StrictMock fails on any call
}

TEST_F(AudioServiceTest, EnteringAScreenPlaysItsMusicLooped) {
    EXPECT_CALL(player, playMusic(MusicId::Menu, true));
    EXPECT_CALL(player, playMusic(MusicId::Battle, true));
    audio.enterScreen(Screen::Menu);
    audio.enterScreen(Screen::Settings); // same track keeps playing
    audio.enterScreen(Screen::Placement);
    audio.enterScreen(Screen::Battle); // same track keeps playing
}

TEST_F(AudioServiceTest, NoMusicWhenMusicIsOff) {
    settings.setMusicEnabled(false);
    audio.enterScreen(Screen::Menu);
}

TEST_F(AudioServiceTest, TogglesAreIndependent) {
    settings.setMusicEnabled(false);
    EXPECT_CALL(player, playEffect(SoundId::Hit));
    audio.onShot(ShotResult::Hit);

    settings.setMusicEnabled(true);
    settings.setSoundEnabled(false);
    EXPECT_CALL(player, playMusic(MusicId::Battle, true));
    audio.enterScreen(Screen::Battle);
}

TEST_F(AudioServiceTest, SettingsChangeStopsOrRestartsMusicImmediately) {
    EXPECT_CALL(player, playMusic(MusicId::Menu, true)).Times(2);
    EXPECT_CALL(player, stopMusic());
    audio.enterScreen(Screen::Menu);

    settings.setMusicEnabled(false);
    audio.onSettingsChanged();

    settings.setMusicEnabled(true);
    audio.onSettingsChanged();
    audio.onSettingsChanged(); // already playing: no second start
}
