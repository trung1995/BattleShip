#pragma once

#include <string>

#include "cocos2d.h"
#include "nodes/LocalizedLabel.h"

// The turn banner: a plaque sized to its localized text. Parchment with a brass edge for the
// player's turn, red wax for the computer's (FR-024).
class Banner final : public cocos2d::Node {
public:
    enum class Style { Parchment, Wax };

    static Banner* create(bs::app::ILocalizer& localizer, LocalizedLabel::Provider provider);

    void setStyle(Style style);
    void refresh(); // cheap when nothing changed; safe to call every frame

    void onEnter() override;
    void onExit() override;

private:
    explicit Banner(bs::app::ILocalizer& localizer) : localizer_(localizer) {}
    bool initBanner(LocalizedLabel::Provider provider);
    void redraw();

    bs::app::ILocalizer& localizer_;
    LocalizedLabel::Provider provider_;
    cocos2d::DrawNode* plaque_ = nullptr;
    LocalizedLabel* label_ = nullptr;
    Style style_ = Style::Parchment;
    std::string shown_;
    bs::app::SubscriptionId subscription_ = 0;
};
