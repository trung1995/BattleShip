#include "scenes/GameOverLayer.h"

#include <new>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"

using bs::app::TextKey;

GameOverLayer* GameOverLayer::create(GameServices& services, bs::core::Side winner) {
    auto* layer = new (std::nothrow) GameOverLayer(services, winner);
    if (layer != nullptr && layer->init()) {
        layer->autorelease();
        return layer;
    }
    delete layer;
    return nullptr;
}

// Victory: a gold frame with a glow over a light dim, so the final boards stay readable.
// Defeat: a tarred frame, red ink and a heavier dim.
bool GameOverLayer::init() {
    const bool victory = winner_ == bs::core::Side::Player;
    if (!LayerColor::initWithColor(victory ? cocos2d::Color4B(10, 6, 3, 140) : cocos2d::Color4B(14, 4, 2, 158))) {
        return false;
    }
    auto& loc = services_.localizer();
    GameServices* services = &services_;
    const bs::core::Side winner = winner_;

    decor::PanelStyle style;
    if (victory) {
        style = {theme::kBrassDeep, theme::kBrassDark, theme::rgba(0xef, 0xdc, 0xae), theme::rgba(120, 80, 30, 0.5f),
                 true};
    } else {
        style = {theme::rgba(0x2a, 0x16, 0x10), theme::rgba(0x6b, 0x24, 0x18), theme::rgba(0xd9, 0xc5, 0x97),
                 theme::rgba(70, 30, 15, 0.6f), false};
    }
    auto sheet = decor::canvas();
    decor::panel(sheet, cocos2d::Rect(370, 182, 540, 356), 12.0f, style);
    if (victory) {
        decor::victoryEmblem(sheet, cocos2d::Vec2(640, 450), 46.0f);
    } else {
        decor::defeatEmblem(sheet, cocos2d::Vec2(640, 450), 46.0f);
    }
    addChild(sheet);

    auto kicker = LocalizedLabel::forKey(loc, victory ? TextKey::GameOverVictory : TextKey::GameOverDefeat, 17.0f,
                                         theme::Face::Caps);
    kicker->setTextColor(victory ? theme::rgb(0x8a, 0x62, 0x24) : theme::rgb(0x8e, 0x2a, 0x1c));
    kicker->setLetterSpacing(3.7f);
    kicker->setPosition(640, 379);
    addChild(kicker);

    auto message = LocalizedLabel::create(
        loc,
        [services, winner] {
            return services->localizer().format(
                TextKey::BattleWinner,
                {{"name", services->localizer().displayName(winner, services->settings())}});
        },
        46.0f, theme::Face::CapsHeavy);
    message->setTextColor(victory ? theme::kInk : theme::kInkRed);
    message->setMaxWidth(460.0f);
    message->setPosition(640, 328);
    addChild(message);

    const cocos2d::Size buttonSize(210, 54);
    auto newGame = TextButton::forKey(
        loc, TextKey::GameOverNewGame, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showNewGame();
        },
        TextButton::Style::Primary);
    newGame->setPosition(528, 251);
    addChild(newGame);

    auto menu = TextButton::forKey(
        loc, TextKey::GameOverMenu, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showMenu();
        },
        TextButton::Style::Secondary);
    menu->setPosition(752, 251);
    addChild(menu);

    // Swallow clicks so nothing underneath reacts while the overlay is up.
    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
