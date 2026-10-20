#pragma once

#include <memory>

#include "bs/app/PlacementController.h"
#include "bs/core/GameSession.h"
#include "cocos2d.h"

class BoardNode;
class DockNode;
class GameServices;
class LocalizedLabel;
class ShipNode;
class TextButton;
class Toast;

class PlacementScene final : public cocos2d::Scene {
public:
    static PlacementScene* create(GameServices& services);
    void onEnter() override;

private:
    explicit PlacementScene(GameServices& services) : services_(services) {}
    bool init() override;
    void addMouseAndKeyboard();
    void onMouseDown(const cocos2d::Vec2& point);
    void onMouseMove(const cocos2d::Vec2& point);
    void onMouseUp(const cocos2d::Vec2& point);
    void rotate();
    bs::core::Coord originUnder(bs::core::Coord cursorCell) const;
    void showReason(bs::core::PlacementError reason);
    void refresh();
    void startBattle();
    void back();

    GameServices& services_;
    std::shared_ptr<bs::core::GameSession> session_;
    std::unique_ptr<bs::app::PlacementController> controller_;
    BoardNode* board_ = nullptr;
    DockNode* dock_ = nullptr;
    ShipNode* ghost_ = nullptr;
    cocos2d::DrawNode* reasonMark_ = nullptr;
    LocalizedLabel* reason_ = nullptr;
    LocalizedLabel* missing_ = nullptr; // under Start battle while ships are missing
    TextButton* start_ = nullptr;
    Toast* toast_ = nullptr;
    int grabOffset_ = 0; // which cell of the ship the cursor holds
    cocos2d::Vec2 lastMouse_;
};
