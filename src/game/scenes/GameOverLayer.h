#pragma once

#include "bs/core/Types.h"
#include "cocos2d.h"

class GameServices;

// Modal overlay over the final battle state: the winner and the ways out (FR-045, FR-046).
class GameOverLayer final : public cocos2d::LayerColor {
public:
    static GameOverLayer* create(GameServices& services, bs::core::Side winner);

private:
    GameOverLayer(GameServices& services, bs::core::Side winner)
        : services_(services), winner_(winner) {}
    bool init() override;

    GameServices& services_;
    bs::core::Side winner_;
};
