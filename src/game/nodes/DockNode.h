#pragma once

#include <vector>

#include "bs/app/Services.h"
#include "bs/core/Fleet.h"
#include "cocos2d.h"

class LocalizedLabel;
class ShipNode;

// The unplaced ships on a parchment sheet: a heading, then one slot per length (4, 3 / 2, 1)
// with a "×N" count. It reports which ship a click picks; the placement controller decides what
// happens. The node's origin is the sheet's bottom-left corner, frame included.
class DockNode final : public cocos2d::Node {
public:
    static DockNode* create(bs::app::ILocalizer& localizer, float cellSize);

    void setShips(const std::vector<bs::core::ShipId>& unplaced);
    // The first unplaced ship in the slot under `worldPoint`, or -1.
    bs::core::ShipId shipAt(const cocos2d::Vec2& worldPoint) const;
    bool contains(const cocos2d::Vec2& worldPoint) const;

private:
    DockNode(bs::app::ILocalizer& localizer, float cellSize) : localizer_(localizer), cellSize_(cellSize) {}
    bool initDock();
    cocos2d::Rect slotRect(int slot) const;

    static constexpr int kSlots = 4; // lengths 4, 3, 2, 1
    bs::app::ILocalizer& localizer_;
    float cellSize_;
    std::vector<bs::core::ShipId> unplaced_;
    ShipNode* shapes_[kSlots] = {};
    LocalizedLabel* counts_[kSlots] = {};
    int remaining_[kSlots] = {};
};
