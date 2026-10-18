#pragma once

#include <array>
#include <vector>

#include "bs/core/Fleet.h"
#include "bs/core/Types.h"

namespace bs {
namespace core {

enum class CellShotState { Untouched, Miss, Hit, Marked };
enum class ShipState { Intact, Damaged, Sunk };

// What a shooter may know about a cell of the opponent's board (data-model §2).
enum class Knowledge { Unknown, Miss, Hit, Sunk, Marked };

struct CellUpdate {
    Coord coord;
    Knowledge now;
};

struct ShotEffect {
    ShotResult result = ShotResult::Miss;
    ShipId sunkShip = -1;
    std::vector<CellUpdate> updates; // the shot cell, a sunk ship's cells, auto-marked cells
};

// One participant's board: the owner's ships plus the shots received.
class Board {
public:
    explicit Board(const Fleet& fleet);

    // Resolves a shot. Precondition: the cell is on the board and not fired upon (otherwise
    // throws std::logic_error). Sinking a ship marks its untouched neighbours as Marked
    // (FR-022), unless `deferMarking` is set; then the caller calls markAround() later.
    ShotEffect shoot(Coord cell, bool deferMarking = false);

    // Marks every untouched side and corner neighbour of `id` as Marked.
    std::vector<CellUpdate> markAround(ShipId id);

    bool isFiredUpon(Coord cell) const;
    CellShotState shotState(Coord cell) const;
    ShipState shipState(ShipId id) const;
    int remainingCells() const;
    Knowledge knowledgeAt(Coord cell) const; // the public projection
    const Fleet& fleet() const { return fleet_; }

private:
    using Grid = std::array<std::array<CellShotState, kBoardSize>, kBoardSize>;

    CellShotState& at(Coord c);
    const CellShotState& at(Coord c) const;

    Fleet fleet_;
    Grid shots_;
    std::array<std::array<ShipId, kBoardSize>, kBoardSize> ships_;
    std::array<int, kFleetSize> hits_{};
};

} // namespace core
} // namespace bs
