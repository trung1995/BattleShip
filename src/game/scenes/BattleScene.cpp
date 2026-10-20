#include "scenes/BattleScene.h"

#include <algorithm>
#include <new>
#include <string>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/Banner.h"
#include "nodes/BoardNode.h"
#include "nodes/Decor.h"
#include "nodes/HelpPreviewNode.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"
#include "nodes/Toast.h"
#include "scenes/ConfirmDialog.h"
#include "scenes/GameOverLayer.h"

using bs::app::TextKey;
using bs::core::Side;
using cocos2d::Vec2;

namespace {
// Layout at 1280 x 720 (redesign v2): two boards, a Help column between them, the turn banner on
// top and a hint bar at the bottom.
constexpr float kOwnGridX = 116.0f;
constexpr float kTargetGridX = 819.0f;
constexpr float kGridY = 162.0f;
constexpr float kNameLineY = 597.0f;
constexpr float kNameGap = 13.0f;
constexpr float kHelpX = 640.0f;
constexpr float kPipSize = 14.0f;
constexpr float kPipGap = 8.0f;
constexpr float kCounterPip = 11.0f;
constexpr float kCounterPipGap = 6.0f;
constexpr GLubyte kDimmed = 140; // enemy waters while the computer's Help lands

float frameCentreX(float gridX) {
    const float frameWidth = theme::kCellSize * bs::core::kBoardSize + BoardNode::kFrameLeft + BoardNode::kFrameRight;
    return gridX - BoardNode::kFrameLeft + frameWidth / 2;
}
} // namespace

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
    decor::storm(backdrop, cocos2d::Size(1280, 720), Vec2(0.3f, 1.0f), theme::kStormLight, theme::kStormDeep);
    addChild(backdrop);
    bloodSky_ = decor::canvas();
    decor::storm(bloodSky_, cocos2d::Size(1280, 720), Vec2(0.2f, 0.5f), theme::kBloodSky, theme::kStormDeep, 0.6f);
    bloodSky_->setVisible(false);
    addChild(bloodSky_);

    auto& loc = services_.localizer();
    GameServices* services = &services_;
    bs::app::BattleController* controller = controller_.get();

    // FR-024: whose turn it is, and whether it is an extra shot or the computer's Help.
    banner_ = Banner::create(loc, [services, controller] {
        const bs::app::TurnIndicator turn = controller->turnIndicator();
        bs::app::ILocalizer& text = services->localizer();
        const std::string computer = text.displayName(Side::Computer, services->settings());
        if (turn.gameOver) return std::string();
        if (turn.computerHelp) return text.format(TextKey::BattleTurnComputerHelp, {{"name", computer}});
        if (turn.side == Side::Computer) return text.format(TextKey::BattleTurnComputer, {{"name", computer}});
        return text.text(turn.extraShot ? TextKey::BattleTurnExtra : TextKey::BattleTurnPlayer);
    });
    banner_->setPosition(640, 671);
    addChild(banner_, 2);

    auto menuButton = TextButton::forKey(loc, TextKey::BattleMenu, cocos2d::Size(120, 46), [this] { requestExit(); },
                                         TextButton::Style::Secondary, 20.0f);
    menuButton->setPosition(92, 669);
    addChild(menuButton);

    addBoardsAndNames();
    if (controller_->showComputerHelpCounter()) {
        addComputerCounter();
    }
    addHelpPanel();

    hintBar_ = Toast::create(loc, Toast::Tone::Info);
    hintBar_->setPosition(640, 43);
    addChild(hintBar_, 20);
    toast_ = Toast::create(loc, Toast::Tone::Info);
    toast_->setPosition(640, 98);
    addChild(toast_, 21);

    addInput();
    renderChanges({});
    scheduleUpdate();
    return true;
}

