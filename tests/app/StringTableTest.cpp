#include <gtest/gtest.h>

#include "bs/app/StringTable.h"

using namespace bs::app;

TEST(StringTable, ParsesTrimmedEntriesAndIgnoresCommentsAndBlankLines) {
    const auto result = StringTable::parse("# comment\n\n  menu.quit =  Quit  \nname.computer=Computer\n");
    ASSERT_TRUE(result.ok) << result.error;
    ASSERT_NE(result.table.find("menu.quit"), nullptr);
    EXPECT_EQ(*result.table.find("menu.quit"), "Quit");
    EXPECT_EQ(*result.table.find("name.computer"), "Computer");
    EXPECT_EQ(result.table.find("missing"), nullptr);
    EXPECT_EQ(result.table.size(), 2u);
}

TEST(StringTable, EscapedNewlineBecomesANewline) {
    const auto result = StringTable::parse("exit.message = Line one\\nLine two\n");
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(*result.table.find("exit.message"), "Line one\nLine two");
}

TEST(StringTable, ToleratesCrlfAndStripsBom) {
    const auto result = StringTable::parse("\xEF\xBB\xBFmenu.quit = Quit\r\nmenu.settings = Settings\r\n");
    ASSERT_TRUE(result.ok) << result.error;
    EXPECT_EQ(*result.table.find("menu.quit"), "Quit");
    EXPECT_EQ(*result.table.find("menu.settings"), "Settings");
}

TEST(StringTable, KeepsUtf8Values) {
    const auto result = StringTable::parse("name.computer = Máy tính\n");
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(*result.table.find("name.computer"), "Máy tính");
}

TEST(StringTable, DuplicateKeyIsAnErrorNamingKeyAndLine) {
    const auto result = StringTable::parse("a.b = 1\na.b = 2\n");
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errorLine, 2);
    EXPECT_NE(result.error.find("a.b"), std::string::npos);
}

TEST(StringTable, LineWithoutEqualsIsAnError) {
    const auto result = StringTable::parse("menu.quit Quit\n");
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errorLine, 1);
}

TEST(StringTable, InvalidKeyIsAnError) {
    EXPECT_FALSE(StringTable::parse("Menu.Quit = Quit\n").ok);
    EXPECT_FALSE(StringTable::parse("menu..quit = Quit\n").ok);
    EXPECT_FALSE(StringTable::parse(" = Quit\n").ok);
}

TEST(StringTable, ReportsPlaceholders) {
    const auto result = StringTable::parse("battle.winner = {name} wins! {name} {count}\n");
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.table.placeholders("battle.winner"), (std::set<std::string>{"name", "count"}));
    EXPECT_TRUE(result.table.placeholders("missing").empty());
}
