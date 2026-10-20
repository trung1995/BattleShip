#include <algorithm>
#include <cstdlib>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GridView.h"
#include "ScriptedRandom.h"
#include "bs/core/Ai.h"

using namespace bs::core;

namespace {
AiAction::Kind kindAt(HardStrategy& s, const GridView& view, int turn, bool first, int helpUsesLeft,
                      IRandom& rng) {
    return s.choose(view, turn, first, helpUsesLeft, rng).kind;
}

// The schedule draws a partial Fisher–Yates over the turns 1..20: nextInt(i, 19) for i = 0, 1, 2
// picks the array index swapped into slot i. Scripting {2, 3, 9} therefore gives turns {3, 4, 10}.
ScriptedRandom scheduleFor3And4And10() { return ScriptedRandom{2, 3, 9}; }
} // namespace

TEST(HardStrategy, DrawsThreeDistinctHelpTurnsAtConstruction) {
    ScriptedRandom rng = scheduleFor3And4And10();
    HardStrategy strategy(rng);
    EXPECT_EQ(rng.ranges(), (std::vector<std::pair<int, int>>{{0, 19}, {1, 19}, {2, 19}}));
    EXPECT_EQ(strategy.helpTurns(), (std::vector<int>{3, 4, 10}));
}

TEST(HardStrategy, ScheduleIsSortedDistinctAndWithinTheFirstTwentyTurns) {
    for (std::uint32_t seed = 0; seed < 10000; ++seed) {
        Mt19937Random rng(seed);
        const HelpPlanner planner(rng);
        const std::vector<int>& turns = planner.targetTurns();
        ASSERT_EQ(turns.size(), static_cast<std::size_t>(kHelpUsesPerGame)) << "seed " << seed;
        for (std::size_t i = 0; i < turns.size(); ++i) {
            ASSERT_GE(turns[i], 1) << "seed " << seed;
            ASSERT_LE(turns[i], HelpPlanner::kLatestTurn) << "seed " << seed;
            if (i > 0) ASSERT_LT(turns[i - 1], turns[i]) << "seed " << seed;
        }
    }
}

TEST(HardStrategy, AUseIsDueOnceItsTurnIsReachedAndNotYetSpent) {
    ScriptedRandom rng = scheduleFor3And4And10();
    const HelpPlanner planner(rng);
    EXPECT_FALSE(planner.shouldUse(2, true, 3, false));  // nothing due yet
    EXPECT_TRUE(planner.shouldUse(3, true, 3, false));   // first target
    EXPECT_FALSE(planner.shouldUse(3, false, 3, false)); // only as the first action of a turn
    EXPECT_FALSE(planner.shouldUse(3, true, 3, true));   // not while finishing a wounded ship
    EXPECT_TRUE(planner.shouldUse(4, true, 2, false));   // second target
    EXPECT_FALSE(planner.shouldUse(5, true, 1, false));  // both reached targets are spent
    EXPECT_TRUE(planner.shouldUse(10, true, 1, false));  // third target
    EXPECT_FALSE(planner.shouldUse(20, true, 0, false)); // all three spent
}

TEST(HardStrategy, UsesDeferredByAWoundedShipAreTakenOnTheNextFreeTurns) {
    ScriptedRandom rng = scheduleFor3And4And10();
    HardStrategy strategy(rng);
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    int usesLeft = kHelpUsesPerGame;
    std::vector<int> helpTurns;
    for (int turn = 1; turn <= 12; ++turn) {
        if (turn == 6) view.set({5, 5}, Knowledge::Sunk); // wounded on turns 1..5
        if (kindAt(strategy, view, turn, true, usesLeft, rng) == AiAction::Kind::Help) {
            helpTurns.push_back(turn);
            --usesLeft;
        }
    }
    EXPECT_EQ(helpTurns, (std::vector<int>{6, 7, 10}));
}

