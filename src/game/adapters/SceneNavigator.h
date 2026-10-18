#pragma once

#include "bs/app/Ports.h"

class GameServices;

// INavigator over cocos2d::Director scene replacement.
class SceneNavigator final : public bs::app::INavigator {
public:
    explicit SceneNavigator(GameServices& services) : services_(services) {}

    void showMenu() override;
    void showSettings() override;
    void showPlacement(bs::core::Difficulty difficulty) override;
    void showBattle() override;
    void showNewGame() override;

private:
    GameServices& services_;
};
