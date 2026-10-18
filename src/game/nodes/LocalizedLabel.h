#pragma once

#include <functional>
#include <string>

#include "bs/app/Services.h"
#include "bs/app/TextKeys.h"
#include "Theme.h"
#include "cocos2d.h"

// A TTF label whose text comes from a provider and is re-applied whenever the language changes
// (FR-040). Text wider than maxWidth is scaled down so the longest translation never clips.
class LocalizedLabel final : public cocos2d::Node {
public:
    using Provider = std::function<std::string()>;

    static LocalizedLabel* create(bs::app::ILocalizer& localizer, Provider provider,
                                  float fontSize, theme::Face face = theme::Face::Body);
    static LocalizedLabel* forKey(bs::app::ILocalizer& localizer, bs::app::TextKey key,
                                  float fontSize, theme::Face face = theme::Face::Body);

    void setProvider(Provider provider);
    void setMaxWidth(float maxWidth);
    void setTextColor(const cocos2d::Color3B& color);
    void setLetterSpacing(float pixels);
    void setShadow(const cocos2d::Color4B& color, const cocos2d::Size& offset);
    void refresh();

    void onEnter() override;
    void onExit() override;

private:
    LocalizedLabel(bs::app::ILocalizer& localizer, Provider provider);
    bool initLabel(float fontSize, theme::Face face);

    bs::app::ILocalizer& localizer_;
    Provider provider_;
    cocos2d::Label* label_ = nullptr;
    float maxWidth_ = 0.0f;
    bs::app::SubscriptionId subscription_ = 0;
};
