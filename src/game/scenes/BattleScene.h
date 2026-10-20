#pragma once

#include <memory>

#include "bs/app/BattleController.h"
#include "bs/core/GameSession.h"
#include "cocos2d.h"

class Banner;
class BoardNode;
class HelpPreviewNode;
class TextButton;
class GameServices;
class LocalizedLabel;
class Toast;

class BattleScene final : public cocos2d::Scene {
public:
    static BattleScene* create(GameServices& services);
    void onEnter() override;
    void onExitTransitionDidStart() override;
    void update(float dt) override;

private:
    explicit BattleScene(GameServices& services) : services_(services) {}
    bool init() override;
    void addInput();
    void onTargetClicked(const cocos2d::Vec2& worldPoint);
    void onHelpButton();
    void cancelHelp();
    void updateHelpPreview(const cocos2d::Vec2& worldPoint);
    void updateTargetHover(const cocos2d::Vec2& worldPoint);
    void requestExit();
    void renderChanges(const std::vector<bs::core::CellChange>& changes);
    void showGameOverOnce();

    GameServices& services_;
    std::shared_ptr<bs::core::GameSession> session_;
    std::unique_ptr<bs::app::BattleController> controller_;
    BoardNode* ownBoard_ = nullptr;
    BoardNode* targetBoard_ = nullptr;
    Banner* banner_ = nullptr;
    LocalizedLabel* helpHint_ = nullptr;
    LocalizedLabel* computerHelp_ = nullptr; // Hard only (FR-032)
    TextButton* helpButton_ = nullptr;
    HelpPreviewNode* helpPreview_ = nullptr;
    Toast* toast_ = nullptr;
    bool gameOverShown_ = false;
    bool exitDialogOpen_ = false;
    cocos2d::Vec2 lastMouse_;
};
