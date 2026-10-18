#include <algorithm>
#include <set>

#include <gtest/gtest.h>

#include "GridView.h"
#include "ScriptedRandom.h"
#include "bs/core/Ai.h"

using namespace bs::core;

namespace {
std::set<std::pair<int, int>> targetsOver(std::uint32_t seeds, const GridView& view) {
    std::set<std::pair<int, int>> targets;
    for (std::uint32_t seed = 0; seed < seeds; ++seed) {
        HuntTargetStrategy strategy;
        Mt19937Random rng(seed);
        const AiAction a = strategy.choose(view, 1, true, false, rng);
        EXPECT_EQ(a.kind, AiAction::Kind::Shot);
        targets.insert({a.target.row, a.target.col});
    }
    return targets;
}
using Set = std::set<std::pair<int, int>>;
} // namespace

TEST(HuntTargetStrategy, WithoutHitsItFiresAtARandomUnknownCell) {
    GridView view;
    view.set({0, 0}, Knowledge::Miss);
    ScriptedRandom rng{3};
    HuntTargetStrategy strategy;
    EXPECT_EQ(strategy.choose(view, 1, true, false, rng).target, (Coord{0, 4}));
}

TEST(HuntTargetStrategy, AfterOneHitItTriesOnlyTheOrthogonalNeighbours) {
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    EXPECT_EQ(targetsOver(200, view), (Set{{4, 5}, {6, 5}, {5, 4}, {5, 6}}));
}

TEST(HuntTargetStrategy, KnownNeighboursAreSkipped) {
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    view.set({4, 5}, Knowledge::Miss);
    view.set({5, 4}, Knowledge::Miss);
    EXPECT_EQ(targetsOver(200, view), (Set{{6, 5}, {5, 6}}));
}

TEST(HuntTargetStrategy, AlignedHitsAreFollowedAlongTheLineOnly) {
    GridView view;
    view.set({5, 5}, Knowledge::Hit);
    view.set({5, 6}, Knowledge::Hit);
    EXPECT_EQ(targetsOver(200, view), (Set{{5, 4}, {5, 7}}));

    GridView vertical;
    vertical.set({3, 2}, Knowledge::Hit);
    vertical.set({4, 2}, Knowledge::Hit);
    vertical.set({5, 2}, Knowledge::Hit);
    EXPECT_EQ(targetsOver(200, vertical), (Set{{2, 2}, {6, 2}}));
}

TEST(HuntTargetStrategy, WhenOneEndIsBlockedItFiresAtTheOther) {
    GridView edge;
    edge.set({5, 0}, Knowledge::Hit);
    edge.set({5, 1}, Knowledge::Hit);
    EXPECT_EQ(targetsOver(100, edge), (Set{{5, 2}}));

    GridView missed;
    missed.set({5, 4}, Knowledge::Miss);
    missed.set({5, 5}, Knowledge::Hit);
    missed.set({5, 6}, Knowledge::Hit);
    EXPECT_EQ(targetsOver(100, missed), (Set{{5, 7}}));
}

TEST(HuntTargetStrategy, ReturnsToRandomOnceTheShipIsSunk) {
    GridView view;
    view.set({0, 0}, Knowledge::Sunk);
    view.set({0, 1}, Knowledge::Marked);
    view.set({1, 0}, Knowledge::Marked);
    view.set({1, 1}, Knowledge::Marked);
    ScriptedRandom rng{0};
    HuntTargetStrategy strategy;
    EXPECT_EQ(strategy.choose(view, 1, true, false, rng).target, (Coord{0, 2}));
}

TEST(HuntTargetStrategy, FinishesTheShipItHitFirst) {
    // Hit B at (7,7) is observed before hit A at (2,2), even though A comes first in row order.
    HuntTargetStrategy strategy;
    Mt19937Random rng(1);
    GridView view;
    view.set({7, 7}, Knowledge::Hit);
    strategy.choose(view, 1, true, false, rng);
    view.set({2, 2}, Knowledge::Hit);
    for (int i = 0; i < 20; ++i) {
        const Coord t = strategy.choose(view, 2, false, false, rng).target;
        EXPECT_EQ(std::abs(t.row - 7) + std::abs(t.col - 7), 1) << t.row << "," << t.col;
    }
}
