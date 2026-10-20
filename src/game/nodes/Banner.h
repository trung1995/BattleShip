#pragma once

#include <string>

#include "cocos2d.h"
#include "nodes/LocalizedLabel.h"

// The turn banner: a plaque sized to its localized text, tilted a little (FR-024). Parchment
// for the player's turn, glowing gold for an extra shot after a hit, blood-red wax while the
// computer fires, and a burning red when the computer unleashes Help.
class Banner final : public cocos2d::Node {
public:
    enum class Style { Parchment, Gold, Wax, Blood };

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
