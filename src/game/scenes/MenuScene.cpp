#include "scenes/MenuScene.h"

#include <new>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"
#include "scenes/DifficultyOverlay.h"

using bs::app::TextKey;

namespace {
constexpr float kColumnX = 276.0f; // the menu column sits on the left, over the dark fade
} // namespace

MenuScene* MenuScene::create(GameServices& services, bool openDifficulty) {
    auto* scene = new (std::nothrow) MenuScene(services, openDifficulty);
    if (scene != nullptr && scene->init()) {
        scene->autorelease();
        return scene;
    }
    delete scene;
    return nullptr;
}

bool MenuScene::init() {
    if (!Scene::init()) {
        return false;
    }
    // The night sea, darkened towards the left where the menu stands.
    auto backdrop = decor::canvas();
    decor::nightSea(backdrop, cocos2d::Size(1280, 720));
    backdrop->drawSolidRect(cocos2d::Vec2::ZERO, cocos2d::Vec2(312, 720), theme::rgba(18, 12, 7, 0.96f));
    for (float x = 312.0f; x < 520.0f; x += 4.0f) {
        const float alpha = 0.96f * (1.0f - (x - 312.0f) / 208.0f);
        backdrop->drawSolidRect(cocos2d::Vec2(x, 0), cocos2d::Vec2(x + 4.0f, 720), theme::rgba(18, 12, 7, alpha));
    }
    decor::compassRose(backdrop, cocos2d::Vec2(kColumnX, 558), 66.0f);
    addChild(backdrop);
    auto& loc = services_.localizer();

    auto title = LocalizedLabel::forKey(loc, TextKey::AppTitle, 64.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kTitle);
    title->setShadow(cocos2d::Color4B(0x3a, 0x24, 0x14, 255), cocos2d::Size(0, -3));
    title->setMaxWidth(460.0f);
    title->setPosition(kColumnX, 433);
    addChild(title);

    const cocos2d::Size buttonSize(360, 64);
    GameServices* services = &services_;
    auto newGame = TextButton::forKey(
        loc, TextKey::MenuNewGame, buttonSize,
        [this] {
            services_.audio().onClick();
            showDifficulty();
        },
        TextButton::Style::Primary, 26.0f);
    newGame->setPosition(kColumnX, 330);
    addChild(newGame);

    auto settings = TextButton::forKey(
        loc, TextKey::MenuSettings, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showSettings();
        },
        TextButton::Style::Secondary, 26.0f);
    settings->setPosition(kColumnX, 252);
    addChild(settings);

    auto quit = TextButton::forKey(
        loc, TextKey::MenuQuit, buttonSize, [] { cocos2d::Director::getInstance()->end(); },
        TextButton::Style::Secondary, 26.0f);
    quit->setPosition(kColumnX, 174);
    addChild(quit);
    if (openDifficulty_) {
        showDifficulty();
    }
    return true;
}

void MenuScene::showDifficulty() { addChild(DifficultyOverlay::create(services_), 100); }

void MenuScene::onEnter() {
    Scene::onEnter();
    services_.audio().enterScreen(bs::app::Screen::Menu);
}
