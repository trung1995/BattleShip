#pragma once

#include "bs/core/Coord.h"
#include "cocos2d.h"
#include "nodes/Hull.h"

// The visual of one ship of a given length and orientation. Pure drawing.
class ShipNode final : public cocos2d::Node {
public:
    static ShipNode* create(int length, bs::core::Orientation orientation, float cellSize,
                            hull::State state = hull::State::Intact);
    void setShape(int length, bs::core::Orientation orientation);

private:
    ShipNode(float cellSize, hull::State state) : cellSize_(cellSize), state_(state) {}
    void redraw();

    float cellSize_;
    hull::State state_;
    int length_ = 1;
    bs::core::Orientation orientation_ = bs::core::Orientation::Horizontal;
    cocos2d::DrawNode* draw_ = nullptr;
};
