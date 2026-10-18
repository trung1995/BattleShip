#include "bs/core/AutoPlacer.h"

#include <algorithm>
#include <array>
#include <vector>

namespace bs {
namespace core {

namespace {

// Counts, per cell, how many placed ships occupy it or its halo. A cell is free for a new ship
// only while its count is zero.
class Occupancy {
public:
    Occupancy() {
        for (auto& row : count_) row.fill(0);
    }

    bool isFree(const ShipPlacement& p) const {
        for (const Coord c : p.cells()) {
            if (!c.isOnBoard() || count_[idx(c.row)][idx(c.col)] != 0) return false;
        }
        return true;
    }

    void add(const ShipPlacement& p, int delta) {
        for (const Coord c : p.cells()) {
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    const Coord n{c.row + dr, c.col + dc};
                    if (n.isOnBoard()) count_[idx(n.row)][idx(n.col)] += delta;
                }
            }
        }
    }

private:
    static std::size_t idx(int v) { return static_cast<std::size_t>(v); }
    std::array<std::array<int, kBoardSize>, kBoardSize> count_;
};

class Search {
public:
    Search(Fleet& fleet, IRandom& rng) : fleet_(fleet), rng_(rng) {}

    bool run(std::vector<ShipId> ships) {
        for (ShipId id = 0; id < kFleetSize; ++id) {
            if (const ShipPlacement* p = fleet_.placementOf(id)) occupancy_.add(*p, +1);
        }
        std::stable_sort(ships.begin(), ships.end(), [](ShipId a, ShipId b) {
            return fleetSpec()[static_cast<std::size_t>(a)] > fleetSpec()[static_cast<std::size_t>(b)];
        });
        ships_ = ships;
        return place(0);
    }

private:
    bool place(std::size_t index) {
        if (index == ships_.size()) return true;
        if (++nodes_ > BacktrackingPlacer::kNodeBudget) return false;

        const ShipId id = ships_[index];
        std::vector<ShipPlacement> candidates = candidatesFor(fleetSpec()[static_cast<std::size_t>(id)]);
        shuffle(candidates, rng_);
        for (const ShipPlacement& candidate : candidates) {
            occupancy_.add(candidate, +1);
            fleet_.place(id, candidate);
            if (place(index + 1)) return true;
            fleet_.remove(id);
            occupancy_.add(candidate, -1);
            if (nodes_ > BacktrackingPlacer::kNodeBudget) return false;
        }
        return false;
    }

    std::vector<ShipPlacement> candidatesFor(int length) const {
        std::vector<ShipPlacement> result;
        for (int row = 0; row < kBoardSize; ++row) {
            for (int col = 0; col < kBoardSize; ++col) {
                const ShipPlacement h{{row, col}, length, Orientation::Horizontal};
                if (occupancy_.isFree(h)) result.push_back(h);
                if (length > 1) {
                    const ShipPlacement v{{row, col}, length, Orientation::Vertical};
                    if (occupancy_.isFree(v)) result.push_back(v);
                }
            }
        }
        return result;
    }

    Fleet& fleet_;
    IRandom& rng_;
    Occupancy occupancy_;
    std::vector<ShipId> ships_;
    long nodes_ = 0;
};

} // namespace

AutoPlaceResult BacktrackingPlacer::placeAll(IRandom& rng) const {
    AutoPlaceResult result;
    Search search(result.fleet, rng);
    result.status = search.run(result.fleet.missingShips()) ? AutoPlaceStatus::Ok
                                                            : AutoPlaceStatus::CannotFit;
    return result;
}

AutoPlaceResult BacktrackingPlacer::placeRemaining(const Fleet& partial, IRandom& rng) const {
    AutoPlaceResult result;
    result.fleet = partial; // hand-placed ships stay fixed; only the missing ones are searched
    Search search(result.fleet, rng);
    if (search.run(partial.missingShips())) {
        result.status = AutoPlaceStatus::Ok;
    } else {
        result.status = AutoPlaceStatus::CannotFit;
        result.fleet = partial; // the search may have left partial placements behind
    }
    return result;
}

} // namespace core
} // namespace bs
