#include "nodes/TextButton.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"

using cocos2d::Color4F;
using cocos2d::Rect;
using cocos2d::Vec2;
using theme::rgba;

namespace {

constexpr float kRadius = 3.0f;
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

Look lookFor(TextButton::Style style, bool hovered, bool pressed) {
    const Color4F none = rgba(0, 0, 0, 0.0f);
    switch (style) {
    case TextButton::Style::Primary:
    case TextButton::Style::Destructive:
        if (pressed) {
            return {theme::kBloodDeep, theme::kBlood, rgba(0xc8, 0x32, 0x1f), none, theme::kCream, none, none,
                    false, false};
        }
        if (hovered) {
            return {theme::kBloodBright, theme::kBloodDeep, theme::kEmberEdge, theme::kBloodShadow, theme::kCream,
                    rgba(224, 64, 46, 0.32f), none};
        }
        return {rgba(0xc8, 0x32, 0x1f), theme::kBloodDeep, rgba(0xe0, 0x57, 0x3f), theme::kBloodShadow,
                theme::kCream, none, none};
    case TextButton::Style::Safe:
    case TextButton::Style::HelpReady:
        if (pressed) {
            return {theme::kGoldDeep, theme::kGoldLight, theme::kGoldEdge, none, theme::kInk, none, none, false,
                    false};
        }
        if (hovered) {
            return {rgba(0xe6, 0xbd, 0x6c), rgba(0xa8, 0x7a, 0x2e), theme::kGoldPale, theme::kGoldLedge,
                    theme::kInk, rgba(224, 176, 78, 0.3f), none};
        }
        return {theme::kGoldLight, theme::kGoldDeep, theme::kGoldEdge, theme::kGoldLedge, theme::kInk, none, none};
    case TextButton::Style::HelpArmed:
        return {theme::kBlood, theme::kBlood, rgba(0xf0, 0xc2, 0x6a), theme::kBloodShadow, theme::kCream,
                rgba(224, 176, 78, 0.4f), none};
    case TextButton::Style::HelpUsed:
        return {theme::kTarSpent, theme::kTarSpent, theme::kTarSpentEdge, none, theme::kSpent, none, none, true,
                false};
    case TextButton::Style::SegmentOn:
        return {theme::kTar, theme::kTar, none, none, theme::rgb(0xf3, 0xe2, 0xc0), none, none, false, false};
    case TextButton::Style::SegmentOff:
        return {hovered ? rgba(30, 20, 16, 0.12f) : none, hovered ? rgba(30, 20, 16, 0.12f) : none, none, none,
                theme::kInk, none, none, false, false};
    case TextButton::Style::Secondary:
        break;
    }
    if (pressed) {
        return {theme::kTarSpent, theme::kTarSpent, theme::kTarEdge, none, theme::kBone, none, none, false, false};
    }
    if (hovered) {
        return {theme::kTarHover, theme::kTarHover, theme::kBlood, theme::kTarLedge, theme::kBoneBright, none,
                rgba(0x4a, 0x30, 0x20)};
    }
    return {theme::kTar, theme::kTar, theme::kTarEdge, theme::kTarLedge, theme::kBone, none, theme::kTarLine};
}

Color4F iconColour(TextButton::Style style) {
    switch (style) {
    case TextButton::Style::HelpArmed: return rgba(0xf0, 0xc2, 0x6a);
    case TextButton::Style::HelpUsed: return rgba(0x7a, 0x6a, 0x50, 0.5f);
    default: return theme::kBloodDark;
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
    const Look look = lookFor(style_, hovered_ && !looksDisabled_, pressed_ && !looksDisabled_);
    setOpacity(looksDisabled_ ? 102 : 255); // 40%, as in the mockup

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
        decor::softShadow(background_, box, radius, Vec2(0.0f, -10.0f), 22.0f, rgba(0, 0, 0, 0.55f));
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
