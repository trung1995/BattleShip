#pragma once

#include "bs/app/TextKeys.h"
#include "cocos2d.h"
#include "nodes/LocalizedLabel.h"

// A short message on a dark plaque that fades out, e.g. why a shot or placement was rejected.
class Toast final : public cocos2d::Node {
public:
    enum class Tone { Info, Error }; // Error gets a red wax border

    static Toast* create(bs::app::ILocalizer& localizer, Tone tone = Tone::Info);
    void show(bs::app::TextKey key);

    void onEnter() override;
    void onExit() override;

private:
    Toast(bs::app::ILocalizer& localizer, Tone tone) : localizer_(localizer), tone_(tone) {}
    bool init() override;
    void redraw();

    bs::app::ILocalizer& localizer_;
    Tone tone_;
    cocos2d::DrawNode* plaque_ = nullptr;
    LocalizedLabel* label_ = nullptr;
    bs::app::SubscriptionId subscription_ = 0;
};
