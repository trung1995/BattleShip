#pragma once

#include <functional>
#include <memory>

#include "bs/app/Services.h"
#include "bs/app/SettingsController.h"
#include "cocos2d.h"

namespace cocos2d {
namespace ui {
class TextField;
}
} // namespace cocos2d

class GameServices;
class LocalizedLabel;
class TextButton;
class ToggleSwitch;

class SettingsScene final : public cocos2d::Scene {
public:
    static SettingsScene* create(GameServices& services);
    void onEnter() override;
    void onExit() override;

private:
    explicit SettingsScene(GameServices& services) : services_(services) {}
    bool init() override;
    void addRowLabel(bs::app::TextKey key, float y);
    ToggleSwitch* addToggleRow(bs::app::TextKey key, float y, bool (bs::app::ISettings::*isOn)() const,
                               std::function<void()> onClick, LocalizedLabel*& stateLabel);
    void addNameField(float y);
    void drawNameField();
    void commitName();
    void refresh();
    void back();

    GameServices& services_;
    cocos2d::Node* sheet_ = nullptr; // the tilted parchment; every control sits on it
    std::unique_ptr<bs::app::SettingsController> controller_;
    ToggleSwitch* sound_ = nullptr;
    ToggleSwitch* music_ = nullptr;
    LocalizedLabel* soundState_ = nullptr; // "On" / "Off" beside each switch
    LocalizedLabel* musicState_ = nullptr;
    TextButton* languageButtons_[4] = {};
    cocos2d::ui::TextField* name_ = nullptr;
    cocos2d::DrawNode* nameFrame_ = nullptr;
    cocos2d::Label* nameCount_ = nullptr;
    bool nameFocused_ = false;
    bs::app::SubscriptionId subscription_ = 0;
};
