#include "nodes/TextButton.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

using cocos2d::Color4F;
using cocos2d::Rect;
using cocos2d::Vec2;
using theme::rgba;

namespace {

constexpr float kRadius = 4.0f;
constexpr float kDrop = 3.0f;    // the ledge under a raised button
constexpr float kIcon = 22.0f;   // the Help cross
constexpr float kIconGap = 12.0f;

struct Look {
    Color4F top, bottom, border, ledge;
    cocos2d::Color3B text;
    Color4F glow;       // alpha 0: none
    Color4F topLine;    // inset highlight; alpha 0: none
    bool dashed = false;
    bool ledgeShown = true;
};

bool hasIcon(TextButton::Style style) {
    return style == TextButton::Style::HelpReady || style == TextButton::Style::HelpArmed ||
           style == TextButton::Style::HelpUsed;
}

Look lookFor(TextButton::Style style, bool hovered, bool pressed, bool disabled) {
    const Color4F none = rgba(0, 0, 0, 0.0f);
    switch (style) {
    case TextButton::Style::Primary:
    case TextButton::Style::HelpReady:
        if (disabled) {
            return {rgba(0xcd, 0xb4, 0x8a), rgba(0x9c, 0x84, 0x60), rgba(0xb9, 0xa1, 0x7a), none,
                    theme::rgb(0x4d, 0x3d, 0x28), none, none, false, false};
        }
        if (pressed) {
            return {rgba(0xa9, 0x7a, 0x2e), rgba(0xc9, 0x9a, 0x4c), rgba(0xd6, 0xb0, 0x6a), none,
                    theme::rgb(0x1f, 0x14, 0x0a), none, none, false, false};
        }
        if (hovered) {
            return {rgba(0xe8, 0xbf, 0x6e), rgba(0xb5, 0x86, 0x34), rgba(0xf6, 0xdc, 0x9c), rgba(0x5e, 0x41, 0x16),
                    theme::rgb(0x1f, 0x14, 0x0a), rgba(224, 176, 78, 0.28f), none};
        }
        return {rgba(0xd9, 0xad, 0x5a), rgba(0xa9, 0x7a, 0x2e), rgba(0xf0, 0xcf, 0x86), rgba(0x5e, 0x41, 0x16),
                theme::rgb(0x1f, 0x14, 0x0a), none, none};
    case TextButton::Style::Destructive:
        if (pressed) {
            return {theme::kWaxShadow, theme::kWaxDeep, theme::kWaxEdge, none, theme::kWaxText, none, none,
                    false, false};
        }
        return {hovered ? theme::kWax : theme::kWaxDeep, hovered ? theme::kWax : theme::kWaxDeep,
                theme::kWaxEdge, theme::kWaxShadow, theme::kWaxText,
                hovered ? rgba(192, 80, 58, 0.3f) : none, none};
    case TextButton::Style::HelpArmed:
        return {theme::kWax, theme::kWax, rgba(0xf0, 0xc2, 0x6a), theme::kWaxShadow, theme::kWaxText,
                rgba(224, 176, 78, 0.4f), none};
    case TextButton::Style::HelpUsed:
        return {rgba(0x2b, 0x1b, 0x0f), rgba(0x2b, 0x1b, 0x0f), rgba(0x5a, 0x3e, 0x26), none,
                theme::rgb(0x8a, 0x76, 0x56), none, none, true, false};
    case TextButton::Style::SegmentOn:
        return {rgba(0x3b, 0x26, 0x15), rgba(0x3b, 0x26, 0x15), none, none, theme::kCream, none, none, false, false};
    case TextButton::Style::SegmentOff:
        return {hovered ? rgba(59, 38, 21, 0.14f) : none, hovered ? rgba(59, 38, 21, 0.14f) : none, none, none,
                theme::kInk, none, none, false, false};
    case TextButton::Style::Secondary:
        break;
    }
    if (pressed) {
        return {rgba(0x2b, 0x1b, 0x0f), rgba(0x2b, 0x1b, 0x0f), rgba(0x7a, 0x55, 0x30), none,
                theme::rgb(0xd9, 0xc6, 0x9c), none, none, false, false};
    }
    if (hovered) {
        return {rgba(0x4a, 0x31, 0x1b), rgba(0x4a, 0x31, 0x1b), theme::kBrassDeep, rgba(0x16, 0x0d, 0x06),
                theme::rgb(0xf6, 0xe8, 0xc4), none, rgba(0x6b, 0x4a, 0x2a)};
    }
    return {rgba(0x3b, 0x26, 0x15), rgba(0x3b, 0x26, 0x15), rgba(0x7a, 0x55, 0x30), rgba(0x16, 0x0d, 0x06),
            theme::kLight, none, rgba(0x5c, 0x3d, 0x22)};
}

Color4F iconColour(TextButton::Style style) {
    switch (style) {
    case TextButton::Style::HelpArmed: return rgba(0xf0, 0xc2, 0x6a);
    case TextButton::Style::HelpUsed: return rgba(0x8a, 0x76, 0x56, 0.5f);
    default: return theme::rgba(0x5e, 0x16, 0x0c);
    }
}

} // namespace

