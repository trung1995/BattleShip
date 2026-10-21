#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "FakeLocalizer.h"
#include "InMemorySettingsStore.h"
#include "MockGameAudio.h"
#include "bs/app/SettingsController.h"
#include "bs/app/SettingsService.h"
#include "bs/app/Utf8.h"

using namespace bs::app;
using ::testing::NiceMock;

class SettingsControllerTest : public ::testing::Test {
protected:
    InMemorySettingsStore store;
    SettingsService settings{store};
    FakeLocalizer localizer;
    NiceMock<MockGameAudio> audio;
    SettingsController controller{settings, localizer, audio};
};

TEST_F(SettingsControllerTest, TogglesFlipPersistAndNotifyAudio) {
    EXPECT_CALL(audio, onSettingsChanged()).Times(2);
    controller.toggleSound();
    EXPECT_FALSE(settings.soundEnabled());
    EXPECT_FALSE(store.bools.at("settings.sound"));
    controller.toggleMusic();
    EXPECT_FALSE(settings.musicEnabled());
    EXPECT_FALSE(store.bools.at("settings.music"));
}

TEST_F(SettingsControllerTest, SelectingALanguageStoresItAndSwitchesTheLocalizer) {
    controller.selectLanguage(Language::Ru);
    EXPECT_EQ(settings.language(), Language::Ru);
    ASSERT_EQ(localizer.languageCalls.size(), 1u);
    EXPECT_EQ(localizer.languageCalls[0], Language::Ru);
}

TEST_F(SettingsControllerTest, LiveNameInputIsCappedAtTwentyCodePoints) {
    const std::string typed = "ЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖ";
    EXPECT_EQ(codePointCount(controller.filterNameInput(typed)), 20u);
    EXPECT_EQ(controller.filterNameInput("Ann"), "Ann");
}

TEST_F(SettingsControllerTest, CommittingABlankNameShowsTheDefault) {
    settings.setPlayerName("Ann");
    const NameResult result = controller.commitName("   ");
    EXPECT_TRUE(result.reset);
    EXPECT_EQ(settings.storedPlayerName(), "");
    EXPECT_EQ(localizer.displayName(bs::core::Side::Player, settings), "name.player_default");
}

TEST_F(SettingsControllerTest, CommittingALongNameStoresTheFirstTwentyCodePoints) {
    const NameResult result = controller.commitName("ЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖ");
    EXPECT_EQ(codePointCount(settings.storedPlayerName()), 20u);
    EXPECT_TRUE(result.truncated);
}

TEST_F(SettingsControllerTest, OffersTheFourLanguagesWithTheirNames) {
    const auto languages = controller.languages();
    ASSERT_EQ(languages.size(), 4u);
    EXPECT_EQ(languages[0].first, Language::En);
    EXPECT_EQ(languages[0].second, TextKey::LangEn);
    EXPECT_EQ(languages[1].first, Language::Ru);
    EXPECT_EQ(languages[1].second, TextKey::LangRu);
    EXPECT_EQ(languages[2].first, Language::Vi);
    EXPECT_EQ(languages[2].second, TextKey::LangVi);
    EXPECT_EQ(languages[3].first, Language::Uk);
    EXPECT_EQ(languages[3].second, TextKey::LangUk);
}
