#include <algorithm>

#include <gtest/gtest.h>

#include "bs/core/Coord.h"

using namespace bs::core;

namespace {
bool contains(const std::vector<Coord>& cells, Coord c) {
    return std::find(cells.begin(), cells.end(), c) != cells.end();
}
} // namespace

TEST(Coord, BoardSizeIsTen) { EXPECT_EQ(kBoardSize, 10); }

TEST(Coord, IsOnBoardOnlyForRowAndColumnInZeroToNine) {
    EXPECT_TRUE((Coord{0, 0}).isOnBoard());
    EXPECT_TRUE((Coord{9, 9}).isOnBoard());
    EXPECT_TRUE((Coord{4, 7}).isOnBoard());
    EXPECT_FALSE((Coord{-1, 0}).isOnBoard());
    EXPECT_FALSE((Coord{0, -1}).isOnBoard());
    EXPECT_FALSE((Coord{10, 0}).isOnBoard());
    EXPECT_FALSE((Coord{0, 10}).isOnBoard());
}

TEST(Coord, EqualityAndOrderingAreByRowThenColumn) {
    EXPECT_EQ((Coord{2, 3}), (Coord{2, 3}));
    EXPECT_NE((Coord{2, 3}), (Coord{3, 2}));
    EXPECT_LT((Coord{2, 9}), (Coord{3, 0}));
    EXPECT_LT((Coord{2, 3}), (Coord{2, 4}));
}

TEST(Coord, OrthogonalNeighboursAreClippedToTheBoard) {
    EXPECT_EQ(orthogonalNeighbours({0, 0}).size(), 2u);
    EXPECT_EQ(orthogonalNeighbours({0, 5}).size(), 3u);
    EXPECT_EQ(orthogonalNeighbours({5, 5}).size(), 4u);
    const auto n = orthogonalNeighbours({5, 5});
    EXPECT_TRUE(contains(n, {4, 5}));
    EXPECT_TRUE(contains(n, {6, 5}));
    EXPECT_TRUE(contains(n, {5, 4}));
    EXPECT_TRUE(contains(n, {5, 6}));
}

TEST(Coord, AllNeighboursIncludeDiagonalsAndAreClipped) {
    EXPECT_EQ(allNeighbours({0, 0}).size(), 3u);
    EXPECT_EQ(allNeighbours({9, 4}).size(), 5u);
    EXPECT_EQ(allNeighbours({5, 5}).size(), 8u);
    EXPECT_TRUE(contains(allNeighbours({5, 5}), {4, 4}));
    EXPECT_FALSE(contains(allNeighbours({5, 5}), {5, 5}));
}

TEST(ShipPlacement, HorizontalGrowsAlongColumnFromTopLeftOrigin) {
    const ShipPlacement p{{2, 3}, 3, Orientation::Horizontal};
    const std::vector<Coord> expected{{2, 3}, {2, 4}, {2, 5}};
    EXPECT_EQ(p.cells(), expected);
}

TEST(ShipPlacement, VerticalGrowsAlongRow) {
    const ShipPlacement p{{2, 3}, 4, Orientation::Vertical};
    const std::vector<Coord> expected{{2, 3}, {3, 3}, {4, 3}, {5, 3}};
    EXPECT_EQ(p.cells(), expected);
}

TEST(ShipPlacement, CellsMayLeaveTheBoardSoCallersCanDetectIt) {
    const ShipPlacement p{{0, 8}, 3, Orientation::Horizontal};
    const auto cells = p.cells();
    ASSERT_EQ(cells.size(), 3u);
    EXPECT_FALSE(cells[2].isOnBoard());
}
