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
    // What the bar at the bottom of the screen is holding up.
    enum class Hint { None, ExtraShot, HelpTargeting, ComputerHelp };

    explicit BattleScene(GameServices& services) : services_(services) {}
    bool init() override;
    void addBoardsAndNames();
    void addHelpPanel();
    void addComputerCounter();
    void addInput();
    void onTargetClicked(const cocos2d::Vec2& worldPoint);
    void onHelpButton();
    void cancelHelp();
    void updateHelpPreview(const cocos2d::Vec2& worldPoint);
    void updateTargetHover(const cocos2d::Vec2& worldPoint);
    void requestExit();
    void renderChanges(const std::vector<bs::core::CellChange>& changes);
    void refreshHelpPanel();
    void refreshComputerCounter(bool burning);
    void refreshHint();
    void layoutNameLines();
    void showGameOverOnce();

    GameServices& services_;
    std::shared_ptr<bs::core::GameSession> session_;
    std::unique_ptr<bs::app::BattleController> controller_;
    cocos2d::DrawNode* bloodSky_ = nullptr; // replaces the storm while the computer's Help lands
    BoardNode* ownBoard_ = nullptr;
    BoardNode* targetBoard_ = nullptr;
    Banner* banner_ = nullptr;
    LocalizedLabel* ownName_ = nullptr;
    LocalizedLabel* ownCaption_ = nullptr;
    LocalizedLabel* enemyName_ = nullptr;
    LocalizedLabel* enemyCaption_ = nullptr; // Easy and Medium
    cocos2d::Node* computerCounter_ = nullptr; // Hard only (FR-032)
    cocos2d::DrawNode* computerCounterPlate_ = nullptr;
    LocalizedLabel* computerCounterLabel_ = nullptr;
    cocos2d::Label* computerCounterDigit_ = nullptr;
    cocos2d::Node* helpPanel_ = nullptr;
    TextButton* helpButton_ = nullptr;
    cocos2d::DrawNode* helpPips_ = nullptr;
    LocalizedLabel* helpLeft_ = nullptr;
    LocalizedLabel* helpCancel_ = nullptr;
    cocos2d::DrawNode* helpCancelLine_ = nullptr;
    HelpPreviewNode* helpPreview_ = nullptr;
    Toast* hintBar_ = nullptr;
    Toast* toast_ = nullptr;
    Hint hint_ = Hint::None;
    int shownCounter_ = -1;
    bool shownCounterBurning_ = false;
    int shownPips_ = -1;
    bool shownArmed_ = false;
    bool gameOverShown_ = false;
    bool exitDialogOpen_ = false;
    cocos2d::Vec2 lastMouse_;
};
