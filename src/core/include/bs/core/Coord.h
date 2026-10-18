#pragma once

#include <vector>

namespace bs {
namespace core {

constexpr int kBoardSize = 10;

struct Coord {
    int row;
    int col;

    bool isOnBoard() const { return row >= 0 && row < kBoardSize && col >= 0 && col < kBoardSize; }
};

inline bool operator==(Coord a, Coord b) { return a.row == b.row && a.col == b.col; }
inline bool operator!=(Coord a, Coord b) { return !(a == b); }
inline bool operator<(Coord a, Coord b) { return a.row != b.row ? a.row < b.row : a.col < b.col; }

// Up, down, left, right, clipped to the board.
std::vector<Coord> orthogonalNeighbours(Coord c);

// All eight surrounding cells, clipped to the board.
std::vector<Coord> allNeighbours(Coord c);

enum class Orientation { Horizontal, Vertical };

struct ShipPlacement {
    Coord origin; // top-left cell
    int length;
    Orientation orientation;

    // The cells the ship covers, starting at origin. May include off-board cells, so callers can
    // detect an out-of-bounds placement.
    std::vector<Coord> cells() const;
};

} // namespace core
} // namespace bs
