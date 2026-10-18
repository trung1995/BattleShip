#include <gtest/gtest.h>

#include "InMemorySettingsStore.h"
#include "bs/app/SettingsService.h"
#include "bs/app/Utf8.h"

using namespace bs::app;

TEST(SettingsService, Defaults) {
    InMemorySettingsStore store;
    SettingsService settings(store);
    EXPECT_TRUE(settings.soundEnabled());
    EXPECT_TRUE(settings.musicEnabled());
    EXPECT_EQ(settings.language(), Language::En);
    EXPECT_EQ(settings.storedPlayerName(), "");
}

TEST(SettingsService, SettersPersistImmediatelyUnderTheContractKeys) {
    InMemorySettingsStore store;
    SettingsService settings(store);

    settings.setSoundEnabled(false);
    EXPECT_EQ(store.bools.at("settings.sound"), false);
    settings.setMusicEnabled(false);
    EXPECT_EQ(store.bools.at("settings.music"), false);
    settings.setLanguage(Language::Ru);
    EXPECT_EQ(store.strings.at("settings.language"), "ru");
    settings.setLanguage(Language::Vi);
    EXPECT_EQ(store.strings.at("settings.language"), "vi");
    settings.setPlayerName("Ann");
    EXPECT_EQ(store.strings.at("settings.playerName"), "Ann");
    EXPECT_EQ(store.flushCount, 5);
}

TEST(SettingsService, UnknownLanguageFallsBackToEnglish) {
    InMemorySettingsStore store;
    store.strings["settings.language"] = "fr";
    EXPECT_EQ(SettingsService(store).language(), Language::En);
    store.strings["settings.language"] = "";
    EXPECT_EQ(SettingsService(store).language(), Language::En);
}

TEST(SettingsService, PlayerNameIsTrimmed) {
    InMemorySettingsStore store;
    SettingsService settings(store);
    const NameResult result = settings.setPlayerName("  Ann  ");
    EXPECT_EQ(result.stored, "Ann");
    EXPECT_TRUE(result.trimmed);
    EXPECT_FALSE(result.truncated);
    EXPECT_FALSE(result.reset);
    EXPECT_EQ(settings.storedPlayerName(), "Ann");
}

TEST(SettingsService, EmptyOrWhitespaceNameResetsToDefault) {
    InMemorySettingsStore store;
    SettingsService settings(store);
    settings.setPlayerName("Ann");
    const NameResult result = settings.setPlayerName(" \t ");
    EXPECT_TRUE(result.reset);
    EXPECT_EQ(result.stored, "");
    EXPECT_EQ(settings.storedPlayerName(), "");
    EXPECT_TRUE(settings.setPlayerName("").reset);
}

TEST(SettingsService, NameLongerThanTwentyCodePointsIsTruncated) {
    InMemorySettingsStore store;
    SettingsService settings(store);
    const NameResult result = settings.setPlayerName("ЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖ");
    EXPECT_TRUE(result.truncated);
    EXPECT_EQ(codePointCount(result.stored), 20u);
    EXPECT_EQ(settings.storedPlayerName(), result.stored);
}

TEST(SettingsService, TwentyCodePointVietnameseNameIsKept) {
    InMemorySettingsStore store;
    SettingsService settings(store);
    const std::string name = "Nguyễn Thị Ẩnh Hồngg"; // 20 code points
    ASSERT_EQ(codePointCount(name), 20u);
    const NameResult result = settings.setPlayerName(name);
    EXPECT_FALSE(result.truncated);
    EXPECT_EQ(result.stored, name);
}

TEST(SettingsService, ValuesSurviveANewServiceOnTheSameStore) {
    InMemorySettingsStore store;
    {
        SettingsService settings(store);
        settings.setSoundEnabled(false);
        settings.setLanguage(Language::Vi);
        settings.setPlayerName("Lan");
    }
    SettingsService reloaded(store);
    EXPECT_FALSE(reloaded.soundEnabled());
    EXPECT_TRUE(reloaded.musicEnabled());
    EXPECT_EQ(reloaded.language(), Language::Vi);
    EXPECT_EQ(reloaded.storedPlayerName(), "Lan");
}
