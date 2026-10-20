#include "scenes/PlacementScene.h"

#include <algorithm>
#include <new>
#include <string>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/BoardNode.h"
#include "nodes/Decor.h"
#include "nodes/DockNode.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/ShipNode.h"
#include "nodes/TextButton.h"
#include "nodes/Toast.h"

using bs::app::TextKey;
using bs::core::Coord;
using bs::core::Orientation;
using bs::core::PlacementError;

namespace {
TextKey reasonKey(PlacementError reason) {
    switch (reason) {
    case PlacementError::OutOfBounds: return TextKey::PlacementErrorOutOfBounds;
    case PlacementError::Overlaps: return TextKey::PlacementErrorOverlaps;
    default: return TextKey::PlacementErrorTouches;
    }
}
} // namespace

PlacementScene* PlacementScene::create(GameServices& services) {
    auto* scene = new (std::nothrow) PlacementScene(services);
    if (scene != nullptr && scene->init()) {
        scene->autorelease();
        return scene;
    }
    delete scene;
    return nullptr;
}

bool PlacementScene::init() {
    if (!Scene::init()) {
        return false;
    }
    // A new game starts when this screen opens: the computer's fleet is placed now and the
    // player arranges theirs directly in the session.
    session_ = services_.setup().newSession(services_.currentDifficulty);
    services_.currentSession = session_;
    controller_ = std::make_unique<bs::app::PlacementController>(session_->playerFleet(), services_.placer(),
                                                                 services_.random());

    auto backdrop = decor::canvas();
    decor::storm(backdrop, cocos2d::Size(1280, 720), cocos2d::Vec2(0.3f, 1.0f), theme::kStormLight, theme::kStormDeep);
    addChild(backdrop);
    auto& loc = services_.localizer();

    auto title = LocalizedLabel::forKey(loc, TextKey::PlacementTitle, 40.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kTitle);
    title->setMaxWidth(760.0f);
    title->setPosition(640, 672);
    addChild(title);
    auto hint = LocalizedLabel::forKey(loc, TextKey::PlacementHint, 19.0f, theme::Face::Italic);
    hint->setTextColor(theme::kHint);
    hint->setMaxWidth(800.0f);
    hint->setPosition(640, 625);
    addChild(hint);

    board_ = BoardNode::create(theme::kCellSize);
    board_->setPosition(146, 182);
    board_->setFleet(&session_->playerFleet());
    addChild(board_);

    // Why the current drop is illegal: a red diamond and the reason, under the board.
    reasonMark_ = decor::canvas();
    const cocos2d::Vec2 diamond[] = {cocos2d::Vec2(117, 139), cocos2d::Vec2(124, 132), cocos2d::Vec2(117, 125),
                                     cocos2d::Vec2(110, 132)};
    reasonMark_->drawSolidPoly(diamond, 4, theme::kEmber);
    reasonMark_->setVisible(false);
    addChild(reasonMark_);
    reason_ = LocalizedLabel::create(loc, nullptr, 19.0f);
    reason_->setTextColor(theme::kWarning);
    reason_->setMaxWidth(400.0f);
    reason_->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    reason_->setPosition(134, 132);
    addChild(reason_);

    dock_ = DockNode::create(loc, theme::kCellSize);
    dock_->setPosition(640, 387);
    addChild(dock_);

    const cocos2d::Size buttonSize(263, 58);
    auto rotateButton = TextButton::forKey(loc, TextKey::PlacementRotate, buttonSize, [this] { rotate(); },
                                           TextButton::Style::Secondary, 20.0f);
    rotateButton->setPosition(771.5f, 281);
    addChild(rotateButton);

    auto remaining = TextButton::forKey(
        loc, TextKey::PlacementAutoRemaining, buttonSize,
        [this] {
            services_.audio().onClick();
            if (controller_->autoPlaceRemaining() == bs::app::AutoResult::CannotFit) {
                toast_->show(TextKey::PlacementErrorCannotFit); // nothing moved (FR-012)
            }
            refresh();
        },
        TextButton::Style::Secondary, 20.0f);
    remaining->setPosition(1048.5f, 281);
    addChild(remaining);

    auto autoAll = TextButton::forKey(
        loc, TextKey::PlacementAutoAll, buttonSize,
        [this] {
            services_.audio().onClick();
            controller_->autoPlaceAll();
            refresh();
        },
        TextButton::Style::Secondary, 20.0f);
    autoAll->setPosition(771.5f, 209);
    addChild(autoAll);

    start_ = TextButton::forKey(loc, TextKey::PlacementStart, buttonSize, [this] { startBattle(); },
                                TextButton::Style::Primary, 20.0f);
    start_->setPosition(1048.5f, 209);
    addChild(start_);

    bs::app::PlacementController* controller = controller_.get();
    bs::app::ILocalizer* source = &loc;
    missing_ = LocalizedLabel::create(
        loc,
        [source, controller] {
            return source->format(TextKey::PlacementMissing, {{"n", std::to_string(controller->dockShips().size())}});
        },
        17.0f, theme::Face::Italic);
    missing_->setTextColor(theme::kWarning);
    missing_->setMaxWidth(263.0f);
    missing_->setPosition(1048.5f, 161);
    addChild(missing_);

    auto backButton = TextButton::forKey(loc, TextKey::CommonBack, cocos2d::Size(130, 46), [this] { back(); },
                                         TextButton::Style::Secondary, 20.0f);
    backButton->setPosition(1183, 669);
    addChild(backButton);

    ghost_ = ShipNode::create(1, Orientation::Horizontal, theme::kCellSize, hull::State::Lifted);
    ghost_->setOpacity(238);
    ghost_->setCascadeOpacityEnabled(true);
    ghost_->setVisible(false);
    addChild(ghost_, 10);

    toast_ = Toast::create(loc, Toast::Tone::Error);
    toast_->setPosition(640, 50);
    addChild(toast_, 20);

    addMouseAndKeyboard();
    refresh();
    return true;
}

