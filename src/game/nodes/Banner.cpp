#include "nodes/Banner.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

namespace {
constexpr float kPadX = 40.0f;
constexpr float kPadY = 8.0f;
} // namespace

Banner* Banner::create(bs::app::ILocalizer& localizer, LocalizedLabel::Provider provider) {
    auto* node = new (std::nothrow) Banner(localizer);
    if (node != nullptr && node->initBanner(std::move(provider))) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool Banner::initBanner(LocalizedLabel::Provider provider) {
    if (!Node::init()) {
        return false;
    }
    provider_ = std::move(provider);
    plaque_ = decor::canvas();
    addChild(plaque_);
    label_ = LocalizedLabel::create(localizer_, provider_, 30.0f, theme::Face::CapsHeavy);
    label_->setMaxWidth(900.0f);
    addChild(label_);
    redraw();
    return true;
}

void Banner::setStyle(Style style) {
    if (style != style_) {
        style_ = style;
        redraw();
    }
}

void Banner::refresh() {
    const std::string text = provider_ ? provider_() : std::string();
    if (text != shown_) {
        label_->refresh();
        redraw();
    }
}

void Banner::redraw() {
    shown_ = provider_ ? provider_() : std::string();
    plaque_->clear();
    if (shown_.empty()) {
        return;
    }
    const cocos2d::Size text = label_->getContentSize();
    const cocos2d::Rect box(-text.width / 2 - kPadX, -text.height / 2 - kPadY, text.width + kPadX * 2,
                            text.height + kPadY * 2);
    const decor::Poly shape = decor::roundedRect(box, 3.0f);
    decor::softShadow(plaque_, box, 3.0f, cocos2d::Vec2(0.0f, -10.0f), 24.0f, theme::rgba(0, 0, 0, 0.5f));
    if (style_ == Style::Parchment) {
        decor::fill(plaque_, decor::roundedRect(cocos2d::Rect(box.getMinX() - 5.0f, box.getMinY() - 5.0f,
                                                              box.size.width + 10.0f, box.size.height + 10.0f),
                                                7.0f),
                    theme::rgba(224, 176, 78, 0.22f));
        decor::fill(plaque_, shape, theme::kParchment);
        decor::innerShadow(plaque_, box, 14.0f, theme::rgba(120, 80, 30, 0.35f));
        decor::stroke(plaque_, shape, 2.0f, theme::kBrassDeep);
        label_->setTextColor(theme::kInk);
    } else {
        decor::fill(plaque_, shape, theme::rgba(0x7e, 0x24, 0x16));
        decor::stroke(plaque_, shape, 2.0f, theme::kWaxEdge);
        label_->setTextColor(theme::kWaxText);
    }
}

void Banner::onEnter() {
    Node::onEnter();
    subscription_ = localizer_.subscribe([this] { redraw(); });
    redraw();
}

void Banner::onExit() {
    localizer_.unsubscribe(subscription_);
    Node::onExit();
}
