#pragma once

#include "cocos2d.h"

class GameServices;

class MenuScene final : public cocos2d::Scene {
public:
    static MenuScene* create(GameServices& services, bool openDifficulty = false);
    void onEnter() override;
    void showDifficulty();

private:
    MenuScene(GameServices& services, bool openDifficulty)
        : services_(services), openDifficulty_(openDifficulty) {}
    bool init() override;

    GameServices& services_;
    bool openDifficulty_;
};