void PlacementScene::addMouseAndKeyboard() {
    auto mouse = cocos2d::EventListenerMouse::create();
    mouse->onMouseDown = [this](cocos2d::EventMouse* e) {
        const cocos2d::Vec2 point(e->getCursorX(), e->getCursorY());
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT) {
            rotate();
        } else if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_LEFT) {
            onMouseDown(point);
        }
    };
    mouse->onMouseMove = [this](cocos2d::EventMouse* e) { onMouseMove(cocos2d::Vec2(e->getCursorX(), e->getCursorY())); };
    mouse->onMouseUp = [this](cocos2d::EventMouse* e) {
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_LEFT) {
            onMouseUp(cocos2d::Vec2(e->getCursorX(), e->getCursorY()));
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    auto keys = cocos2d::EventListenerKeyboard::create();
    keys->onKeyReleased = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event*) {
        if (code == cocos2d::EventKeyboard::KeyCode::KEY_R) {
            rotate();
        } else if (code == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE) {
            if (controller_->isDragging()) {
                controller_->cancelDrag();
                refresh();
            } else {
                back();
            }
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keys, this);
}

void PlacementScene::onEnter() {
    Scene::onEnter();
    services_.audio().enterScreen(bs::app::Screen::Placement);
}

void PlacementScene::onMouseDown(const cocos2d::Vec2& point) {
    Coord cell{0, 0};
    if (board_->cellAt(point, cell)) {
        const bs::core::Fleet& fleet = session_->playerFleet();
        const bs::core::ShipId id = fleet.shipAt(cell);
        if (id >= 0) {
            const bs::core::ShipPlacement* p = fleet.placementOf(id);
            grabOffset_ = std::max(cell.row - p->origin.row, cell.col - p->origin.col);
            controller_->beginDragPlaced(id);
        }
    } else {
        const bs::core::ShipId id = dock_->shipAt(point);
        if (id >= 0) {
            grabOffset_ = 0;
            controller_->beginDragFromDock(id);
        }
    }
    onMouseMove(point);
}