void BattleScene::addBoardsAndNames() {
    auto& loc = services_.localizer();
    GameServices* services = &services_;
    const struct {
        Side owner;
        float x;
    } boards[] = {{Side::Player, kOwnGridX}, {Side::Computer, kTargetGridX}};
    for (const auto& b : boards) {
        auto board = BoardNode::create(theme::kCellSize);
        board->setPosition(b.x, kGridY);
        board->setKnowledge(session_->viewOf(b.owner));
        board->setCascadeOpacityEnabled(true);
        addChild(board);
        if (b.owner == Side::Player) {
            board->setFleet(&session_->ownFleet(Side::Player));
            ownBoard_ = board;
        } else {
            targetBoard_ = board; // never given the computer's fleet (FR-016)
        }

        const Side owner = b.owner;
        auto name = LocalizedLabel::create(
            loc, [services, owner] { return services->localizer().displayName(owner, services->settings()); },
            26.0f, theme::Face::Caps);
        name->setTextColor(theme::kBone);
        name->setMaxWidth(250.0f);
        addChild(name);
        (owner == Side::Player ? ownName_ : enemyName_) = name;
    }

    ownCaption_ = LocalizedLabel::forKey(loc, TextKey::BattleBoardOwn, 18.0f, theme::Face::Italic);
    ownCaption_->setTextColor(theme::kMuted);
    ownCaption_->setMaxWidth(150.0f);
    addChild(ownCaption_);
    if (!controller_->showComputerHelpCounter()) {
        enemyCaption_ = LocalizedLabel::forKey(loc, TextKey::BattleBoardTarget, 18.0f, theme::Face::Italic);
        enemyCaption_->setTextColor(theme::kMuted);
        enemyCaption_->setMaxWidth(150.0f);
        addChild(enemyCaption_);
    }

    helpPreview_ = HelpPreviewNode::create(*targetBoard_);
    helpPreview_->setVisible(false);
    targetBoard_->addChild(helpPreview_, 5);
}

// The Help column between the boards: the button, one gold pip per use left, "n of 3 left", and
// a Cancel link while choosing the centre (FR-032).
void BattleScene::addHelpPanel() {
    auto& loc = services_.localizer();
    GameServices* services = &services_;
    bs::app::BattleController* controller = controller_.get();

    helpPanel_ = cocos2d::Node::create();
    helpPanel_->setCascadeOpacityEnabled(true);
    addChild(helpPanel_, 3);

    helpButton_ = TextButton::create(
        loc,
        [services, controller] {
            return services->localizer().text(controller->playerHelpAvailable() ? TextKey::BattleHelp
                                                                                : TextKey::BattleHelpUsed);
        },
        cocos2d::Size(238, 70), [this] { onHelpButton(); }, TextButton::Style::HelpReady, 23.0f);
    helpButton_->setPosition(kHelpX, 395);
    helpPanel_->addChild(helpButton_);

    helpPips_ = decor::canvas();
    helpPips_->setPosition(kHelpX, 343);
    helpPanel_->addChild(helpPips_);

    helpLeft_ = LocalizedLabel::create(
        loc,
        [services, controller] {
            return services->localizer().format(TextKey::BattleHelpLeft,
                                                {{"n", std::to_string(controller->playerHelpUsesLeft())}});
        },
        18.0f);
    helpLeft_->setTextColor(theme::kBone);
    helpLeft_->setMaxWidth(238.0f);
    helpLeft_->setPosition(kHelpX, 318);
    helpPanel_->addChild(helpLeft_);

    helpCancel_ = LocalizedLabel::forKey(loc, TextKey::BattleHelpCancel, 17.0f);
    helpCancel_->setTextColor(theme::kMuted);
    helpCancel_->setMaxWidth(238.0f);
    helpCancel_->setPosition(kHelpX, 287);
    helpPanel_->addChild(helpCancel_);
    helpCancelLine_ = decor::canvas();
    helpPanel_->addChild(helpCancelLine_);
}

