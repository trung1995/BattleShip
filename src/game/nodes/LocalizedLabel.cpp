#include "nodes/LocalizedLabel.h"

#include <new>


LocalizedLabel::LocalizedLabel(bs::app::ILocalizer& localizer, Provider provider)
    : localizer_(localizer), provider_(std::move(provider)) {}

LocalizedLabel* LocalizedLabel::create(bs::app::ILocalizer& localizer, Provider provider,
                                       float fontSize, theme::Face face) {
    auto* node = new (std::nothrow) LocalizedLabel(localizer, std::move(provider));
    if (node != nullptr && node->initLabel(fontSize, face)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

LocalizedLabel* LocalizedLabel::forKey(bs::app::ILocalizer& localizer, bs::app::TextKey key,
                                       float fontSize, theme::Face face) {
    bs::app::ILocalizer* source = &localizer;
    return create(localizer, [source, key] { return source->text(key); }, fontSize, face);
}

bool LocalizedLabel::initLabel(float fontSize, theme::Face face) {
    if (!Node::init()) {
        return false;
    }
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setCascadeOpacityEnabled(true);
    label_ = cocos2d::Label::createWithTTF("", theme::fontFile(face), fontSize);
    label_->setTextColor(cocos2d::Color4B(theme::kLight));
    label_->setAlignment(cocos2d::TextHAlignment::CENTER);
    addChild(label_);
    refresh();
    return true;
}

void LocalizedLabel::setProvider(Provider provider) {
    provider_ = std::move(provider);
    refresh();
}

void LocalizedLabel::setMaxWidth(float maxWidth) {
    maxWidth_ = maxWidth;
    refresh();
}

void LocalizedLabel::setTextColor(const cocos2d::Color3B& color) {
    label_->setTextColor(cocos2d::Color4B(color));
}

void LocalizedLabel::setLetterSpacing(float pixels) {
    label_->setAdditionalKerning(pixels);
    refresh();
}

void LocalizedLabel::setShadow(const cocos2d::Color4B& color, const cocos2d::Size& offset) {
    label_->enableShadow(color, offset);
}

void LocalizedLabel::refresh() {
    label_->setString(provider_ ? provider_() : std::string());
    const cocos2d::Size size = label_->getContentSize();
    const float scale = (maxWidth_ > 0.0f && size.width > maxWidth_) ? maxWidth_ / size.width : 1.0f;
    label_->setScale(scale);
    setContentSize(cocos2d::Size(size.width * scale, size.height * scale));
    label_->setPosition(getContentSize() / 2);
}

void LocalizedLabel::onEnter() {
    Node::onEnter();
    subscription_ = localizer_.subscribe([this] { refresh(); });
    refresh();
}

void LocalizedLabel::onExit() {
    localizer_.unsubscribe(subscription_);
    Node::onExit();
}