Coord PlacementScene::originUnder(Coord cursorCell) const {
    const int length = bs::core::fleetSpec()[static_cast<std::size_t>(controller_->draggedShip())];
    const int offset = std::min(grabOffset_, length - 1);
    return controller_->dragOrientation() == Orientation::Horizontal
               ? Coord{cursorCell.row, cursorCell.col - offset}
               : Coord{cursorCell.row - offset, cursorCell.col};
}

void PlacementScene::onMouseMove(const cocos2d::Vec2& point) {
    lastMouse_ = point;
    if (!controller_->isDragging()) {
        return;
    }
    const int length = bs::core::fleetSpec()[static_cast<std::size_t>(controller_->draggedShip())];
    const bool vertical = controller_->dragOrientation() == Orientation::Vertical;
    ghost_->setShape(length, controller_->dragOrientation());
    Coord cell{0, 0};
    if (board_->cellAt(point, cell)) {
        // Over the board the ship lifts above the cells it would take, tilted a little.
        const Coord origin = originUnder(cell);
        const bs::app::Preview preview = controller_->hover(origin);
        board_->setPreview(preview.cells, preview.legal);
        showReason(preview.reason);
        const Coord bottomCell = vertical ? Coord{origin.row + length - 1, origin.col} : origin;
        ghost_->setPosition(board_->convertToWorldSpace(board_->cellOrigin(bottomCell)) + cocos2d::Vec2(-8, 12));
        ghost_->setRotation(-4.0f);
    } else {
        board_->clearPreview();
        showReason(PlacementError::None);
        // Off the board the ship follows the cursor by the cell it was picked up by.
        const int held = std::min(grabOffset_, length - 1);
        const float half = theme::kCellSize / 2;
        const cocos2d::Vec2 offset =
            vertical ? cocos2d::Vec2(half, theme::kCellSize * static_cast<float>(length - 1 - held) + half)
                     : cocos2d::Vec2(theme::kCellSize * static_cast<float>(held) + half, half);
        ghost_->setPosition(point - offset);
        ghost_->setRotation(0.0f);
    }
    ghost_->setVisible(true);
}

void PlacementScene::onMouseUp(const cocos2d::Vec2& point) {
    if (!controller_->isDragging()) {
        return;
    }
    Coord cell{0, 0};
    if (board_->cellAt(point, cell)) {
        const bs::app::DropResult result = controller_->drop(originUnder(cell));
        if (result.status == bs::app::DropStatus::Rejected) {
            toast_->show(reasonKey(result.reason)); // the ship stays where it was (FR-013)
        }
    } else if (dock_->contains(point)) {
        controller_->dropOnDock();
    } else {
        controller_->cancelDrag();
    }
    refresh();
}

void PlacementScene::rotate() {
    const PlacementError error = controller_->rotate();
    if (error != PlacementError::None) {
        toast_->show(reasonKey(error));
    }
    if (controller_->isDragging()) {
        onMouseMove(lastMouse_);
    } else {
        refresh();
    }
}

void PlacementScene::showReason(PlacementError reason) {
    reasonMark_->setVisible(reason != PlacementError::None);
    if (reason == PlacementError::None) {
        reason_->setProvider(nullptr);
        return;
    }
    bs::app::ILocalizer* loc = &services_.localizer();
    const TextKey key = reasonKey(reason);
    reason_->setProvider([loc, key] { return loc->text(key); });
}

void PlacementScene::refresh() {
    board_->clearPreview();
    board_->redraw();
    ghost_->setVisible(false);
    showReason(PlacementError::None);
    board_->setSelectedShip(controller_->selectedShip());
    dock_->setShips(controller_->dockShips());
    start_->setLooksDisabled(!controller_->canStartBattle());
    missing_->setVisible(!controller_->canStartBattle());
    missing_->refresh();
}

void PlacementScene::startBattle() {
    services_.audio().onClick();
    if (!controller_->canStartBattle()) {
        toast_->show(TextKey::PlacementErrorFleetIncomplete);
        return;
    }
    if (session_->startBattle() == bs::core::StartStatus::Started) {
        services_.navigator().showBattle();
    }
}

void PlacementScene::back() {
    services_.audio().onClick();
    services_.navigator().showMenu(); // no confirmation outside a battle (US6 scenario 4)
}
