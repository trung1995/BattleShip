#include <memory>
#include <set>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Fleets.h"
#include "MockGameAudio.h"
#include "ScriptedRandom.h"
#include "bs/app/BattleController.h"
#include "bs/app/TextKeys.h"
#include "bs/core/Ai.h"

using namespace bs::app;
using namespace bs::core;
using ::testing::_;
using ::testing::NiceMock;

namespace {
// A strategy that fires at scripted cells, for deterministic controller tests.
class ScriptedStrategy final : public ITargetingStrategy {
public:
    explicit ScriptedStrategy(std::vector<AiAction> actions) : actions_(std::move(actions)) {}
    AiAction choose(const ITargetView&, int turnIndex, bool firstActionOfTurn, int helpUsesLeft,
                    IRandom&) override {
        lastTurnIndex = turnIndex;
        lastFirstAction = firstActionOfTurn;
        lastHelpUsesLeft = helpUsesLeft;
        return actions_.at(next_++);
    }
    int lastTurnIndex = -1;
    bool lastFirstAction = false;
    int lastHelpUsesLeft = -1;

private:
    std::vector<AiAction> actions_;
    std::size_t next_ = 0;
};

AiAction shot(int row, int col) { return {AiAction::Kind::Shot, {row, col}}; }
AiAction help(int row, int col) { return {AiAction::Kind::Help, {row, col}}; }
} // namespace

class BattleControllerTest : public ::testing::Test {
protected:
    void start(std::vector<AiAction> computerActions, Difficulty difficulty = Difficulty::Easy) {
        session = std::make_unique<GameSession>(difficulty, knownFleet());
        session->playerFleet() = knownFleet();
        ASSERT_EQ(session->startBattle(), StartStatus::Started);
        auto owned = std::make_unique<ScriptedStrategy>(std::move(computerActions));
        strategy = owned.get();
        controller = std::make_unique<BattleController>(*session, std::move(owned), rng, audio);
    }

    std::unique_ptr<GameSession> session;
    ScriptedStrategy* strategy = nullptr;
    ScriptedRandom rng;
    NiceMock<MockGameAudio> audio;
    std::unique_ptr<BattleController> controller;
};

TEST_F(BattleControllerTest, StartsAwaitingThePlayer) {
    start({});
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);
    EXPECT_EQ(controller->turnIndicator().side, Side::Player);
    EXPECT_FALSE(controller->turnIndicator().gameOver);
}

TEST_F(BattleControllerTest, PlayerHitKeepsTheTurnAndMissPassesIt) {
    start({shot(9, 9)});
    const ActionFeedback hit = controller->onPlayerFire({0, 0});
    EXPECT_EQ(hit.status, ActionStatus::Applied);
    EXPECT_FALSE(hit.changes.empty());
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);

    const ActionFeedback miss = controller->onPlayerFire({9, 9});
    EXPECT_EQ(miss.status, ActionStatus::Applied);
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
    EXPECT_EQ(controller->turnIndicator().side, Side::Computer);
}

TEST_F(BattleControllerTest, ComputerActsOncePerDelay) {
    start({shot(0, 0), shot(9, 9)});
    controller->onPlayerFire({9, 9});
    EXPECT_TRUE(controller->tick(0.79).empty());
    const auto first = controller->tick(0.01);
    EXPECT_FALSE(first.empty()); // hit at (0,0)
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
    EXPECT_EQ(strategy->lastTurnIndex, 1);
    EXPECT_TRUE(strategy->lastFirstAction);

    const auto second = controller->tick(5.0); // one action only, however much time passed
    EXPECT_FALSE(second.empty());
    EXPECT_FALSE(strategy->lastFirstAction);
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer); // the miss returned the turn
    EXPECT_TRUE(controller->tick(5.0).empty());
}

TEST_F(BattleControllerTest, PlayerInputDuringTheComputersTurnIsRejected) {
    start({shot(9, 9)});
    controller->onPlayerFire({9, 9});
    const ActionFeedback rejected = controller->onPlayerFire({0, 0});
    EXPECT_EQ(rejected.status, ActionStatus::RejectedNotYourTurn);
    EXPECT_TRUE(rejected.changes.empty());
    EXPECT_EQ(session->viewOf(Side::Computer).at({0, 0}), Knowledge::Unknown);
}

TEST_F(BattleControllerTest, AlreadyFiredCellIsRejectedWithAMessage) {
    start({});
    controller->onPlayerFire({0, 0});
    const ActionFeedback rejected = controller->onPlayerFire({0, 0});
    EXPECT_EQ(rejected.status, ActionStatus::RejectedAlreadyFired);
    ASSERT_TRUE(rejected.hasMessage);
    EXPECT_EQ(rejected.message, TextKey::BattleRejectAlreadyFired);
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);
}

TEST_F(BattleControllerTest, LastHitEndsTheGame) {
    start({});
    EXPECT_CALL(audio, onGameOver(true));
    const Fleet fleet = knownFleet();
    for (ShipId id = 0; id < kFleetSize; ++id) {
        for (const Coord c : fleet.placementOf(id)->cells()) {
            controller->onPlayerFire(c);
        }
    }
    EXPECT_EQ(controller->mode(), Mode::GameOver);
    EXPECT_TRUE(controller->turnIndicator().gameOver);
    EXPECT_EQ(controller->onPlayerFire({9, 9}).status, ActionStatus::RejectedWrongPhase);
}

