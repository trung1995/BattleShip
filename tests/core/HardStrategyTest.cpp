#include <gtest/gtest.h>

#include "GridView.h"
#include "ScriptedRandom.h"
#include "bs/core/Ai.h"

using namespace bs::core;

namespace {
AiAction::Kind kindAt(HardStrategy& s, const GridView& view, int turn, bool first, bool helpAvailable,
                      IRandom& rng) {
    return s.choose(view, turn, first, helpAvailable, rng).kind;
}
} // namespace

TEST(HardStrategy, DrawsItsHelpTurnFromOneToTwentyAtConstruction) {
    ScriptedRandom rng{7};
    HardStrategy strategy(rng);
    EXPECT_EQ(rng.calls(), 1);
    EXPECT_EQ(strategy.helpTurn(), 7);
}

TEST(HardStrategy, UsesHelpOnTheFirstActionOfItsChosenTurn) {
    ScriptedRandom rng{7};
    HardStrategy strategy(rng);
    GridView view;
    EXPECT_EQ(kindAt(strategy, view, 6, true, true, rng), AiAction::Kind::Shot);
    EXPECT_EQ(kindAt(strategy, view, 7, false, true, rng), AiAction::Kind::Shot);
    EXPECT_EQ(kindAt(strategy, view, 7, true, true, rng), AiAction::Kind::Help);
}

TEST(HardStrategy, LaterTurnsStillQualifyIfTheChosenTurnWasMissed) {
    ScriptedRandom rng{3};
    HardStrategy strategy(rng);
    GridView view;
    EXPECT_EQ(kindAt(strategy, view, 9, true, true, rng), AiAction::Kind::Help);
}

TEST(HardStrategy, WaitsWhileAWoundedShipIsBeingFinished) {
    ScriptedRandom rng{2};
    HardStrategy strategy(rng);
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    const AiAction finishing = strategy.choose(view, 2, true, true, rng);
    EXPECT_EQ(finishing.kind, AiAction::Kind::Shot);
    EXPECT_EQ(std::abs(finishing.target.row - 5) + std::abs(finishing.target.col - 5), 1);

    view.set({5, 5}, Knowledge::Sunk);
    EXPECT_EQ(kindAt(strategy, view, 3, true, true, rng), AiAction::Kind::Help);
}

TEST(HardStrategy, NeverUsesHelpWhenItIsNotAvailable) {
    ScriptedRandom rng{1};
    HardStrategy strategy(rng);
    GridView view;
    EXPECT_EQ(kindAt(strategy, view, 5, true, false, rng), AiAction::Kind::Shot);
}

TEST(HardStrategy, CentrePrefersAWhollyUnknownCross) {
    GridView view;
    for (int r = 0; r < kBoardSize; ++r)
        for (int c = 0; c < kBoardSize; ++c) view.set({r, c}, Knowledge::Miss);
    for (const Coord c : {Coord{5, 5}, Coord{4, 5}, Coord{6, 5}, Coord{5, 4}, Coord{5, 6}})
        view.set(c, Knowledge::Unknown);
    ScriptedRandom rng{1, 0};
    HardStrategy strategy(rng);
    const AiAction action = strategy.choose(view, 1, true, true, rng);
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
        ScriptedRandom rng{1, pick};
        HardStrategy strategy(rng);
        const AiAction action = strategy.choose(view, 1, true, true, rng);
        ASSERT_EQ(action.kind, AiAction::Kind::Help);
        EXPECT_EQ(action.target, pick == 0 ? (Coord{5, 5}) : (Coord{5, 6}));
    }
}

TEST(HardStrategy, OtherwiseItPlaysExactlyLikeMedium) {
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        Mt19937Random hardRng(seed);
        Mt19937Random mediumRng(seed);
        mediumRng.nextInt(1, 20); // the draw Hard makes for its help turn
        HardStrategy hard(hardRng);
        HuntTargetStrategy medium;
        GridView view;
        for (int shot = 0; shot < 40; ++shot) {
            const AiAction h = hard.choose(view, shot + 1, true, false, hardRng);
            const AiAction m = medium.choose(view, shot + 1, true, false, mediumRng);
            ASSERT_EQ(h.target, m.target) << "seed " << seed << " shot " << shot;
            view.set(h.target, shot % 3 == 0 ? Knowledge::Hit : Knowledge::Miss);
        }
    }
}
