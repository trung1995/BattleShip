#pragma once

#include "cocos2d.h"

class GameServices;

// Modal over the menu: pick Easy, Medium or Hard before a new game (FR-001).
class DifficultyOverlay final : public cocos2d::LayerColor {
public:
    static DifficultyOverlay* create(GameServices& services);

private:
    explicit DifficultyOverlay(GameServices& services) : services_(services) {}
    bool init() override;

    GameServices& services_;
};
