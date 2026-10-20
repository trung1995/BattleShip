#include "nodes/Toast.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

namespace {
constexpr float kMaxTextWidth = 820.0f;
constexpr float kPadX = 24.0f;
constexpr float kPadY = 9.0f;
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
    label_->setMaxWidth(kMaxTextWidth);
    addChild(label_);
    setOpacity(0);
    return true;
}

void Toast::show(bs::app::TextKey key) {
    bs::app::ILocalizer* source = &localizer_;
    label_->setProvider([source, key] { return source->text(key); });
    tone_ = baseTone_;
    holding_ = false;
    redraw();
    stopAllActions();
    setOpacity(255);
    runAction(cocos2d::Sequence::create(cocos2d::DelayTime::create(1.6f),
                                        cocos2d::FadeOut::create(0.4f), nullptr));
}

void Toast::hold(LocalizedLabel::Provider provider, Tone tone) {
    label_->setProvider(std::move(provider));
    tone_ = tone;
    holding_ = true;
    redraw();
    stopAllActions();
    setOpacity(255);
}

void Toast::release() {
    if (holding_) {
        holding_ = false;
        stopAllActions();
        setOpacity(0);
    }
}

void Toast::redraw() {
    plaque_->clear();
    const cocos2d::Size text = label_->getContentSize();
    if (text.width <= 0.0f) {
        return;
    }
    const cocos2d::Rect box(-text.width / 2 - kPadX, -text.height / 2 - kPadY, text.width + kPadX * 2,
                            text.height + kPadY * 2);
    cocos2d::Color4F fill = theme::rgba(0x12, 0x0a, 0x07);
    cocos2d::Color4F edge = theme::kTarEdge;
    cocos2d::Color3B ink = theme::kBone;
    float tilt = 0.0f;
    if (tone_ == Tone::Error) {
        fill = theme::rgba(0x1a, 0x07, 0x05);
        edge = theme::kBlood;
        ink = theme::rgb(0xf3, 0xe2, 0xc0);
        tilt = -0.5f;
        decor::softShadow(plaque_, box, 3.0f, cocos2d::Vec2::ZERO, 24.0f, theme::rgba(179, 36, 27, 0.35f));
    } else if (tone_ == Tone::Gold) {
        edge = theme::kGoldTarnish;
        ink = theme::kGoldText;
    }
    setRotation(tilt);
    decor::softShadow(plaque_, box, 3.0f, cocos2d::Vec2(0.0f, -8.0f), 24.0f, theme::rgba(0, 0, 0, 0.6f));
    const decor::Poly shape = decor::roundedRect(box, 3.0f);
    decor::fill(plaque_, shape, fill);
    decor::stroke(plaque_, shape, 1.0f, edge);
    label_->setTextColor(ink);
}

void Toast::onEnter() {
    Node::onEnter();
    subscription_ = localizer_.subscribe([this] { redraw(); });
}

void Toast::onExit() {
    localizer_.unsubscribe(subscription_);
    Node::onExit();
}