// Hard only: the computer's Help uses beside its name, as red pips and a number (FR-032).
void BattleScene::addComputerCounter() {
    auto& loc = services_.localizer();
    computerCounter_ = cocos2d::Node::create();
    addChild(computerCounter_);
    computerCounterPlate_ = decor::canvas();
    computerCounter_->addChild(computerCounterPlate_);
    computerCounterLabel_ = LocalizedLabel::forKey(loc, TextKey::BattleHelp, 15.0f);
    computerCounterLabel_->setTextColor(theme::kEmberText);
    computerCounterLabel_->setMaxWidth(110.0f);
    computerCounterLabel_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    computerCounter_->addChild(computerCounterLabel_);
    computerCounterDigit_ = cocos2d::Label::createWithTTF("", theme::fontFile(theme::Face::Body), 15.0f);
    computerCounterDigit_->setTextColor(cocos2d::Color4B(theme::kBone));
    computerCounterDigit_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    computerCounter_->addChild(computerCounterDigit_);
}

void BattleScene::addInput() {
    auto touch = cocos2d::EventListenerTouchOneByOne::create();
    touch->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    touch->onTouchEnded = [this](cocos2d::Touch* t, cocos2d::Event*) {
        const Vec2 local = helpCancel_->convertToNodeSpace(t->getLocation());
        const cocos2d::Rect cancelArea(-8.0f, -6.0f, helpCancel_->getContentSize().width + 16.0f,
                                       helpCancel_->getContentSize().height + 12.0f);
        if (helpCancel_->isVisible() && cancelArea.containsPoint(local)) {
            services_.audio().onClick();
            cancelHelp();
            renderChanges({});
            return;
        }
        onTargetClicked(t->getLocation());
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

    auto mouse = cocos2d::EventListenerMouse::create();
    mouse->onMouseMove = [this](cocos2d::EventMouse* e) {
        lastMouse_ = Vec2(e->getCursorX(), e->getCursorY());
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
    } else {
        controller_->armHelp();
        updateHelpPreview(lastMouse_);
    }
    renderChanges({});
}

void BattleScene::cancelHelp() {
    controller_->disarmHelp();
    helpPreview_->hide();
}

void BattleScene::updateHelpPreview(const Vec2& worldPoint) {
    bs::core::Coord cell{0, 0};
    if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting && targetBoard_->cellAt(worldPoint, cell)) {
        helpPreview_->show(cell, session_->viewOf(Side::Computer));
    } else {
        helpPreview_->hide();
    }
}

// A red target frame follows the cursor over enemy waters while it is the player's turn to fire.
void BattleScene::updateTargetHover(const Vec2& worldPoint) {
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

void BattleScene::onTargetClicked(const Vec2& worldPoint) {
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
        targetBoard_->strike(feedback.helpCells);
    }
    renderChanges(feedback.changes);
}

void BattleScene::update(float dt) {
    const int helpBefore = controller_->computerHelpUsesLeft();
    const std::vector<bs::core::CellChange> changes = controller_->tick(static_cast<double>(dt));
    if (controller_->computerHelpUsesLeft() < helpBefore) {
        ownBoard_->strike(controller_->computerHelpReport().cells); // the whole cross flares at once (FR-048)
    }
    renderChanges(changes);
}

// Every change list, including a whole Help cross with its marking, is applied in one pass on
// one frame (FR-048, SC-009).
void BattleScene::renderChanges(const std::vector<bs::core::CellChange>& changes) {
    if (!changes.empty()) {
        ownBoard_->applyChanges(changes, Side::Player);
        targetBoard_->applyChanges(changes, Side::Computer);
    }
    const bs::app::TurnIndicator turn = controller_->turnIndicator();
    Banner::Style style = Banner::Style::Parchment;
    if (turn.computerHelp) {
        style = Banner::Style::Blood;
    } else if (turn.side == Side::Computer) {
        style = Banner::Style::Wax;
    } else if (turn.extraShot) {
        style = Banner::Style::Gold;
    }
    banner_->setStyle(style);
    banner_->refresh();

    // While the computer's Help lands, the sky burns and the player's own board is the focus.
    bloodSky_->setVisible(turn.computerHelp);
    targetBoard_->setOpacity(turn.computerHelp ? kDimmed : 255);
    helpPanel_->setOpacity(turn.computerHelp ? 115 : 255);

    refreshHelpPanel();
    if (computerCounter_ != nullptr) {
        refreshComputerCounter(turn.computerHelp);
    }
    layoutNameLines();
    refreshHint();
    updateTargetHover(lastMouse_);
    showGameOverOnce();
}

