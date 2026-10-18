#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/app/PlacementController.h"
#include "bs/core/AutoPlacer.h"

using namespace bs::app;
using namespace bs::core;

class PlacementControllerTest : public ::testing::Test {
protected:
    Fleet fleet;
    BacktrackingPlacer placer;
    Mt19937Random rng{11};
    PlacementController controller{fleet, placer, rng};
};

TEST_F(PlacementControllerTest, AutoPlaceAllFillsTheFleet) {
    controller.autoPlaceAll();
    EXPECT_TRUE(isLegalCompleteFleet(fleet));
    EXPECT_TRUE(controller.dockShips().empty());
}

TEST_F(PlacementControllerTest, BattleCanStartOnlyWithAllTenShips) {
    EXPECT_FALSE(controller.canStartBattle());
    EXPECT_EQ(controller.dockShips().size(), 10u);
    controller.autoPlaceAll();
    EXPECT_TRUE(controller.canStartBattle());
    fleet.remove(4);
    EXPECT_FALSE(controller.canStartBattle());
    EXPECT_EQ(controller.dockShips(), (std::vector<ShipId>{4}));
}

namespace {
bool covers(const Preview& preview, std::vector<Coord> expected) {
    return preview.cells == expected;
}
} // namespace

TEST_F(PlacementControllerTest, HoverPreviewsTheDraggedShipAndItsLegality) {
    controller.beginDragFromDock(1); // 3-cell ship
    const Preview ok = controller.hover({5, 5});
    EXPECT_TRUE(ok.legal);
    EXPECT_TRUE(covers(ok, {{5, 5}, {5, 6}, {5, 7}}));

    const Preview offBoard = controller.hover({5, 8});
    EXPECT_FALSE(offBoard.legal);
    EXPECT_EQ(offBoard.reason, PlacementError::OutOfBounds);
}

TEST_F(PlacementControllerTest, HoverReportsTouchingIncludingDiagonals) {
    ASSERT_EQ(fleet.place(6, {{4, 4}, 1, Orientation::Horizontal}), PlacementError::None);
    controller.beginDragFromDock(7);
    EXPECT_EQ(controller.hover({4, 4}).reason, PlacementError::Overlaps);
    EXPECT_EQ(controller.hover({3, 5}).reason, PlacementError::Touches);
    EXPECT_EQ(controller.hover({4, 5}).reason, PlacementError::Touches);
    EXPECT_TRUE(controller.hover({2, 6}).legal);
}

TEST_F(PlacementControllerTest, RotateDuringADragFlipsThePreview) {
    controller.beginDragFromDock(1);
    controller.rotate();
    EXPECT_TRUE(covers(controller.hover({2, 2}), {{2, 2}, {3, 2}, {4, 2}}));
}

TEST_F(PlacementControllerTest, LegalDropPlacesTheShip) {
    controller.beginDragFromDock(0);
    const DropResult result = controller.drop({0, 0});
    EXPECT_EQ(result.status, DropStatus::Placed);
    EXPECT_TRUE(fleet.isPlaced(0));
    EXPECT_FALSE(controller.isDragging());
}

TEST_F(PlacementControllerTest, IllegalDropFromTheDockLeavesTheShipInTheDock) {
    controller.beginDragFromDock(0);
    const DropResult result = controller.drop({0, 8});
    EXPECT_EQ(result.status, DropStatus::Rejected);
    EXPECT_EQ(result.reason, PlacementError::OutOfBounds);
    EXPECT_FALSE(fleet.isPlaced(0));
}

TEST_F(PlacementControllerTest, IllegalDropOfAPlacedShipKeepsItsPreviousPosition) {
    ASSERT_EQ(fleet.place(0, {{0, 0}, 4, Orientation::Horizontal}), PlacementError::None);
    ASSERT_EQ(fleet.place(1, {{5, 0}, 3, Orientation::Horizontal}), PlacementError::None);
    controller.beginDragPlaced(1);
    const DropResult result = controller.drop({1, 0});
    EXPECT_EQ(result.status, DropStatus::Rejected);
    EXPECT_EQ(result.reason, PlacementError::Touches);
    EXPECT_EQ(fleet.placementOf(1)->origin, (Coord{5, 0}));
}

