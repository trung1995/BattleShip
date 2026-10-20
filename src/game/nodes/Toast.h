#pragma once

#include "bs/app/TextKeys.h"
#include "cocos2d.h"
#include "nodes/LocalizedLabel.h"

// A message on a dark plaque near the bottom of the screen. show() flashes a short message that
// fades out, e.g. why a shot or placement was rejected; hold() keeps a hint up until release().
class Toast final : public cocos2d::Node {
public:
    enum class Tone {
        Info,  // tar with a timber edge
        Error, // blood-red edge and glow, tilted a little
        Gold,  // gold edge and text: the Help targeting hint
    };

    static Toast* create(bs::app::ILocalizer& localizer, Tone tone = Tone::Info);
    void show(bs::app::TextKey key);
    void hold(LocalizedLabel::Provider provider, Tone tone);
    void release();
    bool holding() const { return holding_; }

    void onEnter() override;
    void onExit() override;

private:
    Toast(bs::app::ILocalizer& localizer, Tone tone) : localizer_(localizer), tone_(tone), baseTone_(tone) {}
    bool init() override;
    void redraw();

    bs::app::ILocalizer& localizer_;
    Tone tone_;
    Tone baseTone_; // the tone show() uses
    cocos2d::DrawNode* plaque_ = nullptr;
    LocalizedLabel* label_ = nullptr;
    bool holding_ = false;
    bs::app::SubscriptionId subscription_ = 0;
};
