#include "nodes/Banner.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

namespace {
constexpr float kPadX = 44.0f;
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
    label_ = LocalizedLabel::create(localizer_, provider_, 29.0f, theme::Face::CapsHeavy);
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
    label_->refresh(); // the plaque is sized to the text it is about to show
    plaque_->clear();
    if (shown_.empty()) {
        return;
    }
    const cocos2d::Size text = label_->getContentSize();
    const cocos2d::Rect box(-text.width / 2 - kPadX, -text.height / 2 - kPadY, text.width + kPadX * 2,
                            text.height + kPadY * 2);
    struct Look {
        cocos2d::Color4F fill, edge, glow;
        cocos2d::Color3B text;
        float tilt; // degrees, clockwise
    };
    Look look{theme::kParchment, theme::kGoldTarnish, theme::rgba(0, 0, 0, 0.0f), theme::kInk, -1.5f};
    switch (style_) {
    case Style::Parchment:
        break;
    case Style::Gold:
        look = {theme::kGoldPip, theme::kGoldPale, theme::rgba(224, 176, 78, 0.35f), theme::kInk, -1.5f};
        break;
    case Style::Wax:
        look = {theme::kBloodDark, theme::kBlood, theme::rgba(0, 0, 0, 0.0f), theme::kCream, 1.2f};
        break;
    case Style::Blood:
        look = {theme::kBloodDeep, theme::kEmber, theme::rgba(224, 64, 46, 0.55f), theme::kCream, 1.2f};
        break;
    }
    setRotation(look.tilt);
    decor::softShadow(plaque_, box, 2.0f, cocos2d::Vec2(0.0f, -10.0f), 24.0f, theme::rgba(0, 0, 0, 0.6f));
    if (look.glow.a > 0.0f) {
        decor::softShadow(plaque_, box, 2.0f, cocos2d::Vec2::ZERO, 44.0f, look.glow);
        decor::fill(plaque_, decor::roundedRect(cocos2d::Rect(box.getMinX() - 4.0f, box.getMinY() - 4.0f,
                                                              box.size.width + 8.0f, box.size.height + 8.0f),
                                                4.0f),
                    cocos2d::Color4F(look.glow.r, look.glow.g, look.glow.b, look.glow.a * 0.55f));
    }
    const decor::Poly shape = decor::roundedRect(box, 1.0f);
    decor::fill(plaque_, shape, look.fill);
    if (style_ == Style::Parchment) {
        decor::innerShadow(plaque_, box, 12.0f, theme::rgba(60, 22, 6, 0.35f));
    }
    decor::stroke(plaque_, shape, 2.0f, look.edge);
    label_->setTextColor(look.text);
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
