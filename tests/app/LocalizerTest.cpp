#include <gtest/gtest.h>

#include "FakeResourceReader.h"
#include "InMemorySettingsStore.h"
#include "bs/app/Localizer.h"
#include "bs/app/SettingsService.h"
#include "bs/app/TextKeys.h"

using namespace bs::app;
using bs::core::Side;

namespace {
FakeResourceReader makeReader() {
    FakeResourceReader reader;
    reader.files["i18n/en.lang"] = "name.computer = Computer\nname.player_default = Player\n"
                                   "menu.quit = Quit\napp.title = {name} wins!\n";
    reader.files["i18n/ru.lang"] = "name.computer = Компьютер\nname.player_default = Игрок\n"
                                   "menu.quit = Выход\n";
    reader.files["i18n/vi.lang"] = "name.computer = Máy tính\nname.player_default = Người chơi\n";
    return reader;
}
} // namespace

TEST(Localizer, LoadsTheInitialLanguage) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::En);
    EXPECT_EQ(localizer.language(), Language::En);
    EXPECT_EQ(localizer.text(TextKey::MenuQuit), "Quit");
}

TEST(Localizer, MissingKeyReturnsTheKeyName) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::Vi);
    EXPECT_EQ(localizer.text(TextKey::MenuQuit), "menu.quit");
}

TEST(Localizer, MissingFileFallsBackToKeyNames) {
    FakeResourceReader empty;
    Localizer localizer(empty, Language::En);
    EXPECT_EQ(localizer.text(TextKey::NameComputer), "name.computer");
}

TEST(Localizer, FormatSubstitutesPlaceholders) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::En);
    EXPECT_EQ(localizer.format(TextKey::AppTitle, {{"name", "Ann"}}), "Ann wins!");
    EXPECT_EQ(localizer.format(TextKey::AppTitle, {}), "{name} wins!");
}

TEST(Localizer, SetLanguageReloadsAndNotifiesSubscribersBeforeReturning) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::En);
    int calls = 0;
    std::string seen;
    localizer.subscribe([&] {
        ++calls;
        seen = localizer.text(TextKey::MenuQuit);
    });
    localizer.setLanguage(Language::Ru);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(seen, "Выход");
    EXPECT_EQ(localizer.language(), Language::Ru);
}

TEST(Localizer, UnsubscribedCallbacksAreNotCalled) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::En);
    int calls = 0;
    const SubscriptionId id = localizer.subscribe([&] { ++calls; });
    localizer.unsubscribe(id);
    localizer.setLanguage(Language::Vi);
    EXPECT_EQ(calls, 0);
}

TEST(Localizer, SubscriberMayUnsubscribeDuringNotification) {
    auto reader = makeReader();
    Localizer localizer(reader, Language::En);
    SubscriptionId id = 0;
    int calls = 0;
    id = localizer.subscribe([&] {
        ++calls;
        localizer.unsubscribe(id);
    });
    localizer.setLanguage(Language::Ru);
    localizer.setLanguage(Language::En);
    EXPECT_EQ(calls, 1);
}

TEST(Localizer, DisplayNameUsesStoredNameOrTranslatedDefault) {
    auto reader = makeReader();
    InMemorySettingsStore store;
    SettingsService settings(store);
    Localizer localizer(reader, Language::Ru);

    EXPECT_EQ(localizer.displayName(Side::Player, settings), "Игрок");
    EXPECT_EQ(localizer.displayName(Side::Computer, settings), "Компьютер");

    settings.setPlayerName("Ann");
    EXPECT_EQ(localizer.displayName(Side::Player, settings), "Ann");

    localizer.setLanguage(Language::Vi);
    EXPECT_EQ(localizer.displayName(Side::Computer, settings), "Máy tính");
}
