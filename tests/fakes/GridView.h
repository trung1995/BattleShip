#pragma once

#include <array>

#include "bs/core/GameSession.h"

// An ITargetView backed by a plain grid, for strategy tests.
class GridView final : public bs::core::ITargetView {
public:
    GridView() {
        for (auto& row : cells_) row.fill(bs::core::Knowledge::Unknown);
    }
    bs::core::Knowledge at(bs::core::Coord c) const override {
        return cells_[static_cast<std::size_t>(c.row)][static_cast<std::size_t>(c.col)];
    }
    void set(bs::core::Coord c, bs::core::Knowledge k) {
        cells_[static_cast<std::size_t>(c.row)][static_cast<std::size_t>(c.col)] = k;
    }

private:
    std::array<std::array<bs::core::Knowledge, bs::core::kBoardSize>, bs::core::kBoardSize> cells_;
};
