#include "scenes/BattleScene.h"

#include <new>
#include <string>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/Banner.h"
#include "nodes/BoardNode.h"
#include "nodes/Decor.h"
#include "nodes/HelpPreviewNode.h"
#include "nodes/TextButton.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/Toast.h"
#include "scenes/ConfirmDialog.h"
#include "scenes/GameOverLayer.h"

using bs::app::TextKey;
using bs::core::Side;

BattleScene* BattleScene::create(GameServices& services) {
    auto* scene = new (std::nothrow) BattleScene(services);
    if (scene != nullptr && scene->init()) {
        scene->autorelease();
        return scene;
    }
    delete scene;
    return nullptr;
}

bool BattleScene::init() {
    if (!Scene::init()) {
        return false;
    }
    session_ = services_.currentSession;
    controller_ = std::make_unique<bs::app::BattleController>(
        *session_, bs::core::makeStrategy(session_->difficulty(), services_.random()), services_.random(),
        services_.audio());

    auto backdrop = decor::canvas();
    decor::table(backdrop, cocos2d::Size(1280, 720));
    addChild(backdrop);
    auto& loc = services_.localizer();
    GameServices* services = &services_;
    bs::app::BattleController* controller = controller_.get();

    banner_ = Banner::create(loc, [services, controller] {
        const bs::app::TurnIndicator turn = controller->turnIndicator();
        if (turn.gameOver) return std::string();
        if (turn.side == Side::Player) return services->localizer().text(TextKey::BattleTurnPlayer);
        return services->localizer().format(
            TextKey::BattleTurnComputer,
            {{"name", services->localizer().displayName(Side::Computer, services->settings())}});
    });
    banner_->setPosition(640, 669);
    addChild(banner_, 2);

    const float boardSide = theme::kCellSize * bs::core::kBoardSize;
    const float frameWidth = boardSide + BoardNode::kFrameLeft + BoardNode::kFrameRight;
    const struct {
        Side owner;
        float x; // the grid's left edge
        TextKey caption;
    } boards[] = {{Side::Player, 116.0f, TextKey::BattleBoardOwn},
                  {Side::Computer, 819.0f, TextKey::BattleBoardTarget}};
    for (const auto& b : boards) {
        auto board = BoardNode::create(theme::kCellSize);
        board->setPosition(b.x, 142.0f);
        board->setKnowledge(session_->viewOf(b.owner));
        addChild(board);

        const Side owner = b.owner;
        const float centreX = b.x - BoardNode::kFrameLeft + frameWidth / 2;
        // On Hard the computer's remaining Help uses sit beside its name at the frame's right
        // edge, and the name centres in the rest of the line (FR-032).
        const bool withHelpCounter = owner == Side::Computer && controller->showComputerHelpCounter();
        const float counterWidth = 120.0f;
        const float nameWidth = withHelpCounter ? frameWidth - counterWidth : frameWidth;
        auto name = LocalizedLabel::create(
            loc, [services, owner] { return services->localizer().displayName(owner, services->settings()); },
            27.0f, theme::Face::Caps);
        name->setMaxWidth(nameWidth);
        name->setPosition(centreX - frameWidth / 2 + nameWidth / 2, 592.0f);
        addChild(name);

        if (withHelpCounter) {
            computerHelp_ = LocalizedLabel::create(
                loc,
                [services, controller] {
                    return services->localizer().format(
                        TextKey::BattleComputerHelp, {{"count", std::to_string(controller->computerHelpUsesLeft())}});
                },
                18.0f, theme::Face::Italic);
            computerHelp_->setTextColor(theme::kGold);
            computerHelp_->setMaxWidth(counterWidth);
            computerHelp_->setPosition(centreX + frameWidth / 2 - counterWidth / 2, 592.0f);
            addChild(computerHelp_);
        }

        auto caption = LocalizedLabel::forKey(loc, b.caption, 18.0f, theme::Face::Italic);
        caption->setTextColor(theme::kMuted);
        caption->setMaxWidth(frameWidth);
        caption->setPosition(centreX, 564.0f);
        addChild(caption);

        if (owner == Side::Player) {
            board->setFleet(&session_->ownFleet(Side::Player));
            ownBoard_ = board;
        } else {
            targetBoard_ = board; // never given the computer's fleet (FR-016)
        }
    }

    helpPreview_ = HelpPreviewNode::create(*targetBoard_);
    helpPreview_->setVisible(false);
    targetBoard_->addChild(helpPreview_, 5);

    helpButton_ = TextButton::create(
        loc,
        [services, controller] {
            if (!controller->playerHelpAvailable()) return services->localizer().text(TextKey::BattleHelpUsed);
            return services->localizer().format(TextKey::BattleHelp,
                                                {{"count", std::to_string(controller->playerHelpUsesLeft())}});
        },
        cocos2d::Size(214, 76), [this] { onHelpButton(); }, TextButton::Style::HelpReady, 22.0f);
    helpButton_->setPosition(640, 332);
    addChild(helpButton_);

    helpHint_ = LocalizedLabel::forKey(loc, TextKey::BattleHelpHint, 19.0f, theme::Face::Italic);
    helpHint_->setTextColor(theme::kGold);
    helpHint_->setMaxWidth(960.0f);
    helpHint_->setPosition(640, 94);
    helpHint_->setVisible(false);
    addChild(helpHint_);

    toast_ = Toast::create(loc, Toast::Tone::Info);
    toast_->setPosition(640, 36);
    addChild(toast_, 20);

    auto menuButton = TextButton::forKey(loc, TextKey::BattleMenu, cocos2d::Size(140, 46), [this] { requestExit(); },
                                         TextButton::Style::Secondary, 20.0f);
    menuButton->setPosition(1178, 669);
    addChild(menuButton);

    addInput();
    scheduleUpdate();
    return true;
}

