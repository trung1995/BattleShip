#include <algorithm>

#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/Board.h"

using namespace bs::core;

namespace {
bool hasUpdate(const std::vector<CellUpdate>& updates, Coord c, Knowledge k) {
    return std::any_of(updates.begin(), updates.end(),
                       [&](const CellUpdate& u) { return u.coord == c && u.now == k; });
}
} // namespace

TEST(Board, EmptyCellIsAMiss) {
    Board board(knownFleet());
    const ShotEffect effect = board.shoot({9, 9});
    EXPECT_EQ(effect.result, ShotResult::Miss);
    EXPECT_EQ(board.shotState({9, 9}), CellShotState::Miss);
    EXPECT_TRUE(hasUpdate(effect.updates, {9, 9}, Knowledge::Miss));
}

TEST(Board, ShipCellIsAHitUntilTheLastCellSinksIt) {
    Board board(knownFleet());
    EXPECT_EQ(board.shoot({2, 4}).result, ShotResult::Hit);
    EXPECT_EQ(board.shipState(3), ShipState::Damaged);
    const ShotEffect effect = board.shoot({2, 5});
    EXPECT_EQ(effect.result, ShotResult::Sunk);
    EXPECT_EQ(effect.sunkShip, 3);
    EXPECT_EQ(board.shipState(3), ShipState::Sunk);
    EXPECT_TRUE(hasUpdate(effect.updates, {2, 4}, Knowledge::Sunk));
    EXPECT_TRUE(hasUpdate(effect.updates, {2, 5}, Knowledge::Sunk));
}

TEST(Board, ShipStatesStartIntact) {
    Board board(knownFleet());
    for (ShipId id = 0; id < kFleetSize; ++id) {
        EXPECT_EQ(board.shipState(id), ShipState::Intact);
    }
}

TEST(Board, SinkingMarksEveryUntouchedSideAndCornerNeighbour) {
    Board board(knownFleet());
    const ShotEffect effect = board.shoot({4, 3}); // 1-cell ship
    ASSERT_EQ(effect.result, ShotResult::Sunk);
    const Coord ring[] = {{3, 2}, {3, 3}, {3, 4}, {4, 2}, {4, 4}, {5, 2}, {5, 3}, {5, 4}};
    for (const Coord c : ring) {
        EXPECT_EQ(board.shotState(c), CellShotState::Marked) << c.row << "," << c.col;
        EXPECT_TRUE(hasUpdate(effect.updates, c, Knowledge::Marked));
    }
}

TEST(Board, MarkingKeepsCellsAlreadyFiredUponAndIsClippedAtEdges) {
    Board board(knownFleet());
    board.shoot({1, 4});                           // miss next to the 4-cell ship
    for (int col = 0; col < 4; ++col) board.shoot({0, col});
    EXPECT_EQ(board.shotState({1, 4}), CellShotState::Miss);
    EXPECT_EQ(board.shotState({1, 0}), CellShotState::Marked);
    EXPECT_EQ(board.shotState({0, 4}), CellShotState::Marked);
}

TEST(Board, DeferredMarkingWaitsForMarkAround) {
    Board board(knownFleet());
    const ShotEffect effect = board.shoot({4, 3}, true);
    EXPECT_EQ(effect.result, ShotResult::Sunk);
    EXPECT_EQ(board.shotState({3, 3}), CellShotState::Untouched);
    const auto updates = board.markAround(6);
    EXPECT_EQ(board.shotState({3, 3}), CellShotState::Marked);
    EXPECT_TRUE(hasUpdate(updates, {3, 3}, Knowledge::Marked));
}

TEST(Board, MarkedCellsAreFiredUponButNotMisses) {
    Board board(knownFleet());
    board.shoot({4, 3});
    EXPECT_TRUE(board.isFiredUpon({3, 3}));
    EXPECT_NE(board.shotState({3, 3}), CellShotState::Miss);
    EXPECT_EQ(board.knowledgeAt({3, 3}), Knowledge::Marked);
    EXPECT_FALSE(board.isFiredUpon({9, 9}));
}

TEST(Board, RemainingCellsCountsDownFromTwenty) {
    Board board(knownFleet());
    EXPECT_EQ(board.remainingCells(), 20);
    board.shoot({0, 0});
    EXPECT_EQ(board.remainingCells(), 19);
    board.shoot({9, 9});
    EXPECT_EQ(board.remainingCells(), 19);
}

TEST(Board, ShootingAFiredCellIsAProgrammingError) {
    Board board(knownFleet());
    board.shoot({9, 9});
    EXPECT_THROW(board.shoot({9, 9}), std::logic_error);
    EXPECT_THROW(board.shoot({10, 0}), std::logic_error);
}
