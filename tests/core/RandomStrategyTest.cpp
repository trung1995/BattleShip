#include <gtest/gtest.h>

#include "ScriptedRandom.h"
#include "bs/core/Ai.h"
#include "GridView.h"

using namespace bs::core;

TEST(RandomStrategy, PicksTheNthUnknownCellInRowMajorOrder) {
    GridView view;
    view.set({0, 0}, Knowledge::Miss);
    view.set({0, 1}, Knowledge::Hit);
    ScriptedRandom rng{2}; // third Unknown cell: (0,0) and (0,1) are known, so (0,4)
    RandomStrategy strategy;
    const AiAction action = strategy.choose(view, 1, true, false, rng);
    EXPECT_EQ(action.kind, AiAction::Kind::Shot);
    EXPECT_EQ(action.target, (Coord{0, 4}));
}

TEST(RandomStrategy, EarlierHitsDoNotInfluenceTheChoice) {
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    ScriptedRandom rng{0};
    RandomStrategy strategy;
    EXPECT_EQ(strategy.choose(view, 1, true, false, rng).target, (Coord{0, 0}));
}