TEST_F(BattleControllerTest, ShotsPlayTheirSounds) {
    start({shot(9, 8)});
    EXPECT_CALL(audio, onShot(ShotResult::Hit));
    EXPECT_CALL(audio, onShot(ShotResult::Miss)).Times(2);
    controller->onPlayerFire({0, 0});
    controller->onPlayerFire({9, 9});
    controller->tick(1.0);
}

TEST_F(BattleControllerTest, ArmHelpEntersTargetingOnlyOnThePlayersTurn) {
    start({shot(9, 9)});
    controller->armHelp();
    EXPECT_EQ(controller->mode(), Mode::PlayerHelpTargeting);
    controller->disarmHelp();
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);

    controller->onPlayerFire({9, 9}); // computer's turn now
    controller->armHelp();
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
}

TEST_F(BattleControllerTest, PlayerHelpAppliesAndPassesTheTurn) {
    start({shot(9, 9)});
    EXPECT_CALL(audio, onHelp());
    controller->armHelp();
    const ActionFeedback feedback = controller->onPlayerHelp({0, 1});
    EXPECT_EQ(feedback.status, ActionStatus::Applied);
    EXPECT_FALSE(feedback.changes.empty());
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
    EXPECT_EQ(controller->playerHelpUsesLeft(), kHelpUsesPerGame - 1);
    EXPECT_TRUE(controller->playerHelpAvailable());
}

TEST_F(BattleControllerTest, HelpIsOnlyAcceptedWhileTargeting) {
    start({});
    EXPECT_EQ(controller->onPlayerHelp({0, 1}).status, ActionStatus::RejectedWrongPhase);
    controller->armHelp();
    EXPECT_EQ(controller->onPlayerFire({0, 1}).status, ActionStatus::RejectedNotYourTurn);
}

TEST_F(BattleControllerTest, HelpWithNoEffectStaysInTargetingWithAMessage) {
    start({shot(9, 8), shot(8, 9), shot(7, 9)});
    controller->onPlayerFire({9, 9});
    controller->tick(1.0);              // computer misses: player's turn
    controller->onPlayerFire({8, 9});   // miss
    controller->tick(1.0);
    controller->onPlayerFire({9, 8});   // miss
    ASSERT_EQ(controller->mode(), Mode::ComputerActing);
    controller->tick(1.0);
    ASSERT_EQ(controller->mode(), Mode::AwaitingPlayer);
    controller->armHelp();
    const ActionFeedback feedback = controller->onPlayerHelp({9, 9});
    EXPECT_EQ(feedback.status, ActionStatus::RejectedNoEffect);
    ASSERT_TRUE(feedback.hasMessage);
    EXPECT_EQ(feedback.message, TextKey::BattleRejectHelpNoEffect);
    EXPECT_EQ(controller->mode(), Mode::PlayerHelpTargeting);
    EXPECT_EQ(controller->playerHelpUsesLeft(), kHelpUsesPerGame);
}

TEST_F(BattleControllerTest, PlayerHelpCountsDownAndArmingStopsAtZero) {
    start({shot(9, 0), shot(9, 1), shot(9, 2)});
    const Coord centres[] = {{7, 1}, {7, 4}, {7, 7}};
    for (int i = 0; i < kHelpUsesPerGame; ++i) {
        ASSERT_TRUE(controller->playerHelpAvailable());
        controller->armHelp();
        ASSERT_EQ(controller->onPlayerHelp(centres[i]).status, ActionStatus::Applied);
        EXPECT_EQ(controller->playerHelpUsesLeft(), kHelpUsesPerGame - 1 - i);
        controller->tick(1.0); // computer misses, back to the player
        ASSERT_EQ(controller->mode(), Mode::AwaitingPlayer);
    }
    EXPECT_FALSE(controller->playerHelpAvailable());
    controller->armHelp();
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);
}

TEST_F(BattleControllerTest, HelpCanBeArmedForTheExtraShotAfterAHit) {
    start({shot(9, 9)});
    ASSERT_EQ(controller->onPlayerFire({0, 0}).status, ActionStatus::Applied); // hit
    ASSERT_EQ(controller->mode(), Mode::AwaitingPlayer);
    controller->armHelp();
    ASSERT_EQ(controller->mode(), Mode::PlayerHelpTargeting);
    EXPECT_EQ(controller->onPlayerHelp({7, 4}).status, ActionStatus::Applied);
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
    EXPECT_EQ(controller->playerHelpUsesLeft(), kHelpUsesPerGame - 1);
}

TEST_F(BattleControllerTest, ComputerHelpCounterIsShownOnlyOnHard) {
    for (const Difficulty d : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard}) {
        start({}, d);
        EXPECT_EQ(controller->showComputerHelpCounter(), d == Difficulty::Hard);
        EXPECT_EQ(controller->computerHelpUsesLeft(), kHelpUsesPerGame);
    }
}

