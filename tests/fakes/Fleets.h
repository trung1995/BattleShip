#pragma once

#include "bs/core/Fleet.h"

// A fixed legal fleet used by many tests:
//   row 0: 4-cell at cols 0-3, 3-cell at cols 5-7
//   row 2: 3-cell at cols 0-2, 2-cell at cols 4-5, 2-cell at cols 7-8
//   row 4: 2-cell at cols 0-1, 1-cells at cols 3, 5, 7, 9
inline bs::core::Fleet knownFleet() {
    using namespace bs::core;
    const ShipPlacement layout[kFleetSize] = {
        {{0, 0}, 4, Orientation::Horizontal}, {{0, 5}, 3, Orientation::Horizontal},
        {{2, 0}, 3, Orientation::Horizontal}, {{2, 4}, 2, Orientation::Horizontal},
        {{2, 7}, 2, Orientation::Horizontal}, {{4, 0}, 2, Orientation::Horizontal},
        {{4, 3}, 1, Orientation::Horizontal}, {{4, 5}, 1, Orientation::Horizontal},
        {{4, 7}, 1, Orientation::Horizontal}, {{4, 9}, 1, Orientation::Horizontal}};
    Fleet fleet;
    for (ShipId id = 0; id < kFleetSize; ++id) {
        fleet.place(id, layout[id]);
    }
    return fleet;
}

// True if every placed ship obeys the placement rules against the others and the fleet is
// complete with the exact composition.
inline bool isLegalCompleteFleet(const bs::core::Fleet& fleet) {
    using namespace bs::core;
    if (!fleet.isComplete()) {
        return false;
    }
    for (ShipId id = 0; id < kFleetSize; ++id) {
        const ShipPlacement* p = fleet.placementOf(id);
        if (p == nullptr || p->length != fleetSpec()[static_cast<std::size_t>(id)] ||
            fleet.canPlace(id, *p) != PlacementError::None) {
            return false;
        }
    }
    return true;
}

// Nine ships (every ship except the 4-cell one, ShipId 0) placed so that no legal position is
// left for the 4-cell ship. AutoPlacerTest proves this exhaustively.
inline bs::core::Fleet fleetWithNoRoomForTheFourShip() {
    using namespace bs::core;
    Fleet fleet;
    fleet.place(1, {{4, 8}, 3, Orientation::Vertical});
    fleet.place(2, {{9, 1}, 3, Orientation::Horizontal});
    fleet.place(3, {{7, 5}, 2, Orientation::Vertical});
    fleet.place(4, {{5, 3}, 2, Orientation::Vertical});
    fleet.place(5, {{4, 5}, 2, Orientation::Vertical});
    fleet.place(6, {{3, 1}, 1, Orientation::Horizontal});
    fleet.place(7, {{8, 7}, 1, Orientation::Horizontal});
    fleet.place(8, {{1, 5}, 1, Orientation::Horizontal});
    fleet.place(9, {{0, 2}, 1, Orientation::Horizontal});
    return fleet;
}
