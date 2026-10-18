#pragma once

#include <vector>

#include "bs/core/AutoPlacer.h"
#include "bs/core/Fleet.h"

namespace bs {
namespace app {

struct Preview {
    std::vector<bs::core::Coord> cells; // may include off-board cells
    bool legal = true;
    bs::core::PlacementError reason = bs::core::PlacementError::None;
};

enum class DropStatus { Placed, Rejected, ReturnedToDock };

struct DropResult {
    DropStatus status = DropStatus::Rejected;
    bs::core::PlacementError reason = bs::core::PlacementError::None;
};

enum class AutoResult { Ok, CannotFit };

// Placement decisions for the placement screen. The scene maps mouse positions to cells and
// renders what this returns. `fleet` is always session.playerFleet().
class PlacementController {
public:
    PlacementController(bs::core::Fleet& fleet, const bs::core::IFleetPlacer& placer,
                        bs::core::IRandom& rng);

    // Drag and drop (FR-007, FR-008, FR-013)
    void beginDragFromDock(bs::core::ShipId id);
    void beginDragPlaced(bs::core::ShipId id);
    bool isDragging() const { return dragging_; }
    bs::core::ShipId draggedShip() const { return dragShip_; }
    bs::core::Orientation dragOrientation() const { return dragOrientation_; }
    Preview hover(bs::core::Coord cell);
    // During a drag: flips the preview and returns None. Otherwise rotates the selected ship in
    // place if legal, and returns the reason if not.
    bs::core::PlacementError rotate();
    DropResult drop(bs::core::Coord cell);
    void dropOnDock(); // removes a placed ship (US2 scenario 7)
    void cancelDrag();
    void select(bs::core::ShipId id);
    bs::core::ShipId selectedShip() const; // a placed ship, or -1

    // Automatic placement (FR-009, FR-010, FR-012)
    void autoPlaceAll();
    AutoResult autoPlaceRemaining();

    bool canStartBattle() const; // FR-014
    std::vector<bs::core::ShipId> dockShips() const;

private:
    bs::core::ShipPlacement placementAt(bs::core::Coord cell) const;

    bs::core::Fleet& fleet_;
    const bs::core::IFleetPlacer& placer_;
    bs::core::IRandom& rng_;
    bool dragging_ = false;
    bs::core::ShipId dragShip_ = -1;
    bs::core::Orientation dragOrientation_ = bs::core::Orientation::Horizontal;
    bs::core::ShipId selected_ = -1;
};

} // namespace app
} // namespace bs
