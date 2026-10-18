#include "nodes/ShipNode.h"

#include <new>

#include "nodes/Decor.h"

ShipNode* ShipNode::create(int length, bs::core::Orientation orientation, float cellSize, hull::State state) {
    auto* node = new (std::nothrow) ShipNode(cellSize, state);
    if (node != nullptr && node->init()) {
        node->draw_ = decor::canvas();
        node->addChild(node->draw_);
        node->setShape(length, orientation);
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

void ShipNode::setShape(int length, bs::core::Orientation orientation) {
    length_ = length;
    orientation_ = orientation;
    redraw();
}

void ShipNode::redraw() {
    const bool horizontal = orientation_ == bs::core::Orientation::Horizontal;
    const float w = cellSize_ * static_cast<float>(horizontal ? length_ : 1);
    const float h = cellSize_ * static_cast<float>(horizontal ? 1 : length_);
    setContentSize(cocos2d::Size(w, h));
    draw_->clear();
    hull::draw(draw_, cocos2d::Rect(0.0f, 0.0f, w, h), length_, !horizontal, state_);
}
