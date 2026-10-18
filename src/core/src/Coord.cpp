#include "bs/core/Coord.h"

namespace bs {
namespace core {

std::vector<Coord> orthogonalNeighbours(Coord c) {
    std::vector<Coord> result;
    const Coord candidates[] = {{c.row - 1, c.col}, {c.row + 1, c.col}, {c.row, c.col - 1},
                                {c.row, c.col + 1}};
    for (const Coord n : candidates) {
        if (n.isOnBoard()) {
            result.push_back(n);
        }
    }
    return result;
}

std::vector<Coord> allNeighbours(Coord c) {
    std::vector<Coord> result;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            const Coord n{c.row + dr, c.col + dc};
            if ((dr != 0 || dc != 0) && n.isOnBoard()) {
                result.push_back(n);
            }
        }
    }
    return result;
}

std::vector<Coord> ShipPlacement::cells() const {
    std::vector<Coord> result;
    for (int i = 0; i < length; ++i) {
        result.push_back(orientation == Orientation::Horizontal ? Coord{origin.row, origin.col + i}
                                                                : Coord{origin.row + i, origin.col});
    }
    return result;
}

} // namespace core
} // namespace bs
