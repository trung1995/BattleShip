#include "nodes/Toast.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

namespace {
constexpr float kMaxTextWidth = 720.0f;
constexpr float kPadX = 24.0f;
constexpr float kPadY = 10.0f;
} // namespace

Toast* Toast::create(bs::app::ILocalizer& localizer, Tone tone) {
    auto* node = new (std::nothrow) Toast(localizer, tone);
    if (node != nullptr && node->init()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool Toast::init() {
    if (!Node::init()) {
        return false;
    }
    setCascadeOpacityEnabled(true);
    plaque_ = decor::canvas();
    addChild(plaque_);
    label_ = LocalizedLabel::create(localizer_, nullptr, 19.0f);
    label_->setTextColor(theme::kCream);
    label_->setMaxWidth(kMaxTextWidth);
    addChild(label_);
    setOpacity(0);
    return true;
}

void Toast::show(bs::app::TextKey key) {
    bs::app::ILocalizer* source = &localizer_;
    label_->setProvider([source, key] { return source->text(key); });
    redraw();
    stopAllActions();
    setOpacity(255);
    runAction(cocos2d::Sequence::create(cocos2d::DelayTime::create(1.6f),
                                        cocos2d::FadeOut::create(0.4f), nullptr));
}

void Toast::redraw() {
    plaque_->clear();
    const cocos2d::Size text = label_->getContentSize();
    if (text.width <= 0.0f) {
        return;
    }
    const cocos2d::Rect box(-text.width / 2 - kPadX, -text.height / 2 - kPadY, text.width + kPadX * 2,
                            text.height + kPadY * 2);
    decor::softShadow(plaque_, box, 4.0f, cocos2d::Vec2(0.0f, -8.0f), 24.0f, theme::rgba(0, 0, 0, 0.5f));
    const decor::Poly shape = decor::roundedRect(box, 4.0f);
    decor::fill(plaque_, shape, theme::rgba(0x2a, 0x1a, 0x10));
    decor::stroke(plaque_, shape, 1.0f, tone_ == Tone::Error ? theme::kWax : theme::rgba(0x7a, 0x55, 0x30));
}

void Toast::onEnter() {
    Node::onEnter();
    subscription_ = localizer_.subscribe([this] { redraw(); });
}

void Toast::onExit() {
    localizer_.unsubscribe(subscription_);
    Node::onExit();
}