void BattleScene::refreshHelpPanel() {
    const bool armed = controller_->mode() == bs::app::Mode::PlayerHelpTargeting;
    helpButton_->caption()->refresh();
    if (armed) {
        helpButton_->setStyle(TextButton::Style::HelpArmed);
    } else {
        helpButton_->setStyle(controller_->playerHelpAvailable() ? TextButton::Style::HelpReady
                                                                 : TextButton::Style::HelpUsed); // FR-032
    }
    helpButton_->setRotation(armed ? -1.0f : 0.0f);
    helpLeft_->refresh();
    helpCancel_->setVisible(armed);
    helpCancelLine_->clear();
    if (armed) {
        const cocos2d::Size size = helpCancel_->getContentSize();
        helpCancelLine_->drawSolidRect(Vec2(kHelpX - size.width / 2, 275.5f), Vec2(kHelpX + size.width / 2, 276.5f),
                                       theme::rgba(0xa8, 0x98, 0x7a));
    }

    const int left = controller_->playerHelpUsesLeft();
    if (left == shownPips_ && armed == shownArmed_) {
        return;
    }
    shownPips_ = left;
    shownArmed_ = armed;
    helpPips_->clear();
    if (armed && left > 0) { // the use about to be spent glows
        const float span = kPipSize * 3 + kPipGap * 2;
        const Vec2 next(-span / 2 + kPipSize / 2 + (kPipSize + kPipGap) * static_cast<float>(left - 1), 0.0f);
        helpPips_->drawSolidCircle(next, kPipSize / 2 + 6.0f, 0.0f, 20, theme::rgba(224, 176, 78, 0.25f));
        helpPips_->drawSolidCircle(next, kPipSize / 2 + 3.0f, 0.0f, 20, theme::rgba(224, 176, 78, 0.45f));
    }
    decor::pips(helpPips_, Vec2::ZERO, bs::core::kHelpUsesPerGame, left, kPipSize, kPipGap, theme::kGoldPip,
                theme::kGoldPale, theme::kPipEmpty, theme::rgba(0, 0, 0, 0.0f));
}

void BattleScene::refreshComputerCounter(bool burning) {
    const int left = controller_->computerHelpUsesLeft();
    computerCounterLabel_->refresh();
    computerCounterDigit_->setString(std::to_string(left));
    const float labelWidth = computerCounterLabel_->getContentSize().width;
    const float pipsWidth = kCounterPip * 3 + kCounterPipGap * 2;
    const float pipsX = 10.0f + labelWidth + 8.0f;
    const float width = pipsX + pipsWidth + 10.0f + computerCounterDigit_->getContentSize().width + 10.0f;
    computerCounter_->setContentSize(cocos2d::Size(width, 26.0f));
    computerCounterLabel_->setPosition(10.0f, 13.0f);
    computerCounterDigit_->setPosition(pipsX + pipsWidth + 10.0f, 13.0f);
    if (left == shownCounter_ && burning == shownCounterBurning_ &&
        computerCounterPlate_->getContentSize().width == width) {
        return;
    }
    shownCounter_ = left;
    shownCounterBurning_ = burning;
    computerCounterPlate_->setContentSize(cocos2d::Size(width, 26.0f));
    computerCounterPlate_->clear();
    const cocos2d::Rect box(0.0f, 0.0f, width, 26.0f);
    if (burning) { // its counter flares as it drops
        decor::softShadow(computerCounterPlate_, box, 3.0f, Vec2::ZERO, 18.0f, theme::rgba(224, 64, 46, 0.45f));
    }
    const decor::Poly shape = decor::roundedRect(box, 3.0f);
    decor::fill(computerCounterPlate_, shape, burning ? theme::rgba(0x2a, 0x0a, 0x06) : theme::rgba(0x16, 0x08, 0x06));
    decor::stroke(computerCounterPlate_, shape, 1.0f, burning ? theme::kEmber : theme::kFrameInlay);
    decor::pips(computerCounterPlate_, Vec2(pipsX + pipsWidth / 2, 13.0f), bs::core::kHelpUsesPerGame, left,
                kCounterPip, kCounterPipGap, theme::kBlood, theme::kEmber, theme::kFrameInlay, theme::kEmber);
}

