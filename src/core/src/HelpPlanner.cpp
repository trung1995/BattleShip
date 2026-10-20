#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

#include "bs/core/Ai.h"

namespace bs {
namespace core {

constexpr int HelpPlanner::kLatestTurn; // C++14: definition for ODR-uses

HelpPlanner::HelpPlanner(IRandom& rng) {
    // Partial Fisher–Yates over the turns 1..kLatestTurn: a uniform choice of distinct turns.
    std::vector<int> turns(kLatestTurn);
    std::iota(turns.begin(), turns.end(), 1);
    for (int i = 0; i < kHelpUsesPerGame; ++i) {
        const int j = rng.nextInt(i, kLatestTurn - 1);
        std::swap(turns[static_cast<std::size_t>(i)], turns[static_cast<std::size_t>(j)]);
    }
    targetTurns_.assign(turns.begin(), turns.begin() + kHelpUsesPerGame);
    std::sort(targetTurns_.begin(), targetTurns_.end());
}

bool HelpPlanner::shouldUse(int turnIndex, bool firstActionOfTurn, int helpUsesLeft,
                            bool woundedShip) const {
    if (!firstActionOfTurn || helpUsesLeft <= 0 || woundedShip) {
        return false;
    }
    const auto reached = std::count_if(targetTurns_.begin(), targetTurns_.end(),
                                       [turnIndex](int turn) { return turn <= turnIndex; });
    const int spent = kHelpUsesPerGame - helpUsesLeft;
    return reached > spent;
}

Coord HelpPlanner::chooseCentre(const ITargetView& view, IRandom& rng) const {
    std::vector<Coord> whollyUnknown;
    std::vector<Coord> mostUnknown;
    int best = 0;
    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            const Coord centre{row, col};
            const Coord cross[] = {centre, {row - 1, col}, {row + 1, col}, {row, col - 1}, {row, col + 1}};
            int onBoard = 0;
            int unknown = 0;
            for (const Coord c : cross) {
                if (!c.isOnBoard()) continue;
                ++onBoard;
                unknown += view.at(c) == Knowledge::Unknown ? 1 : 0;
            }
            if (unknown == onBoard) {
                whollyUnknown.push_back(centre);
            }
            if (unknown > best) {
                best = unknown;
                mostUnknown.clear();
            }
            if (unknown == best && unknown > 0) {
                mostUnknown.push_back(centre);
            }
        }
    }
    const std::vector<Coord>& pool = whollyUnknown.empty() ? mostUnknown : whollyUnknown;
    if (pool.empty()) {
        return {0, 0}; // unreachable while the game is running
    }
    return pool[static_cast<std::size_t>(rng.nextInt(0, static_cast<int>(pool.size()) - 1))];
}

AiAction HardStrategy::choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                              int helpUsesLeft, IRandom& rng) {
    const bool wounded = hunt_.observe(view);
    if (planner_.shouldUse(turnIndex, firstActionOfTurn, helpUsesLeft, wounded)) {
        return {AiAction::Kind::Help, planner_.chooseCentre(view, rng)};
    }
    return hunt_.choose(view, turnIndex, firstActionOfTurn, helpUsesLeft, rng);
}

} // namespace core
} // namespace bs
