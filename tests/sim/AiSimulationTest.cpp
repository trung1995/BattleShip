#include <array>
#include <cstdio>
#include <memory>

#include <gtest/gtest.h>

#include "bs/core/Ai.h"
#include "bs/core/AutoPlacer.h"
#include "bs/core/GameSession.h"

using namespace bs::core;

namespace {

struct GameResult {
    Side winner = Side::Player;
    int rejected = 0;
};

// Plays one headless game. `playerSide` plays as Side::Player, `computerSide` as Side::Computer.
GameResult play(Difficulty playerSide, Difficulty computerSide, std::uint32_t seed) {
    Mt19937Random rng(seed);
    BacktrackingPlacer placer;
    GameSession session(computerSide, placer.placeAll(rng).fleet);
    session.playerFleet() = placer.placeAll(rng).fleet;
    EXPECT_EQ(session.startBattle(), StartStatus::Started);

    std::array<std::unique_ptr<ITargetingStrategy>, 2> strategies{
        {makeStrategy(playerSide, rng), makeStrategy(computerSide, rng)}};

    GameResult result;
    Side lastShooter = Side::Computer;
    int actions = 0;
    while (session.phase() == Phase::Battle && actions < 1000) {
        const Side shooter = session.currentTurn();
        const bool firstAction = shooter != lastShooter;
        lastShooter = shooter;
        ITargetingStrategy& strategy = *strategies[shooter == Side::Player ? 0 : 1];
        const AiAction action = strategy.choose(session.viewOf(opponentOf(shooter)),
                                                session.turnsTaken(shooter), firstAction,
                                                session.helpAvailable(shooter), rng);
        const ActionStatus status = action.kind == AiAction::Kind::Help
                                        ? session.useHelp(shooter, action.target).status
                                        : session.fire(shooter, action.target).status;
        if (status != ActionStatus::Applied) {
            ++result.rejected;
            break;
        }
        ++actions;
    }
    EXPECT_EQ(session.phase(), Phase::Finished) << "seed " << seed;
    if (session.phase() == Phase::Finished) {
        result.winner = session.winner();
    }
    return result;
}

} // namespace

// SC-005 (Easy part): no rejected or repeated computer actions, every game has one winner.
TEST(AiSimulation, EasyVersusEasyNeverMakesAnIllegalMove) {
    for (std::uint32_t seed = 1; seed <= 1000; ++seed) {
        const GameResult result = play(Difficulty::Easy, Difficulty::Easy, seed);
        ASSERT_EQ(result.rejected, 0) << "seed " << seed;
    }
}

// SC-005: no rejected (and therefore no repeated) actions at any difficulty.
TEST(AiSimulation, EveryDifficultyNeverMakesAnIllegalMove) {
    for (const Difficulty d : {Difficulty::Medium, Difficulty::Hard}) {
        for (std::uint32_t seed = 1; seed <= 1000; ++seed) {
            const GameResult result = play(d, d, seed);
            ASSERT_EQ(result.rejected, 0) << "difficulty " << static_cast<int>(d) << " seed " << seed;
        }
    }
}

namespace {
// Wins for `stronger` against `weaker` over `games` seeded games, alternating which side each
// strategy plays (the Player side always shoots first).
int winsFor(Difficulty stronger, Difficulty weaker, int games) {
    int wins = 0;
    for (int game = 0; game < games; ++game) {
        const std::uint32_t seed = static_cast<std::uint32_t>(game + 1);
        const bool strongerIsPlayer = game % 2 == 0;
        const GameResult result = strongerIsPlayer ? play(stronger, weaker, seed) : play(weaker, stronger, seed);
        const Side strongerSide = strongerIsPlayer ? Side::Player : Side::Computer;
        wins += result.winner == strongerSide ? 1 : 0;
    }
    return wins;
}
} // namespace

// SC-004
TEST(AiSimulation, MediumBeatsEasyAtLeastSixtyPercentOfTheTime) {
    const int wins = winsFor(Difficulty::Medium, Difficulty::Easy, 100);
    std::printf("Medium vs Easy: %d/100 wins\n", wins);
    EXPECT_GE(wins, 60);
}

TEST(AiSimulation, HardWinsMoreThanItLosesAgainstMedium) {
    const int wins = winsFor(Difficulty::Hard, Difficulty::Medium, 100);
    std::printf("Hard vs Medium: %d/100 wins\n", wins);
    EXPECT_GT(wins, 100 - wins);
}
