#include <memory>
#include <set>

#include <gtest/gtest.h>

#include "GridView.h"
#include "bs/core/Ai.h"

using namespace bs::core;

// Liskov contract shared by every difficulty: a Shot at an Unknown cell, or a Help whose cross
// contains an Unknown cell and only when help is available.
template <class Strategy> struct Factory {
    static std::unique_ptr<ITargetingStrategy> make(IRandom&) {
        return std::make_unique<Strategy>();
    }
};
template <> struct Factory<HardStrategy> {
    static std::unique_ptr<ITargetingStrategy> make(IRandom& rng) {
        return std::make_unique<HardStrategy>(rng);
    }
};

template <class Strategy> class StrategyContract : public ::testing::Test {};

using Strategies = ::testing::Types<RandomStrategy, HuntTargetStrategy, HardStrategy>;
TYPED_TEST_SUITE(StrategyContract, Strategies, );

namespace {
bool crossHasUnknown(const ITargetView& view, Coord centre) {
    const Coord cross[] = {centre, {centre.row - 1, centre.col}, {centre.row + 1, centre.col},
                           {centre.row, centre.col - 1}, {centre.row, centre.col + 1}};
    for (const Coord c : cross) {
        if (c.isOnBoard() && view.at(c) == Knowledge::Unknown) return true;
    }
    return false;
}
} // namespace

TYPED_TEST(StrategyContract, EveryActionIsLegalAndNoCellIsRepeated) {
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        Mt19937Random rng(seed);
        auto strategy = Factory<TypeParam>::make(rng);
        GridView view;
        std::set<std::pair<int, int>> fired;
        bool helpAvailable = true;
        for (int action = 0; action < 100 && fired.size() < 100; ++action) {
            const AiAction a = strategy->choose(view, action + 1, true, helpAvailable, rng);
            ASSERT_TRUE(a.target.isOnBoard());
            if (a.kind == AiAction::Kind::Help) {
                ASSERT_TRUE(helpAvailable);
                ASSERT_TRUE(crossHasUnknown(view, a.target));
                helpAvailable = false;
                continue; // the contract is about choosing, not resolving
            }
            ASSERT_EQ(view.at(a.target), Knowledge::Unknown);
            ASSERT_TRUE(fired.insert({a.target.row, a.target.col}).second);
            view.set(a.target, (a.target.row + a.target.col) % 7 == 0 ? Knowledge::Hit : Knowledge::Miss);
        }
    }
}

TEST(StrategyContract, EasyAndMediumNeverUseHelp) {
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        Mt19937Random rng(seed);
        RandomStrategy easy;
        HuntTargetStrategy medium;
        GridView view;
        for (int turn = 1; turn <= 30; ++turn) {
            EXPECT_EQ(easy.choose(view, turn, true, true, rng).kind, AiAction::Kind::Shot);
            EXPECT_EQ(medium.choose(view, turn, true, true, rng).kind, AiAction::Kind::Shot);
        }
    }
}

TEST(StrategyFactory, BuildsOneStrategyPerDifficulty) {
    Mt19937Random rng(1);
    EXPECT_NE(dynamic_cast<RandomStrategy*>(makeStrategy(Difficulty::Easy, rng).get()), nullptr);
    EXPECT_NE(dynamic_cast<HuntTargetStrategy*>(makeStrategy(Difficulty::Medium, rng).get()), nullptr);
    EXPECT_NE(dynamic_cast<HardStrategy*>(makeStrategy(Difficulty::Hard, rng).get()), nullptr);
}
