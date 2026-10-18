#include "bs/app/PlacementController.h"

namespace bs {
namespace app {

using bs::core::Orientation;
using bs::core::PlacementError;

PlacementController::PlacementController(bs::core::Fleet& fleet,
                                         const bs::core::IFleetPlacer& placer,
                                         bs::core::IRandom& rng)
    : fleet_(fleet), placer_(placer), rng_(rng) {}

void PlacementController::beginDragFromDock(bs::core::ShipId id) {
    dragging_ = true;
    dragShip_ = id;
    dragOrientation_ = Orientation::Horizontal;
}

void PlacementController::beginDragPlaced(bs::core::ShipId id) {
    const bs::core::ShipPlacement* current = fleet_.placementOf(id);
    dragging_ = true;
    dragShip_ = id;
    dragOrientation_ = current != nullptr ? current->orientation : Orientation::Horizontal;
    selected_ = id;
}

bs::core::ShipPlacement PlacementController::placementAt(bs::core::Coord cell) const {
    return {cell, bs::core::fleetSpec()[static_cast<std::size_t>(dragShip_)], dragOrientation_};
}

Preview PlacementController::hover(bs::core::Coord cell) {
    Preview preview;
    if (!dragging_) {
        return preview;
    }
    const bs::core::ShipPlacement placement = placementAt(cell);
    preview.cells = placement.cells();
    preview.reason = fleet_.canPlace(dragShip_, placement);
    preview.legal = preview.reason == PlacementError::None;
    return preview;
}

PlacementError PlacementController::rotate() {
    if (dragging_) {
        dragOrientation_ = dragOrientation_ == Orientation::Horizontal ? Orientation::Vertical
                                                                       : Orientation::Horizontal;
        return PlacementError::None;
    }
    const bs::core::ShipPlacement* current = fleet_.placementOf(selected_);
    if (current == nullptr) {
        return PlacementError::None;
    }
    bs::core::ShipPlacement rotated = *current;
    rotated.orientation = current->orientation == Orientation::Horizontal ? Orientation::Vertical
                                                                          : Orientation::Horizontal;
    return fleet_.place(selected_, rotated); // keeps the old position if illegal
}

DropResult PlacementController::drop(bs::core::Coord cell) {
    DropResult result;
    if (!dragging_) {
        return result;
    }
    result.reason = fleet_.place(dragShip_, placementAt(cell));
    result.status = result.reason == PlacementError::None ? DropStatus::Placed : DropStatus::Rejected;
    if (result.status == DropStatus::Placed) {
        selected_ = dragShip_;
    }
    dragging_ = false;
    return result;
}

void PlacementController::dropOnDock() {
    if (dragging_) {
        fleet_.remove(dragShip_);
        if (selected_ == dragShip_) selected_ = -1;
        dragging_ = false;
    }
}

void PlacementController::cancelDrag() { dragging_ = false; }

void PlacementController::select(bs::core::ShipId id) { selected_ = id; }

bs::core::ShipId PlacementController::selectedShip() const {
    return selected_ >= 0 && fleet_.isPlaced(selected_) ? selected_ : -1;
}

void PlacementController::autoPlaceAll() { fleet_ = placer_.placeAll(rng_).fleet; }

AutoResult PlacementController::autoPlaceRemaining() {
    const bs::core::AutoPlaceResult result = placer_.placeRemaining(fleet_, rng_);
    if (result.status != bs::core::AutoPlaceStatus::Ok) {
        return AutoResult::CannotFit; // layout unchanged (FR-012)
    }
    fleet_ = result.fleet;
    return AutoResult::Ok;
}

bool PlacementController::canStartBattle() const { return fleet_.isComplete(); }

std::vector<bs::core::ShipId> PlacementController::dockShips() const { return fleet_.missingShips(); }

} // namespace app
} // namespace bs