void BattleScene::addInput() {
    auto touch = cocos2d::EventListenerTouchOneByOne::create();
    touch->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    touch->onTouchEnded = [this](cocos2d::Touch* t, cocos2d::Event*) { onTargetClicked(t->getLocation()); };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

    auto mouse = cocos2d::EventListenerMouse::create();
    mouse->onMouseMove = [this](cocos2d::EventMouse* e) {
        lastMouse_ = cocos2d::Vec2(e->getCursorX(), e->getCursorY());
        updateHelpPreview(lastMouse_);
        updateTargetHover(lastMouse_);
    };
    mouse->onMouseDown = [this](cocos2d::EventMouse* e) {
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT) cancelHelp();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    auto keys = cocos2d::EventListenerKeyboard::create();
    keys->onKeyReleased = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event*) {
        if (code != cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE || exitDialogOpen_) {
            return;
        }
        if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting) {
            cancelHelp();
        } else {
            requestExit();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keys, this);
}

// FR-044: leaving an in-progress battle asks first; the battle is frozen while it asks.
void BattleScene::requestExit() {
    services_.audio().onClick();
    if (controller_->mode() == bs::app::Mode::GameOver) {
        services_.navigator().showMenu(); // no confirmation once the game is over
        return;
    }
    if (exitDialogOpen_) {
        return;
    }
    controller_->requestExit();
    helpPreview_->hide();
    exitDialogOpen_ = true;
    const ConfirmDialog::Spec spec{TextKey::ExitTitle, TextKey::ExitMessage, TextKey::ExitStay, TextKey::ExitLeave};
    addChild(ConfirmDialog::create(
                 services_.localizer(), spec,
                 [this] {
                     exitDialogOpen_ = false;
                     controller_->cancelExit();
                 },
                 [this] {
                     exitDialogOpen_ = false;
                     if (controller_->confirmExit()) services_.navigator().showMenu();
                 }),
             200);
}

void BattleScene::onHelpButton() {
    services_.audio().onClick();
    if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting) {
        cancelHelp();
        return;
    }
    controller_->armHelp();
    helpHint_->setVisible(controller_->mode() == bs::app::Mode::PlayerHelpTargeting);
}

void BattleScene::cancelHelp() {
    controller_->disarmHelp();
    helpPreview_->hide();
    helpHint_->setVisible(false);
}

void BattleScene::updateHelpPreview(const cocos2d::Vec2& worldPoint) {
    bs::core::Coord cell{0, 0};
    if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting && targetBoard_->cellAt(worldPoint, cell)) {
        helpPreview_->show(cell, session_->viewOf(Side::Computer));
    } else {
        helpPreview_->hide();
    }
}

// A red ring follows the cursor over enemy waters while it is the player's turn to fire.
void BattleScene::updateTargetHover(const cocos2d::Vec2& worldPoint) {
    bs::core::Coord cell{0, 0};
    if (controller_->mode() == bs::app::Mode::AwaitingPlayer && targetBoard_->cellAt(worldPoint, cell)) {
        targetBoard_->setHover(cell);
    } else {
        targetBoard_->clearHover();
    }
}

void BattleScene::onEnter() {
    Scene::onEnter();
    services_.audio().enterScreen(bs::app::Screen::Battle);
}

void BattleScene::onExitTransitionDidStart() {
    unscheduleUpdate();
    Scene::onExitTransitionDidStart();
}

void BattleScene::onTargetClicked(const cocos2d::Vec2& worldPoint) {
    bs::core::Coord cell{0, 0};
    if (gameOverShown_ || !targetBoard_->cellAt(worldPoint, cell)) {
        return;
    }
    const bool helpTargeting = controller_->mode() == bs::app::Mode::PlayerHelpTargeting;
    const bs::app::ActionFeedback feedback =
        helpTargeting ? controller_->onPlayerHelp(cell) : controller_->onPlayerFire(cell);
    if (feedback.hasMessage) {
        toast_->show(feedback.message);
    }
    if (helpTargeting && feedback.status == bs::core::ActionStatus::Applied) {
        helpPreview_->hide();
        helpHint_->setVisible(false);
    }
    renderChanges(feedback.changes);
}

void BattleScene::update(float dt) { renderChanges(controller_->tick(static_cast<double>(dt))); }

// Every change list, including a whole Help cross with its marking, is applied in one pass on
// one frame (FR-048, SC-009).
void BattleScene::renderChanges(const std::vector<bs::core::CellChange>& changes) {
    if (!changes.empty()) {
        ownBoard_->applyChanges(changes, Side::Player);
        targetBoard_->applyChanges(changes, Side::Computer);
    }
    const bs::app::TurnIndicator turn = controller_->turnIndicator();
    banner_->setStyle(turn.side == Side::Player ? Banner::Style::Parchment : Banner::Style::Wax);
    banner_->refresh();
    helpButton_->caption()->refresh();
    if (computerHelp_ != nullptr) {
        computerHelp_->refresh();
    }
    if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting) {
        helpButton_->setStyle(TextButton::Style::HelpArmed);
    } else {
        helpButton_->setStyle(controller_->playerHelpAvailable() ? TextButton::Style::HelpReady
                                                                 : TextButton::Style::HelpUsed); // FR-032
    }
    updateTargetHover(lastMouse_);
    showGameOverOnce();
}

void BattleScene::showGameOverOnce() {
    if (gameOverShown_ || controller_->mode() != bs::app::Mode::GameOver) {
        return;
    }
    gameOverShown_ = true;
    addChild(GameOverLayer::create(services_, session_->winner()), 100);
}
