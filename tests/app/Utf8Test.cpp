#include <gtest/gtest.h>

#include "bs/app/Utf8.h"

using namespace bs::app;

TEST(Utf8, CountsCodePointsNotBytes) {
    EXPECT_EQ(codePointCount("Người chơi"), 10u);
    EXPECT_EQ(codePointCount("Компьютер"), 9u);
    EXPECT_EQ(codePointCount("Player"), 6u);
    EXPECT_EQ(codePointCount(""), 0u);
}

TEST(Utf8, TruncateNeverSplitsAMultibyteSequence) {
    const std::string cyrillic = "ЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖЖ"; // 25 code points, 50 bytes
    const std::string cut = truncateCodePoints(cyrillic, 20);
    EXPECT_EQ(codePointCount(cut), 20u);
    EXPECT_EQ(cut.size(), 40u);
}

TEST(Utf8, TruncateLeavesShortStringsUnchanged) {
    const std::string vietnamese = "Ẩẩ Người chơi Việt"; // 18 code points
    EXPECT_EQ(truncateCodePoints(vietnamese, 20), vietnamese);
    EXPECT_EQ(truncateCodePoints("abc", 3), "abc");
}

TEST(Utf8, TrimRemovesAsciiAndUnicodeSpaceAtBothEnds) {
    EXPECT_EQ(trim("  Ann \t\n"), "Ann");
    EXPECT_EQ(trim("\xC2\xA0" "Ann" "\xE2\x80\x83"), "Ann"); // U+00A0, U+2003
    EXPECT_EQ(trim("\xE3\x80\x80" "Анна"), "Анна");         // U+3000
    EXPECT_EQ(trim("Ann Lee"), "Ann Lee");
}

TEST(Utf8, TrimOfWhitespaceOnlyIsEmpty) {
    EXPECT_EQ(trim("   "), "");
    EXPECT_EQ(trim("\xC2\xA0 \xE3\x80\x80"), "");
    EXPECT_EQ(trim(""), "");
}
