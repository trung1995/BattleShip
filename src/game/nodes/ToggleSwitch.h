#pragma once

#include <functional>

#include "cocos2d.h"

// A brass pill switch: knob right on brass when on, left on dark wood when off. It reports
// clicks; the owner decides the new state and calls setOn.
class ToggleSwitch final : public cocos2d::Node {
public:
    static ToggleSwitch* create(std::function<void()> onClick);
    void setOn(bool on);

private:
    explicit ToggleSwitch(std::function<void()> onClick) : onClick_(std::move(onClick)) {}
    bool initSwitch();
    void redraw();

    std::function<void()> onClick_;
    cocos2d::DrawNode* draw_ = nullptr;
    bool on_ = false;
};
