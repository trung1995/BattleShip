#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/GameSession.h"

using namespace bs::core;

TEST(TargetView, ProjectsShotStatesAndHidesUnrevealedShips) {
    Board board(knownFleet());
    TargetView view(board);
    EXPECT_EQ(view.at({0, 0}), Knowledge::Unknown); // unrevealed ship cell (Principle III)
    EXPECT_EQ(view.at({9, 9}), Knowledge::Unknown);

    board.shoot({9, 9});
    EXPECT_EQ(view.at({9, 9}), Knowledge::Miss);
    board.shoot({0, 0});
    EXPECT_EQ(view.at({0, 0}), Knowledge::Hit);
    board.shoot({4, 3});
    EXPECT_EQ(view.at({4, 3}), Knowledge::Sunk);
    EXPECT_EQ(view.at({3, 3}), Knowledge::Marked);
}

TEST(TargetView, SessionNeverRevealsUntouchedComputerShipCells) {
    GameSession session(Difficulty::Easy, knownFleet());
    session.playerFleet() = knownFleet();
    ASSERT_EQ(session.startBattle(), StartStatus::Started);
    const ITargetView& view = session.viewOf(Side::Computer);
    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            EXPECT_EQ(view.at({r, c}), Knowledge::Unknown);
        }
    }
}
