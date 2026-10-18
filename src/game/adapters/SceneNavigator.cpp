#include "adapters/SceneNavigator.h"

#include "GameServices.h"
#include "cocos2d.h"
#include "scenes/BattleScene.h"
#include "scenes/MenuScene.h"
#include "scenes/PlacementScene.h"
#include "scenes/SettingsScene.h"

namespace {
void replace(cocos2d::Scene* scene) {
    if (scene == nullptr) {
        return;
    }
    auto director = cocos2d::Director::getInstance();
    if (director->getRunningScene() == nullptr) {
        director->runWithScene(scene);
    } else {
        director->replaceScene(cocos2d::TransitionFade::create(0.2f, scene));
    }
}
} // namespace

void SceneNavigator::showMenu() {
    services_.currentSession.reset(); // scenes still fading out keep their own reference
    replace(MenuScene::create(services_));
}

void SceneNavigator::showNewGame() {
    services_.currentSession.reset();
    replace(MenuScene::create(services_, true));
}

void SceneNavigator::showSettings() { replace(SettingsScene::create(services_)); }

void SceneNavigator::showPlacement(bs::core::Difficulty difficulty) {
    services_.currentDifficulty = difficulty;
    replace(PlacementScene::create(services_));
}

void SceneNavigator::showBattle() {
    if (services_.currentSession) {
        replace(BattleScene::create(services_));
    }
}
