#include "bs/core/Fleet.h"

#include <cstdlib>
#include <stdexcept>

namespace bs {
namespace core {

namespace {
bool validId(ShipId id) { return id >= 0 && id < kFleetSize; }

bool touchesOrOverlaps(Coord a, Coord b) {
    return std::abs(a.row - b.row) <= 1 && std::abs(a.col - b.col) <= 1;
}
} // namespace

const std::array<int, kFleetSize>& fleetSpec() {
    static const std::array<int, kFleetSize> spec{4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    return spec;
}

PlacementError Fleet::canPlace(ShipId id, const ShipPlacement& placement) const {
    if (!validId(id)) {
        return PlacementError::ShipIdOutOfRange;
    }
    if (placement.length != fleetSpec()[static_cast<std::size_t>(id)]) {
        throw std::invalid_argument("placement length does not match the ship's length");
    }
    const std::vector<Coord> cells = placement.cells();
    for (const Coord c : cells) {
        if (!c.isOnBoard()) {
            return PlacementError::OutOfBounds;
        }
    }

    bool touches = false;
    for (ShipId other = 0; other < kFleetSize; ++other) {
        if (other == id || !placed_[static_cast<std::size_t>(other)]) {
            continue;
        }
        for (const Coord theirs : placements_[static_cast<std::size_t>(other)].cells()) {
            for (const Coord mine : cells) {
                if (mine == theirs) {
                    return PlacementError::Overlaps;
                }
                touches = touches || touchesOrOverlaps(mine, theirs);
            }
        }
    }
    return touches ? PlacementError::Touches : PlacementError::None;
}

PlacementError Fleet::place(ShipId id, const ShipPlacement& placement) {
    const PlacementError error = canPlace(id, placement);
    if (error == PlacementError::None) {
        placed_[static_cast<std::size_t>(id)] = true;
        placements_[static_cast<std::size_t>(id)] = placement;
    }
    return error;
}

void Fleet::remove(ShipId id) {
    if (validId(id)) {
        placed_[static_cast<std::size_t>(id)] = false;
    }
}

void Fleet::clear() { placed_.fill(false); }

bool Fleet::isPlaced(ShipId id) const { return validId(id) && placed_[static_cast<std::size_t>(id)]; }

bool Fleet::isComplete() const {
    for (const bool p : placed_) {
        if (!p) {
            return false;
        }
    }
    return true;
}

const ShipPlacement* Fleet::placementOf(ShipId id) const {
    return isPlaced(id) ? &placements_[static_cast<std::size_t>(id)] : nullptr;
}

std::vector<ShipId> Fleet::missingShips() const {
    std::vector<ShipId> missing;
    for (ShipId id = 0; id < kFleetSize; ++id) {
        if (!placed_[static_cast<std::size_t>(id)]) {
            missing.push_back(id);
        }
    }
    return missing;
}

ShipId Fleet::shipAt(Coord cell) const {
    for (ShipId id = 0; id < kFleetSize; ++id) {
        if (!placed_[static_cast<std::size_t>(id)]) {
            continue;
        }
        for (const Coord c : placements_[static_cast<std::size_t>(id)].cells()) {
            if (c == cell) {
                return id;
            }
        }
    }
    return -1;
}

} // namespace core
} // namespace bs
