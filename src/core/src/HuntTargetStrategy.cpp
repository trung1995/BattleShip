#include <algorithm>
#include <set>
#include <vector>

#include "bs/core/Ai.h"

namespace bs {
namespace core {

namespace {

// The orthogonally connected Hit cells containing `start`. Ships never touch, so this is
// exactly the hit part of one ship.
std::vector<Coord> clusterAt(const ITargetView& view, Coord start) {
    std::vector<Coord> cluster{start};
    std::set<Coord> seen{start};
    for (std::size_t i = 0; i < cluster.size(); ++i) {
        for (const Coord n : orthogonalNeighbours(cluster[i])) {
            if (view.at(n) == Knowledge::Hit && seen.insert(n).second) {
                cluster.push_back(n);
            }
        }
    }
    return cluster;
}

void addIfUnknown(const ITargetView& view, Coord c, std::vector<Coord>& out) {
    if (c.isOnBoard() && view.at(c) == Knowledge::Unknown &&
        std::find(out.begin(), out.end(), c) == out.end()) {
        out.push_back(c);
    }
}

std::vector<Coord> candidatesFor(const ITargetView& view, const std::vector<Coord>& cluster) {
    std::vector<Coord> out;
    if (cluster.size() >= 2) {
        const bool sameRow = std::all_of(cluster.begin(), cluster.end(),
                                         [&](Coord c) { return c.row == cluster[0].row; });
        const bool sameCol = std::all_of(cluster.begin(), cluster.end(),
                                         [&](Coord c) { return c.col == cluster[0].col; });
        if (sameRow || sameCol) {
            const auto bounds = std::minmax_element(cluster.begin(), cluster.end());
            const Coord first = *bounds.first;
            const Coord last = *bounds.second;
            if (sameRow) {
                addIfUnknown(view, {first.row, first.col - 1}, out);
                addIfUnknown(view, {last.row, last.col + 1}, out);
            } else {
                addIfUnknown(view, {first.row - 1, first.col}, out);
                addIfUnknown(view, {last.row + 1, last.col}, out);
            }
            return out;
        }
    }
    for (const Coord c : cluster) {
        for (const Coord n : orthogonalNeighbours(c)) addIfUnknown(view, n, out);
    }
    return out;
}

} // namespace

bool HuntTargetStrategy::observe(const ITargetView& view) {
    bool wounded = false;
    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            const Coord c{row, col};
            if (view.at(c) == Knowledge::Hit) {
                wounded = true;
                if (std::find(hitOrder_.begin(), hitOrder_.end(), c) == hitOrder_.end()) {
                    hitOrder_.push_back(c);
                }
            }
        }
    }
    return wounded;
}

AiAction HuntTargetStrategy::choose(const ITargetView& view, int /*turnIndex*/,
                                    bool /*firstActionOfTurn*/, int /*helpUsesLeft*/,
                                    IRandom& rng) {
    observe(view);
    std::set<Coord> visited;
    for (const Coord hit : hitOrder_) {
        if (view.at(hit) != Knowledge::Hit || visited.count(hit) != 0) {
            continue; // sunk since, or part of a cluster already considered
        }
        const std::vector<Coord> cluster = clusterAt(view, hit);
        visited.insert(cluster.begin(), cluster.end());
        const std::vector<Coord> candidates = candidatesFor(view, cluster);
        if (!candidates.empty()) {
            const int pick = rng.nextInt(0, static_cast<int>(candidates.size()) - 1);
            return {AiAction::Kind::Shot, candidates[static_cast<std::size_t>(pick)]};
        }
    }
    return {AiAction::Kind::Shot, randomUnknownCell(view, rng)};
}

} // namespace core
} // namespace bs
