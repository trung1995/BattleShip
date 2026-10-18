#include "nodes/ToggleSwitch.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

namespace {
constexpr float kWidth = 68.0f;
constexpr float kHeight = 34.0f;
constexpr float kKnob = 13.0f;
} // namespace

ToggleSwitch* ToggleSwitch::create(std::function<void()> onClick) {
    auto* node = new (std::nothrow) ToggleSwitch(std::move(onClick));
    if (node != nullptr && node->initSwitch()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool ToggleSwitch::initSwitch() {
    if (!Node::init()) {
        return false;
    }
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setContentSize(cocos2d::Size(kWidth, kHeight));
    draw_ = decor::canvas();
    addChild(draw_);
    redraw();

    auto touch = cocos2d::EventListenerTouchOneByOne::create();
    touch->setSwallowTouches(true);
    const auto inside = [this](cocos2d::Touch* t) {
        return cocos2d::Rect(-6.0f, -6.0f, kWidth + 12.0f, kHeight + 12.0f).containsPoint(convertToNodeSpace(t->getLocation()));
    };
    touch->onTouchBegan = [inside](cocos2d::Touch* t, cocos2d::Event*) { return inside(t); };
    touch->onTouchEnded = [this, inside](cocos2d::Touch* t, cocos2d::Event*) {
        if (inside(t) && onClick_) onClick_();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);
    return true;
}

void ToggleSwitch::setOn(bool on) {
    if (on != on_) {
        on_ = on;
        redraw();
    }
}

void ToggleSwitch::redraw() {
    draw_->clear();
    const decor::Poly pill = decor::roundedRect(cocos2d::Rect(0.0f, 0.0f, kWidth, kHeight), kHeight / 2);
    decor::fill(draw_, pill, on_ ? theme::kBrassDeep : theme::rgba(0x5a, 0x3e, 0x26));
    decor::fill(draw_, decor::roundedRect(cocos2d::Rect(2.0f, kHeight - 7.0f, kWidth - 4.0f, 5.0f), 2.5f),
                on_ ? theme::rgba(60, 35, 10, 0.25f) : theme::rgba(0, 0, 0, 0.3f)); // inset shade
    const cocos2d::Color4F edge = on_ ? theme::rgba(0x8a, 0x6a, 0x2a) : theme::kHullEdge;
    decor::stroke(draw_, pill, 1.0f, edge);
    const cocos2d::Vec2 knob(on_ ? kWidth - 3.0f - kKnob : 3.0f + kKnob, kHeight / 2);
    if (on_) {
        draw_->drawSolidCircle(knob + cocos2d::Vec2(0.0f, -2.0f), kKnob, 0.0f, 24, theme::rgba(0, 0, 0, 0.3f));
    }
    draw_->drawSolidCircle(knob, kKnob + 1.0f, 0.0f, 24, edge);
    draw_->drawSolidCircle(knob, kKnob, 0.0f, 24, on_ ? theme::rgba(0xf3, 0xe2, 0xb8) : theme::rgba(0xcd, 0xb9, 0x8e));
}
