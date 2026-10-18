#include <vector>

#include "bs/core/Ai.h"

namespace bs {
namespace core {

HelpPlanner::HelpPlanner(IRandom& rng) : targetTurn_(rng.nextInt(1, kLatestTurn)) {}

bool HelpPlanner::shouldUse(int turnIndex, bool firstActionOfTurn, bool helpAvailable,
                            bool woundedShip) const {
    return helpAvailable && firstActionOfTurn && turnIndex >= targetTurn_ && !woundedShip;
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
                              bool helpAvailable, IRandom& rng) {
    const bool wounded = hunt_.observe(view);
    if (planner_.shouldUse(turnIndex, firstActionOfTurn, helpAvailable, wounded)) {
        return {AiAction::Kind::Help, planner_.chooseCentre(view, rng)};
    }
    return hunt_.choose(view, turnIndex, firstActionOfTurn, helpAvailable, rng);
}

} // namespace core
} // namespace bs
