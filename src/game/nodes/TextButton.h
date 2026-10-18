#pragma once

#include <functional>

#include "cocos2d.h"
#include "nodes/LocalizedLabel.h"

// A button with a localized caption, drawn in one of the chart-table styles. Clicks arrive as
// touches on desktop; hover and press states follow the mouse.
class TextButton final : public cocos2d::Node {
public:
    enum class Style {
        Primary,     // brass: the main action on a screen
        Secondary,   // dark wood
        Destructive, // red sealing wax
        HelpReady,   // brass with a cross icon
        HelpArmed,   // red wax with a glowing brass cross: choosing the Help target
        HelpUsed,    // tarred and dashed: Help already spent
        SegmentOn,   // the chosen option in a segmented control
        SegmentOff,
    };

    static TextButton* create(bs::app::ILocalizer& localizer, LocalizedLabel::Provider caption,
                              const cocos2d::Size& size, std::function<void()> onClick,
                              Style style = Style::Secondary, float fontSize = 22.0f);
    static TextButton* forKey(bs::app::ILocalizer& localizer, bs::app::TextKey key,
                              const cocos2d::Size& size, std::function<void()> onClick,
                              Style style = Style::Secondary, float fontSize = 22.0f);

    void setStyle(Style style);
    // Disabled-looking buttons still report clicks, so a screen can explain why (FR-014).
    void setLooksDisabled(bool disabled);
    LocalizedLabel* caption() { return caption_; }

    void onEnter() override;
    void onExit() override;

private:
    TextButton(bs::app::ILocalizer& localizer, std::function<void()> onClick, Style style)
        : localizer_(localizer), onClick_(std::move(onClick)), style_(style) {}
    bool initButton(bs::app::ILocalizer& localizer, LocalizedLabel::Provider caption,
                    const cocos2d::Size& size, float fontSize);
    void redraw();
    bool contains(const cocos2d::Vec2& worldPoint) const;

    bs::app::ILocalizer& localizer_;
    std::function<void()> onClick_;
    Style style_;
    bs::app::SubscriptionId subscription_ = 0;
    cocos2d::DrawNode* background_ = nullptr;
    LocalizedLabel* caption_ = nullptr;
    bool hovered_ = false;
    bool pressed_ = false;
    bool looksDisabled_ = false;
};