TEST_F(PlacementControllerTest, DroppingOnTheDockRemovesAPlacedShip) {
    ASSERT_EQ(fleet.place(0, {{0, 0}, 4, Orientation::Horizontal}), PlacementError::None);
    controller.beginDragPlaced(0);
    controller.dropOnDock();
    EXPECT_FALSE(fleet.isPlaced(0));
    EXPECT_FALSE(controller.isDragging());
}

TEST_F(PlacementControllerTest, RotatingASelectedShipInPlace) {
    ASSERT_EQ(fleet.place(1, {{2, 2}, 3, Orientation::Horizontal}), PlacementError::None);
    controller.select(1);
    EXPECT_EQ(controller.rotate(), PlacementError::None);
    EXPECT_EQ(fleet.placementOf(1)->orientation, Orientation::Vertical);

    ASSERT_EQ(fleet.place(1, {{8, 5}, 3, Orientation::Horizontal}), PlacementError::None);
    controller.select(1);
    EXPECT_EQ(controller.rotate(), PlacementError::OutOfBounds); // would run off the bottom edge
    EXPECT_EQ(fleet.placementOf(1)->orientation, Orientation::Horizontal);
}

TEST_F(PlacementControllerTest, SelectedShipFollowsDropsAndRemovals) {
    EXPECT_EQ(controller.selectedShip(), -1);
    controller.beginDragFromDock(0);
    controller.drop({0, 0});
    EXPECT_EQ(controller.selectedShip(), 0);

    controller.beginDragFromDock(1);
    controller.drop({0, 8}); // rejected: the previous selection stays
    EXPECT_EQ(controller.selectedShip(), 0);

    controller.beginDragPlaced(0);
    controller.dropOnDock();
    EXPECT_EQ(controller.selectedShip(), -1);
}

TEST_F(PlacementControllerTest, SelectingAnUnplacedShipSelectsNothing) {
    controller.select(3);
    EXPECT_EQ(controller.selectedShip(), -1);
}

TEST_F(PlacementControllerTest, RotationThatWouldOverlapIsRejectedWithNoChange) {
    ASSERT_EQ(fleet.place(0, {{4, 0}, 4, Orientation::Horizontal}), PlacementError::None);
    ASSERT_EQ(fleet.place(1, {{2, 1}, 3, Orientation::Horizontal}), PlacementError::None);
    controller.select(1);
    EXPECT_EQ(controller.rotate(), PlacementError::Overlaps);
    EXPECT_EQ(fleet.placementOf(1)->orientation, Orientation::Horizontal);
}

TEST_F(PlacementControllerTest, CancelDragRestoresThePreviousState) {
    ASSERT_EQ(fleet.place(0, {{0, 0}, 4, Orientation::Horizontal}), PlacementError::None);
    controller.beginDragPlaced(0);
    controller.rotate();
    controller.hover({5, 5});
    controller.cancelDrag();
    EXPECT_FALSE(controller.isDragging());
    EXPECT_EQ(fleet.placementOf(0)->orientation, Orientation::Horizontal);
    EXPECT_EQ(fleet.placementOf(0)->origin, (Coord{0, 0}));
}

TEST_F(PlacementControllerTest, AutoPlaceRemainingOnlyAddsShips) {
    ASSERT_EQ(fleet.place(0, {{0, 0}, 4, Orientation::Horizontal}), PlacementError::None);
    EXPECT_EQ(controller.autoPlaceRemaining(), AutoResult::Ok);
    EXPECT_TRUE(isLegalCompleteFleet(fleet));
    EXPECT_EQ(fleet.placementOf(0)->origin, (Coord{0, 0}));
}

TEST_F(PlacementControllerTest, AutoPlaceRemainingThatCannotFitChangesNothing) {
    fleet = fleetWithNoRoomForTheFourShip();
    EXPECT_EQ(controller.autoPlaceRemaining(), AutoResult::CannotFit);
    EXPECT_FALSE(fleet.isPlaced(0));
    EXPECT_EQ(fleet.placementOf(1)->origin, (Coord{4, 8}));
}

TEST_F(PlacementControllerTest, DockListsUnplacedShipsGroupedByLength) {
    ASSERT_EQ(fleet.place(1, {{0, 0}, 3, Orientation::Horizontal}), PlacementError::None);
    EXPECT_EQ(controller.dockShips(), (std::vector<ShipId>{0, 2, 3, 4, 5, 6, 7, 8, 9}));
}
