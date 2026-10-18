#include <algorithm>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "bs/core/Random.h"

using namespace bs::core;

TEST(Mt19937Random, SameSeedGivesSameSequence) {
    Mt19937Random a(1234);
    Mt19937Random b(1234);
    for (int i = 0; i < 1000; ++i) {
        ASSERT_EQ(a.nextInt(0, 1000), b.nextInt(0, 1000)) << "at draw " << i;
    }
}

TEST(Mt19937Random, CoversBothEndsAndStaysInRange) {
    Mt19937Random rng(7);
    bool sawLo = false;
    bool sawHi = false;
    for (int i = 0; i < 10000; ++i) {
        const int v = rng.nextInt(3, 9);
        ASSERT_GE(v, 3);
        ASSERT_LE(v, 9);
        sawLo = sawLo || v == 3;
        sawHi = sawHi || v == 9;
    }
    EXPECT_TRUE(sawLo);
    EXPECT_TRUE(sawHi);
}

TEST(Mt19937Random, SingleValueRange) {
    Mt19937Random rng(1);
    EXPECT_EQ(rng.nextInt(5, 5), 5);
}

TEST(Mt19937Random, NegativeRange) {
    Mt19937Random rng(3);
    for (int i = 0; i < 1000; ++i) {
        const int v = rng.nextInt(-4, -2);
        ASSERT_GE(v, -4);
        ASSERT_LE(v, -2);
    }
}

// Golden values from the research R6 mapping applied to raw std::mt19937(42) output, which the
// C++ standard fully defines. Guards cross-platform determinism (Principle I).
TEST(Mt19937Random, GoldenSequenceForSeed42) {
    Mt19937Random rng(42);
    const std::vector<int> expected{42, 67, 76, 14, 26, 35, 20, 24, 50, 13};
    std::vector<int> actual;
    for (int i = 0; i < 10; ++i) {
        actual.push_back(rng.nextInt(0, 99));
    }
    EXPECT_EQ(actual, expected);
}

TEST(Mt19937Random, StandardEngineCheckValue) {
    std::mt19937 engine;
    engine.discard(9999);
    EXPECT_EQ(engine(), 4123659995u);
}

TEST(Shuffle, ReturnsAPermutation) {
    Mt19937Random rng(99);
    std::vector<int> values{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffle(values, rng);
    std::vector<int> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted, (std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}
