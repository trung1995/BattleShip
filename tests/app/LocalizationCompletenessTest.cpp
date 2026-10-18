#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "bs/app/StringTable.h"
#include "bs/app/TextKeys.h"

using namespace bs::app;

namespace {
StringTable load(const std::string& code) {
    const std::string path = std::string(BATTLESHIP_RESOURCES_DIR) + "/i18n/" + code + ".lang";
    std::ifstream in(path, std::ios::binary);
    EXPECT_TRUE(in.good()) << "cannot open " << path;
    std::stringstream buffer;
    buffer << in.rdbuf();
    auto result = StringTable::parse(buffer.str());
    EXPECT_TRUE(result.ok) << path << ":" << result.errorLine << ": " << result.error;
    return result.table;
}

std::set<std::string> declaredKeys() {
    std::set<std::string> keys;
    for (const TextKey key : allTextKeys()) {
        keys.insert(keyName(key));
    }
    return keys;
}

std::set<std::string> keysOf(const StringTable& table) {
    std::set<std::string> keys;
    for (const auto& entry : table.entries()) {
        keys.insert(entry.first);
    }
    return keys;
}
} // namespace

class LocalizationCompleteness : public ::testing::TestWithParam<const char*> {};

TEST_P(LocalizationCompleteness, KeySetMatchesTextKeysExactly) {
    EXPECT_EQ(keysOf(load(GetParam())), declaredKeys());
}

TEST_P(LocalizationCompleteness, PlaceholdersMatchEnglish) {
    const StringTable english = load("en");
    const StringTable other = load(GetParam());
    for (const std::string& key : declaredKeys()) {
        EXPECT_EQ(other.placeholders(key), english.placeholders(key)) << key;
    }
}

TEST_P(LocalizationCompleteness, NoEmptyValues) {
    const StringTable table = load(GetParam());
    for (const auto& entry : table.entries()) {
        EXPECT_FALSE(entry.second.empty()) << entry.first;
    }
}

INSTANTIATE_TEST_SUITE_P(AllLanguages, LocalizationCompleteness,
                         ::testing::Values("en", "ru", "vi"));

TEST(LocalizationCompleteness, LanguageNamesAreTheSameInEveryFile) {
    const StringTable en = load("en");
    for (const char* code : {"ru", "vi"}) {
        const StringTable other = load(code);
        for (const char* key : {"lang.en", "lang.ru", "lang.vi"}) {
            ASSERT_NE(other.find(key), nullptr) << code << " " << key;
            ASSERT_NE(en.find(key), nullptr) << key;
            EXPECT_EQ(*other.find(key), *en.find(key)) << code << " " << key;
        }
    }
}

TEST(LocalizationCompleteness, KeyNamesAreUnique) {
    EXPECT_EQ(declaredKeys().size(), allTextKeys().size());
}
