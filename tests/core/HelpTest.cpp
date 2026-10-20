#include <algorithm>

#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/GameSession.h"

using namespace bs::core;

namespace {
GameSession startedSession() {
    GameSession session(Difficulty::Hard, knownFleet());
    session.playerFleet() = knownFleet();
    EXPECT_EQ(session.startBattle(), StartStatus::Started);
    return session;
}
} // namespace

struct CrossCase {
    Coord centre;
    std::vector<Coord> expected;
};

class HelpCross : public ::testing::TestWithParam<CrossCase> {};

TEST_P(HelpCross, AffectsTheCentreAndItsOrthogonalNeighboursOnTheBoard) {
    GameSession session = startedSession();
    const HelpOutcome outcome = session.useHelp(Side::Player, GetParam().centre);
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_EQ(outcome.affected, GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(
    CentreEdgeCorner, HelpCross,
    ::testing::Values(CrossCase{{7, 4}, {{7, 4}, {6, 4}, {8, 4}, {7, 3}, {7, 5}}},  // centre: 5
                      CrossCase{{9, 4}, {{9, 4}, {8, 4}, {9, 3}, {9, 5}}},          // edge: 4
                      CrossCase{{9, 0}, {{9, 0}, {8, 0}, {9, 1}}}));               // corner: 3

TEST(Help, SkipsCellsAlreadyFiredUponAndLeavesThemUnchanged) {
    GameSession session = startedSession();
    session.fire(Side::Player, {0, 1}); // hit
    session.fire(Side::Player, {7, 4}); // miss, turn passes
    session.fire(Side::Computer, {9, 9});
    const HelpOutcome outcome = session.useHelp(Side::Player, {1, 1});
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_EQ(outcome.affected, (std::vector<Coord>{{1, 1}, {2, 1}, {1, 0}, {1, 2}}));
    EXPECT_EQ(session.viewOf(Side::Computer).at({0, 1}), Knowledge::Hit);
}

TEST(Help, SkipsMarkedCells) {
    GameSession session = startedSession();
    session.fire(Side::Player, {4, 3}); // sinks a 1-cell ship; (5,3) etc. become Marked
    const HelpOutcome outcome = session.useHelp(Side::Player, {6, 3});
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_EQ(std::count(outcome.affected.begin(), outcome.affected.end(), Coord{5, 3}), 0);
}

TEST(Help, CentreWhoseWholeCrossIsFiredUponIsRejectedAndNotConsumed) {
    GameSession session = startedSession();
    for (const Coord c : {Coord{9, 0}, Coord{8, 0}}) {
        session.fire(Side::Player, c);
        session.fire(Side::Computer, {9, 9 - c.row});
    }
    session.fire(Side::Player, {9, 1});
    session.fire(Side::Computer, {7, 9});
    ASSERT_EQ(session.currentTurn(), Side::Player);
    const HelpOutcome outcome = session.useHelp(Side::Player, {9, 0});
    EXPECT_EQ(outcome.status, ActionStatus::RejectedNoEffect);
    EXPECT_EQ(session.helpUsesLeft(Side::Player), kHelpUsesPerGame); // not consumed (FR-029)
    EXPECT_EQ(session.currentTurn(), Side::Player);
}

TEST(Help, EachParticipantStartsWithThreeUses) {
    GameSession session = startedSession();
    EXPECT_EQ(kHelpUsesPerGame, 3);
    EXPECT_EQ(session.helpUsesLeft(Side::Player), 3);
    EXPECT_EQ(session.helpUsesLeft(Side::Computer), 3);
    EXPECT_TRUE(session.helpAvailable(Side::Player));
}

TEST(Help, EachUseConsumesOneAndAFourthIsRejected) {
    GameSession session = startedSession();
    // Three crosses on empty water that do not overlap.
    const Coord centres[] = {{7, 1}, {7, 4}, {7, 7}};
    for (int used = 1; used <= kHelpUsesPerGame; ++used) {
        ASSERT_TRUE(session.helpAvailable(Side::Player));
        ASSERT_EQ(session.useHelp(Side::Player, centres[used - 1]).status, ActionStatus::Applied);
        EXPECT_EQ(session.helpUsesLeft(Side::Player), kHelpUsesPerGame - used);
        EXPECT_EQ(session.helpUsesLeft(Side::Computer), kHelpUsesPerGame); // US4 scenario 9
        ASSERT_EQ(session.fire(Side::Computer, {9, used}).status, ActionStatus::Applied); // miss
    }
    EXPECT_FALSE(session.helpAvailable(Side::Player));

    const int computerTurns = session.turnsTaken(Side::Computer);
    const HelpOutcome fourth = session.useHelp(Side::Player, {9, 5});
    EXPECT_EQ(fourth.status, ActionStatus::RejectedAlreadyUsed);
    EXPECT_TRUE(fourth.changes.empty());
    EXPECT_EQ(session.helpUsesLeft(Side::Player), 0);
    EXPECT_EQ(session.currentTurn(), Side::Player);
    EXPECT_EQ(session.turnsTaken(Side::Computer), computerTurns);
    EXPECT_EQ(session.viewOf(Side::Computer).at({9, 5}), Knowledge::Unknown);
}

TEST(Help, CountersAreSeparatePerParticipant) {
    GameSession session = startedSession();
    ASSERT_EQ(session.useHelp(Side::Player, {7, 4}).status, ActionStatus::Applied);
    ASSERT_EQ(session.useHelp(Side::Computer, {7, 4}).status, ActionStatus::Applied);
    ASSERT_EQ(session.useHelp(Side::Player, {7, 7}).status, ActionStatus::Applied);
    EXPECT_EQ(session.helpUsesLeft(Side::Player), 1);
    EXPECT_EQ(session.helpUsesLeft(Side::Computer), 2);
}

TEST(Help, CanReplaceTheExtraShotAfterAHit) {
    GameSession session = startedSession();
    const ShotOutcome hit = session.fire(Side::Player, {0, 0});
    ASSERT_EQ(hit.result, ShotResult::Hit);
    ASSERT_EQ(session.currentTurn(), Side::Player); // the extra shot (FR-019)

    const HelpOutcome outcome = session.useHelp(Side::Player, {7, 4});
    EXPECT_EQ(outcome.status, ActionStatus::Applied); // FR-027, US4 scenario 8
    EXPECT_EQ(session.helpUsesLeft(Side::Player), kHelpUsesPerGame - 1);
    EXPECT_EQ(session.currentTurn(), Side::Computer);
}

TEST(Help, TurnPassesEvenWhenItHits) {
    GameSession session = startedSession();
    const HelpOutcome outcome = session.useHelp(Side::Player, {0, 1});
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_EQ(session.currentTurn(), Side::Computer);
    EXPECT_EQ(session.turnsTaken(Side::Computer), 1);
}

TEST(Help, IsRejectedOutOfTurnOffBoardOrOutsideTheBattle) {
    GameSession placement(Difficulty::Hard, knownFleet());
    EXPECT_EQ(placement.useHelp(Side::Player, {5, 5}).status, ActionStatus::RejectedWrongPhase);

    GameSession session = startedSession();
    EXPECT_EQ(session.useHelp(Side::Computer, {5, 5}).status, ActionStatus::RejectedNotYourTurn);
    EXPECT_EQ(session.useHelp(Side::Player, {-1, 5}).status, ActionStatus::RejectedOffBoard);
    EXPECT_EQ(session.helpUsesLeft(Side::Player), kHelpUsesPerGame);
    EXPECT_EQ(session.helpUsesLeft(Side::Computer), kHelpUsesPerGame);
}

TEST(Help, SinkingResolvesTheCrossFirstThenMarks) {
    GameSession session = startedSession();
    // The 1-cell ship at (4,5) is sunk by the centre of the cross (4,5),(3,5),(5,5),(4,4),(4,6).
    const HelpOutcome outcome = session.useHelp(Side::Player, {4, 5});
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_EQ(outcome.sunkShips, (std::vector<ShipId>{7}));
    // Cross neighbours are recorded as real misses, not Marked.
    EXPECT_EQ(session.viewOf(Side::Computer).at({3, 5}), Knowledge::Miss);
    EXPECT_EQ(session.viewOf(Side::Computer).at({4, 4}), Knowledge::Miss);
    // Diagonal neighbours outside the cross are marked afterwards.
    EXPECT_EQ(session.viewOf(Side::Computer).at({3, 4}), Knowledge::Marked);
    EXPECT_EQ(session.viewOf(Side::Computer).at({5, 6}), Knowledge::Marked);
    EXPECT_EQ(session.viewOf(Side::Computer).at({4, 5}), Knowledge::Sunk);
}

TEST(Help, DestroyingTheLastShipCellsWinsImmediately) {
    GameSession session = startedSession();
    const Fleet fleet = knownFleet();
    // Sink everything except the 1-cell ship at (4,9), keeping the turn with hits.
    for (ShipId id = 0; id < kFleetSize; ++id) {
        if (id == 9) continue;
        for (const Coord c : fleet.placementOf(id)->cells()) {
            ASSERT_EQ(session.fire(Side::Player, c).status, ActionStatus::Applied);
        }
    }
    ASSERT_EQ(session.currentTurn(), Side::Player);
    const HelpOutcome outcome = session.useHelp(Side::Player, {5, 9});
    ASSERT_EQ(outcome.status, ActionStatus::Applied);
    EXPECT_TRUE(outcome.gameOver);
    EXPECT_EQ(session.phase(), Phase::Finished);
    EXPECT_EQ(session.winner(), Side::Player);
    EXPECT_EQ(session.currentTurn(), Side::Player); // no turn pass after the winning Help
}