TEST_F(BattleControllerTest, HelpChangesArriveAsOneBatchWithMarkingLast) {
    start({});
    controller->armHelp();
    // Sinks the 1-cell ship at (4,5); its diagonal neighbours get Marked after the cross.
    const ActionFeedback feedback = controller->onPlayerHelp({4, 5});
    ASSERT_EQ(feedback.status, ActionStatus::Applied);
    std::set<std::pair<int, int>> crossSeen;
    bool markingStarted = false;
    int marked = 0;
    for (const CellChange& change : feedback.changes) {
        if (change.now == Knowledge::Marked) {
            markingStarted = true;
            ++marked;
        } else {
            EXPECT_FALSE(markingStarted) << "cross results must come before the marking";
            crossSeen.insert({change.coord.row, change.coord.col});
        }
    }
    EXPECT_EQ(crossSeen, (std::set<std::pair<int, int>>{{4, 5}, {3, 5}, {5, 5}, {4, 4}, {4, 6}}));
    EXPECT_GT(marked, 0);
}

TEST_F(BattleControllerTest, WinningHelpEndsTheGame) {
    start({});
    const Fleet fleet = knownFleet();
    for (ShipId id = 0; id < 9; ++id) {
        for (const Coord c : fleet.placementOf(id)->cells()) controller->onPlayerFire(c);
    }
    EXPECT_CALL(audio, onGameOver(true));
    controller->armHelp();
    controller->onPlayerHelp({5, 9});
    EXPECT_EQ(controller->mode(), Mode::GameOver);
}

TEST_F(BattleControllerTest, ComputerHelpIsAppliedAndPassesTheTurn) {
    start({{AiAction::Kind::Help, {7, 4}}});
    EXPECT_CALL(audio, onHelp());
    controller->onPlayerFire({9, 9});
    const auto changes = controller->tick(1.0);
    EXPECT_EQ(changes.size(), 5u); // a full cross of misses on the player's board, in one batch
    EXPECT_EQ(controller->computerHelpUsesLeft(), kHelpUsesPerGame - 1);
    EXPECT_EQ(controller->computerHelpUsesLeft(), session->helpUsesLeft(Side::Computer));
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);
    EXPECT_EQ(strategy->lastHelpUsesLeft, kHelpUsesPerGame);
}

TEST_F(BattleControllerTest, StrategyIsToldHowManyHelpUsesTheComputerHasLeft) {
    start({help(7, 4), shot(9, 9)}, Difficulty::Hard);
    controller->onPlayerFire({9, 9});
    controller->tick(1.0); // computer Help: 3 -> 2
    controller->onPlayerFire({9, 8});
    controller->tick(1.0);
    EXPECT_EQ(strategy->lastHelpUsesLeft, kHelpUsesPerGame - 1);
}

TEST_F(BattleControllerTest, ExitRequestFreezesTheBattleUntilCancelled) {
    start({shot(9, 9)});
    controller->onPlayerFire({9, 9}); // computer to act
    controller->tick(0.5);            // 0.5 s of the 0.8 s delay used
    controller->requestExit();
    EXPECT_EQ(controller->mode(), Mode::ConfirmExit);

    EXPECT_TRUE(controller->tick(10.0).empty()); // the computer never acts behind the dialog
    EXPECT_EQ(session->viewOf(Side::Player).at({9, 9}), Knowledge::Unknown);
    EXPECT_NE(controller->onPlayerFire({0, 0}).status, ActionStatus::Applied);

    controller->cancelExit();
    EXPECT_EQ(controller->mode(), Mode::ComputerActing);
    EXPECT_TRUE(controller->tick(0.29).empty()); // the remaining delay is preserved
    EXPECT_FALSE(controller->tick(0.02).empty());
}

TEST_F(BattleControllerTest, ExitRequestWorksFromEveryInProgressMode) {
    start({});
    controller->requestExit();
    EXPECT_EQ(controller->mode(), Mode::ConfirmExit);
    controller->cancelExit();
    EXPECT_EQ(controller->mode(), Mode::AwaitingPlayer);

    controller->armHelp();
    controller->requestExit();
    EXPECT_EQ(controller->mode(), Mode::ConfirmExit);
    EXPECT_NE(controller->onPlayerHelp({7, 4}).status, ActionStatus::Applied);
    controller->cancelExit();
    EXPECT_EQ(controller->mode(), Mode::PlayerHelpTargeting);
}

TEST_F(BattleControllerTest, ConfirmExitOnlyAfterARequest) {
    start({});
    EXPECT_FALSE(controller->confirmExit());
    controller->requestExit();
    EXPECT_TRUE(controller->confirmExit());
}

TEST_F(BattleControllerTest, NoExitPromptOnceTheGameIsOver) {
    start({});
    const Fleet fleet = knownFleet();
    for (ShipId id = 0; id < kFleetSize; ++id) {
        for (const Coord c : fleet.placementOf(id)->cells()) controller->onPlayerFire(c);
    }
    controller->requestExit();
    EXPECT_EQ(controller->mode(), Mode::GameOver);
}
