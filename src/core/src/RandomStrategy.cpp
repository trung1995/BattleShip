#include "bs/core/Ai.h"

#include <vector>

namespace bs {
namespace core {

Coord randomUnknownCell(const ITargetView& view, IRandom& rng) {
    std::vector<Coord> unknown;
    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            if (view.at({row, col}) == Knowledge::Unknown) unknown.push_back({row, col});
        }
    }
    if (unknown.empty()) {
        return {0, 0}; // unreachable while the game is running
    }
    return unknown[static_cast<std::size_t>(rng.nextInt(0, static_cast<int>(unknown.size()) - 1))];
}

AiAction RandomStrategy::choose(const ITargetView& view, int /*turnIndex*/,
                                bool /*firstActionOfTurn*/, int /*helpUsesLeft*/, IRandom& rng) {
    return {AiAction::Kind::Shot, randomUnknownCell(view, rng)};
}

} // namespace core
} // namespace bs
