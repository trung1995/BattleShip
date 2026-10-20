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
// Defeat: a scorched, torn note tilted the other way, over a blood-red vignette.
bool GameOverLayer::init() {
    const bool victory = winner_ == bs::core::Side::Player;
    if (!LayerColor::initWithColor(victory ? cocos2d::Color4B(4, 5, 6, 153) : cocos2d::Color4B(20, 2, 0, 165))) {
        return false;
    }
    auto& loc = services_.localizer();
    GameServices* services = &services_;
    const bs::core::Side winner = winner_;

    if (!victory) {
        auto vignette = decor::canvas();
        for (int i = 40; i >= 1; --i) { // many faint layers, so no rings show
            const float f = static_cast<float>(i) / 40.0f;
            vignette->drawSolidCircle(cocos2d::Vec2(640, 360), 620.0f * f, 0.0f, 96, 1.0f, 0.62f,
                                      theme::rgba(0x3a, 0x0c, 0x08, 0.025f));
        }
        addChild(vignette);
    }

    auto sheet = cocos2d::Node::create();
    sheet->setPosition(640, 360);
    sheet->setRotation(victory ? -1.0f : 1.4f);
    addChild(sheet);
    auto paper = decor::canvas();
    if (victory) {
        decor::goldFrame(paper, cocos2d::Rect(-280, -175, 560, 350), 10.0f);
        decor::treasureChest(paper, cocos2d::Vec2(0, 89), 48.0f);
    } else {
        decor::tornSheet(paper, cocos2d::Rect(-280, -175, 560, 350), theme::rgba(0xc9, 0xb3, 0x86),
                         theme::rgba(40, 8, 0, 0.9f), 56.0f);
        decor::drownedSkull(paper, cocos2d::Vec2(0, 89), 48.0f);
    }
    sheet->addChild(paper);

    auto kicker = LocalizedLabel::forKey(loc, victory ? TextKey::GameOverVictory : TextKey::GameOverDefeat, 17.0f,
                                         theme::Face::Caps);
    kicker->setTextColor(victory ? theme::kInkGold : theme::kInkRed);
    kicker->setLetterSpacing(3.7f);
    kicker->setPosition(0, 21);
    sheet->addChild(kicker);

    auto message = LocalizedLabel::create(
        loc,
        [services, winner] {
            return services->localizer().format(
                TextKey::BattleWinner,
                {{"name", services->localizer().displayName(winner, services->settings())}});
        },
        46.0f, theme::Face::CapsHeavy);
    message->setTextColor(victory ? theme::kInk : theme::kInkBlood);
    message->setMaxWidth(480.0f);
    message->setPosition(0, -30);
    sheet->addChild(message);

    const cocos2d::Size buttonSize(210, 54);
    auto newGame = TextButton::forKey(
        loc, TextKey::GameOverNewGame, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showNewGame();
        },
        TextButton::Style::Primary);
    newGame->setPosition(-112, -107);
    sheet->addChild(newGame);

    auto menu = TextButton::forKey(
        loc, TextKey::GameOverMenu, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showMenu();
        },
        TextButton::Style::Secondary);
    menu->setPosition(112, -107);
    sheet->addChild(menu);

    // Swallow clicks so nothing underneath reacts while the overlay is up.
    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
