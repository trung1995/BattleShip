#include <algorithm>

#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/GameSession.h"

using namespace bs::core;

namespace {
GameSession startedSession() {
    GameSession session(Difficulty::Easy, knownFleet());
    session.playerFleet() = knownFleet();
    EXPECT_EQ(session.startBattle(), StartStatus::Started);
    return session;
}

std::vector<Knowledge> snapshot(const GameSession& s, Side side) {
    std::vector<Knowledge> cells;
    for (int r = 0; r < kBoardSize; ++r)
        for (int c = 0; c < kBoardSize; ++c) cells.push_back(s.viewOf(side).at({r, c}));
    return cells;
}

bool hasChange(const ShotOutcome& o, Coord c, Knowledge k) {
    return std::any_of(o.changes.begin(), o.changes.end(), [&](const CellChange& ch) {
        return ch.board == Side::Computer && ch.coord == c && ch.now == k;
    });
}
} // namespace

TEST(GameSession, StartsInPlacementWithThePlayerToMove) {
    GameSession session(Difficulty::Medium, knownFleet());
    EXPECT_EQ(session.phase(), Phase::Placement);
    EXPECT_EQ(session.currentTurn(), Side::Player);
    EXPECT_EQ(session.difficulty(), Difficulty::Medium);
}

TEST(GameSession, BattleStartsOnlyWithACompleteFleet) {
    GameSession session(Difficulty::Easy, knownFleet());
    Fleet nine = knownFleet();
    nine.remove(9);
    session.playerFleet() = nine;
    EXPECT_EQ(session.startBattle(), StartStatus::FleetIncomplete);
    EXPECT_EQ(session.phase(), Phase::Placement);

    session.playerFleet() = knownFleet();
    EXPECT_EQ(session.startBattle(), StartStatus::Started);
    EXPECT_EQ(session.phase(), Phase::Battle);
    EXPECT_EQ(session.startBattle(), StartStatus::WrongPhase);
}

TEST(GameSession, FiringBeforeTheBattleIsRejected) {
    GameSession session(Difficulty::Easy, knownFleet());
    EXPECT_EQ(session.fire(Side::Player, {0, 0}).status, ActionStatus::RejectedWrongPhase);
}

TEST(GameSession, RejectionsLeaveTheSessionUnchanged) {
    GameSession session = startedSession();
    session.fire(Side::Player, {9, 9}); // miss: computer's turn now
    ASSERT_EQ(session.currentTurn(), Side::Computer);
    session.fire(Side::Computer, {0, 0}); // hit: computer shoots again
    session.fire(Side::Computer, {4, 3}); // sinks a 1-cell ship, marks around it
    session.fire(Side::Computer, {9, 9}); // miss: player's turn
    ASSERT_EQ(session.currentTurn(), Side::Player);

    const auto before = snapshot(session, Side::Computer);
    EXPECT_EQ(session.fire(Side::Computer, {5, 5}).status, ActionStatus::RejectedNotYourTurn);
    EXPECT_EQ(session.fire(Side::Player, {10, 0}).status, ActionStatus::RejectedOffBoard);
    EXPECT_EQ(session.fire(Side::Player, {9, 9}).status, ActionStatus::RejectedAlreadyFired);
    EXPECT_EQ(snapshot(session, Side::Computer), before);
    EXPECT_EQ(session.currentTurn(), Side::Player);

    // Hit and Marked cells on the player's board are also "already fired" for the computer.
    session.fire(Side::Player, {8, 8});
    ASSERT_EQ(session.currentTurn(), Side::Computer);
    EXPECT_EQ(session.fire(Side::Computer, {0, 0}).status, ActionStatus::RejectedAlreadyFired);
    EXPECT_EQ(session.fire(Side::Computer, {3, 3}).status, ActionStatus::RejectedAlreadyFired);
}

TEST(GameSession, MissPassesTheTurnAndHitKeepsIt) {
    GameSession session = startedSession();
    const ShotOutcome hit = session.fire(Side::Player, {0, 0});
    EXPECT_EQ(hit.status, ActionStatus::Applied);
    EXPECT_EQ(hit.result, ShotResult::Hit);
    EXPECT_FALSE(hit.turnPassed);
    EXPECT_EQ(session.currentTurn(), Side::Player);

    const ShotOutcome sunk = session.fire(Side::Player, {4, 3});
    EXPECT_EQ(sunk.result, ShotResult::Sunk);
    EXPECT_FALSE(sunk.turnPassed);
    EXPECT_EQ(session.currentTurn(), Side::Player);

    const ShotOutcome miss = session.fire(Side::Player, {9, 9});
    EXPECT_EQ(miss.result, ShotResult::Miss);
    EXPECT_TRUE(miss.turnPassed);
    EXPECT_EQ(session.currentTurn(), Side::Computer);
}

TEST(GameSession, TurnsTakenCountsEachPassOfTheTurn) {
    GameSession session = startedSession();
    EXPECT_EQ(session.turnsTaken(Side::Player), 1); // the first turn passes to the player
    EXPECT_EQ(session.turnsTaken(Side::Computer), 0);
    session.fire(Side::Player, {0, 0}); // hit: no pass
    EXPECT_EQ(session.turnsTaken(Side::Computer), 0);
    session.fire(Side::Player, {9, 9});
    EXPECT_EQ(session.turnsTaken(Side::Computer), 1);
    session.fire(Side::Computer, {9, 9});
    EXPECT_EQ(session.turnsTaken(Side::Player), 2);
}

TEST(GameSession, SinkChangesIncludeTheShipAndTheMarkedRing) {
    GameSession session = startedSession();
    session.fire(Side::Player, {2, 4});
    const ShotOutcome sunk = session.fire(Side::Player, {2, 5});
    EXPECT_EQ(sunk.sunkShip, 3);
    EXPECT_TRUE(hasChange(sunk, {2, 4}, Knowledge::Sunk));
    EXPECT_TRUE(hasChange(sunk, {2, 5}, Knowledge::Sunk));
    EXPECT_TRUE(hasChange(sunk, {1, 3}, Knowledge::Marked));
    EXPECT_TRUE(hasChange(sunk, {3, 6}, Knowledge::Marked));
}

TEST(GameSession, LastShipCellEndsTheGameImmediately) {
    GameSession session = startedSession();
    const Fleet fleet = knownFleet();
    ShotOutcome last{};
    for (ShipId id = 0; id < kFleetSize; ++id) {
        for (const Coord c : fleet.placementOf(id)->cells()) {
            last = session.fire(Side::Player, c);
            ASSERT_EQ(last.status, ActionStatus::Applied);
        }
    }
    EXPECT_TRUE(last.gameOver);
    EXPECT_FALSE(last.turnPassed);
    EXPECT_EQ(session.phase(), Phase::Finished);
    EXPECT_EQ(session.winner(), Side::Player);
    EXPECT_EQ(session.fire(Side::Player, {9, 9}).status, ActionStatus::RejectedWrongPhase);
}

TEST(GameSession, OwnFleetIsAvailableForRendering) {
    GameSession session = startedSession();
    EXPECT_TRUE(session.ownFleet(Side::Player).isComplete());
    EXPECT_TRUE(session.ownFleet(Side::Computer).isComplete());
}

TEST(GameSession, PlayerFleetIsLockedOnceTheBattleStarts) {
    GameSession session = startedSession();
    EXPECT_THROW(session.playerFleet(), std::logic_error);
}
