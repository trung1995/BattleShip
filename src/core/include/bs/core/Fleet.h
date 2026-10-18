#pragma once

#include <array>
#include <vector>

#include "bs/core/Coord.h"

namespace bs {
namespace core {

using ShipId = int; // 0..9, the slot in fleetSpec()

constexpr int kFleetSize = 10;

// Ship lengths by ShipId: one 4-cell, two 3-cell, three 2-cell, four 1-cell ships (FR-004).
const std::array<int, kFleetSize>& fleetSpec();

enum class PlacementError { None, OutOfBounds, Overlaps, Touches, ShipIdOutOfRange };

// Up to ten ships under the placement rules: on the board, no overlap, no touching on any side
// or corner (FR-005, FR-006). Rules are checked in the order OutOfBounds, Overlaps, Touches.
class Fleet {
public:
    // Checks a placement for ship `id`, ignoring that ship's own current cells (move/rotate).
    // Throws std::invalid_argument if the placement length differs from fleetSpec()[id].
    PlacementError canPlace(ShipId id, const ShipPlacement& placement) const;

    // Places or moves ship `id` if legal; otherwise returns the error and keeps the previous
    // placement.
    PlacementError place(ShipId id, const ShipPlacement& placement);

    void remove(ShipId id);
    void clear();

    bool isPlaced(ShipId id) const;
    bool isComplete() const;
    const ShipPlacement* placementOf(ShipId id) const; // nullptr if not placed
    std::vector<ShipId> missingShips() const;

    // The ship occupying `cell`, or -1.
    ShipId shipAt(Coord cell) const;

private:
    std::array<bool, kFleetSize> placed_{};
    std::array<ShipPlacement, kFleetSize> placements_{};
};

} // namespace core
} // namespace bs
