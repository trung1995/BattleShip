#include <gtest/gtest.h>

#include "bs/core/Fleet.h"

using namespace bs::core;

namespace {
ShipPlacement h(int row, int col, int length) { return {{row, col}, length, Orientation::Horizontal}; }
ShipPlacement v(int row, int col, int length) { return {{row, col}, length, Orientation::Vertical}; }
} // namespace

TEST(Fleet, SpecIsOneFourTwoThreesThreeTwosFourOnes) {
    const std::array<int, 10> expected{4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    EXPECT_EQ(fleetSpec(), expected);
}

TEST(Fleet, StartsEmptyAndCompletesWithAllTenShips) {
    Fleet fleet;
    EXPECT_FALSE(fleet.isComplete());
    EXPECT_EQ(fleet.missingShips().size(), 10u);

    // A legal layout: every ship separated by at least one cell.
    const ShipPlacement layout[10] = {h(0, 0, 4), h(0, 5, 3), h(2, 0, 3), h(2, 4, 2), h(2, 7, 2),
                                      h(4, 0, 2), h(4, 3, 1), h(4, 5, 1), h(4, 7, 1), h(4, 9, 1)};
    for (ShipId id = 0; id < 10; ++id) {
        ASSERT_EQ(fleet.place(id, layout[id]), PlacementError::None) << "ship " << id;
        EXPECT_TRUE(fleet.isPlaced(id));
    }
    EXPECT_TRUE(fleet.isComplete());
    EXPECT_TRUE(fleet.missingShips().empty());
}

TEST(Fleet, RemoveAndClear) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(0, 0, 4)), PlacementError::None);
    fleet.remove(0);
    EXPECT_FALSE(fleet.isPlaced(0));
    EXPECT_EQ(fleet.placementOf(0), nullptr);
    fleet.remove(0); // no-op when empty

    ASSERT_EQ(fleet.place(6, h(5, 5, 1)), PlacementError::None);
    fleet.clear();
    EXPECT_EQ(fleet.missingShips().size(), 10u);
}

TEST(Fleet, OutOfBounds) {
    Fleet fleet;
    EXPECT_EQ(fleet.canPlace(0, h(0, 7, 4)), PlacementError::OutOfBounds);
    EXPECT_EQ(fleet.canPlace(0, v(7, 0, 4)), PlacementError::OutOfBounds);
    EXPECT_EQ(fleet.canPlace(6, h(-1, 0, 1)), PlacementError::OutOfBounds);
}

TEST(Fleet, Overlaps) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    EXPECT_EQ(fleet.canPlace(1, v(1, 4, 3)), PlacementError::Overlaps);
}

TEST(Fleet, TouchesOnASide) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    EXPECT_EQ(fleet.canPlace(6, h(4, 4, 1)), PlacementError::Touches); // below
    EXPECT_EQ(fleet.canPlace(6, h(3, 7, 1)), PlacementError::Touches); // right end
}

TEST(Fleet, TouchesOnADiagonalOnly) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    EXPECT_EQ(fleet.canPlace(6, h(2, 2, 1)), PlacementError::Touches);
    EXPECT_EQ(fleet.canPlace(6, h(4, 7, 1)), PlacementError::Touches);
}

TEST(Fleet, OneEmptyCellOfSeparationIsLegal) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    EXPECT_EQ(fleet.canPlace(6, h(5, 3, 1)), PlacementError::None);
    EXPECT_EQ(fleet.canPlace(6, h(1, 1, 1)), PlacementError::None);
}

TEST(Fleet, CheckOrderIsOutOfBoundsThenOverlapsThenTouches) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(0, 6, 4)), PlacementError::None);
    // Off-board and overlapping: OutOfBounds wins.
    EXPECT_EQ(fleet.canPlace(1, h(0, 8, 3)), PlacementError::OutOfBounds);
    // Touching only.
    EXPECT_EQ(fleet.canPlace(1, v(0, 5, 3)), PlacementError::Touches);
    // Overlapping and touching: Overlaps wins.
    EXPECT_EQ(fleet.canPlace(1, v(0, 6, 3)), PlacementError::Overlaps);
}

TEST(Fleet, MovingAShipIgnoresItsOwnCells) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    EXPECT_EQ(fleet.place(0, h(3, 4, 4)), PlacementError::None); // shift right by one
    EXPECT_EQ(fleet.place(0, v(3, 4, 4)), PlacementError::None); // rotate in place
    ASSERT_NE(fleet.placementOf(0), nullptr);
    EXPECT_EQ(fleet.placementOf(0)->orientation, Orientation::Vertical);
}

TEST(Fleet, IllegalPlaceKeepsThePreviousPlacement) {
    Fleet fleet;
    ASSERT_EQ(fleet.place(0, h(3, 3, 4)), PlacementError::None);
    ASSERT_EQ(fleet.place(1, h(6, 3, 3)), PlacementError::None);
    EXPECT_EQ(fleet.place(1, h(4, 3, 3)), PlacementError::Touches);
    ASSERT_NE(fleet.placementOf(1), nullptr);
    EXPECT_EQ(fleet.placementOf(1)->origin, (Coord{6, 3}));
}

TEST(Fleet, ShipIdOutOfRange) {
    Fleet fleet;
    EXPECT_EQ(fleet.canPlace(10, h(0, 0, 1)), PlacementError::ShipIdOutOfRange);
    EXPECT_EQ(fleet.canPlace(-1, h(0, 0, 1)), PlacementError::ShipIdOutOfRange);
    EXPECT_EQ(fleet.place(10, h(0, 0, 1)), PlacementError::ShipIdOutOfRange);
}

TEST(Fleet, ShipsMayTouchTheBoardEdge) {
    Fleet fleet;
    EXPECT_EQ(fleet.place(0, v(6, 9, 4)), PlacementError::None);
    EXPECT_EQ(fleet.place(1, h(9, 0, 3)), PlacementError::None);
}

TEST(Fleet, PlacementLengthMustMatchTheShipsSpec) {
    Fleet fleet;
    EXPECT_THROW(fleet.place(0, h(0, 0, 3)), std::invalid_argument);
}