TEST(HardStrategy, AtMostOneHelpPerTurnEvenWhenAllUsesAreDue) {
    ScriptedRandom rng; // falls back to lo: turns {1, 2, 3}
    HardStrategy strategy(rng);
    ASSERT_EQ(strategy.helpTurns(), (std::vector<int>{1, 2, 3}));
    GridView view;
    EXPECT_EQ(kindAt(strategy, view, 5, true, 3, rng), AiAction::Kind::Help);
    EXPECT_EQ(kindAt(strategy, view, 5, false, 2, rng), AiAction::Kind::Shot);
    EXPECT_EQ(kindAt(strategy, view, 6, true, 2, rng), AiAction::Kind::Help);
    EXPECT_EQ(kindAt(strategy, view, 7, true, 1, rng), AiAction::Kind::Help);
    EXPECT_EQ(kindAt(strategy, view, 8, true, 0, rng), AiAction::Kind::Shot);
}

TEST(HardStrategy, WaitsWhileAWoundedShipIsBeingFinished) {
    ScriptedRandom rng{1}; // turns {2, 1, 3} sorted to {1, 2, 3}
    HardStrategy strategy(rng);
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    const AiAction finishing = strategy.choose(view, 2, true, 3, rng);
    EXPECT_EQ(finishing.kind, AiAction::Kind::Shot);
    EXPECT_EQ(std::abs(finishing.target.row - 5) + std::abs(finishing.target.col - 5), 1);

    view.set({5, 5}, Knowledge::Sunk);
    EXPECT_EQ(kindAt(strategy, view, 3, true, 3, rng), AiAction::Kind::Help);
}

TEST(HardStrategy, NeverUsesHelpWithNoUsesLeft) {
    ScriptedRandom rng;
    HardStrategy strategy(rng);
    GridView view;
    for (int turn = 1; turn <= 20; ++turn) {
        EXPECT_EQ(kindAt(strategy, view, turn, true, 0, rng), AiAction::Kind::Shot);
    }
}

TEST(HardStrategy, CentrePrefersAWhollyUnknownCross) {
    GridView view;
    for (int r = 0; r < kBoardSize; ++r)
        for (int c = 0; c < kBoardSize; ++c) view.set({r, c}, Knowledge::Miss);
    for (const Coord c : {Coord{5, 5}, Coord{4, 5}, Coord{6, 5}, Coord{5, 4}, Coord{5, 6}})
        view.set(c, Knowledge::Unknown);
    ScriptedRandom rng{0, 1, 2, 0}; // turns {1, 2, 3}, then the centre pick
    HardStrategy strategy(rng);
    const AiAction action = strategy.choose(view, 1, true, 3, rng);
    EXPECT_EQ(action.kind, AiAction::Kind::Help);
    EXPECT_EQ(action.target, (Coord{5, 5}));
}

TEST(HardStrategy, CentreFallsBackToTheMostUnknownCrossCells) {
    GridView view;
    for (int r = 0; r < kBoardSize; ++r)
        for (int c = 0; c < kBoardSize; ++c) view.set({r, c}, Knowledge::Miss);
    view.set({5, 5}, Knowledge::Unknown);
    view.set({5, 6}, Knowledge::Unknown);
    for (int pick = 0; pick < 2; ++pick) {
        ScriptedRandom rng{0, 1, 2, pick};
        HardStrategy strategy(rng);
        const AiAction action = strategy.choose(view, 1, true, 3, rng);
        ASSERT_EQ(action.kind, AiAction::Kind::Help);
        EXPECT_EQ(action.target, pick == 0 ? (Coord{5, 5}) : (Coord{5, 6}));
    }
}

TEST(HardStrategy, OtherwiseItPlaysExactlyLikeMedium) {
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        Mt19937Random hardRng(seed);
        Mt19937Random mediumRng(seed);
        for (int i = 0; i < kHelpUsesPerGame; ++i) {
            mediumRng.nextInt(i, HelpPlanner::kLatestTurn - 1); // the draws Hard makes for its schedule
        }
        HardStrategy hard(hardRng);
        HuntTargetStrategy medium;
        GridView view;
        for (int shot = 0; shot < 40; ++shot) {
            const AiAction h = hard.choose(view, shot + 1, true, 0, hardRng);
            const AiAction m = medium.choose(view, shot + 1, true, 0, mediumRng);
            ASSERT_EQ(h.target, m.target) << "seed " << seed << " shot " << shot;
            view.set(h.target, shot % 3 == 0 ? Knowledge::Hit : Knowledge::Miss);
        }
    }
}