// Name and caption (or, on Hard, the computer's Help counter) share one centred line above each
// board, so their positions follow the current language's text widths.
void BattleScene::layoutNameLines() {
    const auto line = [](cocos2d::Node* name, cocos2d::Node* second, float centreX, bool counter) {
        const float a = name->getContentSize().width;
        const float b = second->getContentSize().width;
        const float left = centreX - (a + kNameGap + b) / 2;
        name->setPosition(left + a / 2, kNameLineY);
        if (counter) { // the counter's origin is its bottom-left corner
            second->setPosition(left + a + kNameGap, kNameLineY - second->getContentSize().height / 2);
        } else { // the caption sits on the name's baseline
            second->setPosition(left + a + kNameGap + b / 2, kNameLineY - 2.0f);
        }
    };
    line(ownName_, ownCaption_, frameCentreX(kOwnGridX), false);
    if (computerCounter_ != nullptr) {
        line(enemyName_, computerCounter_, frameCentreX(kTargetGridX), true);
    } else {
        line(enemyName_, enemyCaption_, frameCentreX(kTargetGridX), false);
    }
}

// The bar at the bottom explains the moment: choosing a Help centre, the extra shot after a hit,
// or what the computer's Help just did.
void BattleScene::refreshHint() {
    const bs::app::TurnIndicator turn = controller_->turnIndicator();
    Hint wanted = Hint::None;
    if (controller_->mode() == bs::app::Mode::PlayerHelpTargeting) {
        wanted = Hint::HelpTargeting;
    } else if (turn.computerHelp) {
        wanted = Hint::ComputerHelp;
    } else if (turn.extraShot && controller_->playerHelpAvailable()) {
        wanted = Hint::ExtraShot;
    }
    if (wanted == hint_) {
        return;
    }
    hint_ = wanted;
    bs::app::ILocalizer* text = &services_.localizer();
    bs::app::BattleController* controller = controller_.get();
    switch (wanted) {
    case Hint::None:
        hintBar_->release();
        break;
    case Hint::ExtraShot:
        hintBar_->hold([text] { return text->text(TextKey::BattleHintExtra); }, Toast::Tone::Info);
        break;
    case Hint::HelpTargeting:
        hintBar_->hold([text] { return text->text(TextKey::BattleHelpHint); }, Toast::Tone::Gold);
        break;
    case Hint::ComputerHelp:
        hintBar_->hold(
            [text, controller] {
                const bs::app::HelpReport& report = controller->computerHelpReport();
                return text->format(TextKey::BattleHelpResult, {{"hits", std::to_string(report.hits)},
                                                                {"misses", std::to_string(report.misses)}});
            },
            Toast::Tone::Error);
        break;
    }
}

void BattleScene::showGameOverOnce() {
    if (gameOverShown_ || controller_->mode() != bs::app::Mode::GameOver) {
        return;
    }
    gameOverShown_ = true;
    hintBar_->release();
    addChild(GameOverLayer::create(services_, session_->winner()), 100);
}