TextButton* TextButton::create(bs::app::ILocalizer& localizer, LocalizedLabel::Provider caption,
                               const cocos2d::Size& size, std::function<void()> onClick, Style style,
                               float fontSize) {
    auto* node = new (std::nothrow) TextButton(localizer, std::move(onClick), style);
    if (node != nullptr && node->initButton(localizer, std::move(caption), size, fontSize)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

TextButton* TextButton::forKey(bs::app::ILocalizer& localizer, bs::app::TextKey key,
                               const cocos2d::Size& size, std::function<void()> onClick, Style style,
                               float fontSize) {
    bs::app::ILocalizer* source = &localizer;
    return create(localizer, [source, key] { return source->text(key); }, size, std::move(onClick), style,
                  fontSize);
}

bool TextButton::initButton(bs::app::ILocalizer& localizer, LocalizedLabel::Provider caption,
                            const cocos2d::Size& size, float fontSize) {
    if (!Node::init()) {
        return false;
    }
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setContentSize(size);
    setCascadeOpacityEnabled(true);
    background_ = decor::canvas();
    addChild(background_);
    const bool segment = style_ == Style::SegmentOn || style_ == Style::SegmentOff;
    caption_ = LocalizedLabel::create(localizer, std::move(caption), fontSize,
                                      segment ? theme::Face::Body : theme::Face::Caps);
    addChild(caption_);
    redraw();

    auto touch = cocos2d::EventListenerTouchOneByOne::create();
    touch->setSwallowTouches(true);
    touch->onTouchBegan = [this](cocos2d::Touch* t, cocos2d::Event*) {
        if (!isVisible() || !contains(t->getLocation())) {
            return false;
        }
        pressed_ = true;
        redraw();
        return true;
    };
    touch->onTouchEnded = [this](cocos2d::Touch* t, cocos2d::Event*) {
        pressed_ = false;
        redraw();
        if (contains(t->getLocation()) && onClick_) {
            onClick_();
        }
    };
    touch->onTouchCancelled = [this](cocos2d::Touch*, cocos2d::Event*) {
        pressed_ = false;
        redraw();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

    auto mouse = cocos2d::EventListenerMouse::create();
    mouse->onMouseMove = [this](cocos2d::EventMouse* e) {
        const bool inside = contains(cocos2d::Vec2(e->getCursorX(), e->getCursorY()));
        if (inside != hovered_) {
            hovered_ = inside;
            redraw();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);
    return true;
}

// A new language changes the caption width, which moves the Help icon.
void TextButton::onEnter() {
    Node::onEnter();
    subscription_ = localizer_.subscribe([this] { redraw(); });
    redraw();
}

void TextButton::onExit() {
    localizer_.unsubscribe(subscription_);
    Node::onExit();
}

void TextButton::setStyle(Style style) {
    if (style != style_) {
        style_ = style;
        redraw();
    }
}

void TextButton::setLooksDisabled(bool disabled) {
    if (disabled != looksDisabled_) {
        looksDisabled_ = disabled;
        redraw();
    }
}

void TextButton::redraw() {
    background_->clear();
    const cocos2d::Size size = getContentSize();
    const bool primaryLike = style_ == Style::Primary || style_ == Style::HelpReady;
    const Look look = lookFor(style_, hovered_ && !looksDisabled_, pressed_ && !looksDisabled_,
                              looksDisabled_ && primaryLike);
    setOpacity(looksDisabled_ ? 140 : 255);

    const float shift = (pressed_ && !looksDisabled_ && look.ledge.a == 0.0f && style_ != Style::SegmentOn &&
                         style_ != Style::SegmentOff)
                            ? -kDrop
                            : 0.0f;
    const Rect box(0.0f, shift, size.width, size.height);
    const bool segment = style_ == Style::SegmentOn || style_ == Style::SegmentOff;
    const float radius = segment ? 0.0f : kRadius;

    if (look.glow.a > 0.0f) {
        decor::softShadow(background_, box, radius, Vec2::ZERO, 26.0f, look.glow);
        decor::fill(background_, decor::roundedRect(Rect(box.getMinX() - 3.0f, box.getMinY() - 3.0f,
                                                         box.size.width + 6.0f, box.size.height + 6.0f),
                                                    radius + 3.0f),
                    Color4F(look.glow.r, look.glow.g, look.glow.b, look.glow.a * 0.9f));
    }
    if (look.ledgeShown && look.ledge.a > 0.0f) {
        decor::softShadow(background_, box, radius, Vec2(0.0f, -6.0f), 14.0f, rgba(0, 0, 0, 0.4f));
        decor::fill(background_, decor::roundedRect(Rect(0.0f, -kDrop, size.width, size.height), radius),
                    look.ledge);
    }
    const decor::Poly shape = decor::roundedRect(box, radius);
    if (look.top.a > 0.0f) {
        decor::gradient(background_, shape, look.top, look.bottom);
    }
    if (look.topLine.a > 0.0f) {
        background_->drawSolidRect(Vec2(2.0f, box.getMaxY() - 2.0f), Vec2(size.width - 2.0f, box.getMaxY() - 1.0f),
                                   look.topLine);
    }
    if (look.dashed) {
        decor::dashedRect(background_, Rect(0.5f, shift + 0.5f, size.width - 1.0f, size.height - 1.0f), 5.0f, 3.0f,
                          1.0f, look.border);
    } else if (look.border.a > 0.0f) {
        decor::stroke(background_, shape, 1.0f, look.border);
    }

    caption_->setTextColor(look.text);
    float captionX = size.width / 2;
    float maxWidth = size.width - 24.0f;
    if (hasIcon(style_)) {
        maxWidth -= kIcon + kIconGap;
        const float total = std::min(caption_->getContentSize().width, maxWidth) + kIcon + kIconGap;
        const float iconX = (size.width - total) / 2 + kIcon / 2;
        const Vec2 centre(iconX, shift + size.height / 2);
        const Color4F ink = iconColour(style_);
        background_->drawSolidRect(centre + Vec2(-3.0f, -kIcon / 2), centre + Vec2(3.0f, kIcon / 2), ink);
        background_->drawSolidRect(centre + Vec2(-kIcon / 2, -3.0f), centre + Vec2(kIcon / 2, 3.0f), ink);
        captionX = iconX + kIcon / 2 + kIconGap + std::min(caption_->getContentSize().width, maxWidth) / 2;
    }
    caption_->setMaxWidth(maxWidth);
    caption_->setPosition(captionX, shift + size.height / 2 + 1.0f);
}

bool TextButton::contains(const cocos2d::Vec2& worldPoint) const {
    const cocos2d::Vec2 local = convertToNodeSpace(worldPoint);
    return cocos2d::Rect(cocos2d::Vec2::ZERO, getContentSize()).containsPoint(local);
}
