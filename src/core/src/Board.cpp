#include "bs/core/Board.h"

#include <stdexcept>

namespace bs {
namespace core {

namespace {
std::size_t idx(int v) { return static_cast<std::size_t>(v); }
} // namespace

Board::Board(const Fleet& fleet) : fleet_(fleet) {
    for (auto& row : shots_) row.fill(CellShotState::Untouched);
    for (auto& row : ships_) row.fill(-1);
    for (ShipId id = 0; id < kFleetSize; ++id) {
        if (const ShipPlacement* p = fleet_.placementOf(id)) {
            for (const Coord c : p->cells()) {
                ships_[idx(c.row)][idx(c.col)] = id;
            }
        }
    }
}

CellShotState& Board::at(Coord c) { return shots_[idx(c.row)][idx(c.col)]; }
const CellShotState& Board::at(Coord c) const { return shots_[idx(c.row)][idx(c.col)]; }

ShotEffect Board::shoot(Coord cell, bool deferMarking) {
    if (!cell.isOnBoard() || isFiredUpon(cell)) {
        throw std::logic_error("Board::shoot on an off-board or already fired cell");
    }
    ShotEffect effect;
    const ShipId id = ships_[idx(cell.row)][idx(cell.col)];
    if (id < 0) {
        at(cell) = CellShotState::Miss;
        effect.result = ShotResult::Miss;
        effect.updates.push_back({cell, Knowledge::Miss});
        return effect;
    }

    at(cell) = CellShotState::Hit;
    ++hits_[idx(id)];
    if (shipState(id) != ShipState::Sunk) {
        effect.result = ShotResult::Hit;
        effect.updates.push_back({cell, Knowledge::Hit});
        return effect;
    }

    effect.result = ShotResult::Sunk;
    effect.sunkShip = id;
    for (const Coord c : fleet_.placementOf(id)->cells()) {
        effect.updates.push_back({c, Knowledge::Sunk});
    }
    if (!deferMarking) {
        const std::vector<CellUpdate> marked = markAround(id);
        effect.updates.insert(effect.updates.end(), marked.begin(), marked.end());
    }
    return effect;
}

std::vector<CellUpdate> Board::markAround(ShipId id) {
    std::vector<CellUpdate> updates;
    const ShipPlacement* p = fleet_.placementOf(id);
    if (p == nullptr) {
        return updates;
    }
    for (const Coord shipCell : p->cells()) {
        for (const Coord n : allNeighbours(shipCell)) {
            if (at(n) == CellShotState::Untouched && ships_[idx(n.row)][idx(n.col)] != id) {
                at(n) = CellShotState::Marked;
                updates.push_back({n, Knowledge::Marked});
            }
        }
    }
    return updates;
}

bool Board::isFiredUpon(Coord cell) const { return at(cell) != CellShotState::Untouched; }

CellShotState Board::shotState(Coord cell) const { return at(cell); }

ShipState Board::shipState(ShipId id) const {
    const ShipPlacement* p = fleet_.placementOf(id);
    const int hits = hits_[idx(id)];
    if (p == nullptr || hits == 0) {
        return ShipState::Intact;
    }
    return hits >= p->length ? ShipState::Sunk : ShipState::Damaged;
}

int Board::remainingCells() const {
    int remaining = 0;
    for (ShipId id = 0; id < kFleetSize; ++id) {
        if (const ShipPlacement* p = fleet_.placementOf(id)) {
            remaining += p->length - hits_[idx(id)];
        }
    }
    return remaining;
}

Knowledge Board::knowledgeAt(Coord cell) const {
    switch (at(cell)) {
    case CellShotState::Untouched:
        return Knowledge::Unknown;
    case CellShotState::Miss:
        return Knowledge::Miss;
    case CellShotState::Marked:
        return Knowledge::Marked;
    case CellShotState::Hit:
        break;
    }
    const ShipId id = ships_[idx(cell.row)][idx(cell.col)];
    return shipState(id) == ShipState::Sunk ? Knowledge::Sunk : Knowledge::Hit;
}

} // namespace core
} // namespace bs
