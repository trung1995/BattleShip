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
constexpr float kColumnX = 286.0f; // the menu column sits on the left, over the dark fade
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
    // The rain-streaked night sea, fading to near-black towards the left where the menu stands.
    auto backdrop = decor::canvas();
    decor::rainSea(backdrop, cocos2d::Size(1280, 720));
    backdrop->drawSolidRect(cocos2d::Vec2::ZERO, cocos2d::Vec2(325, 720), theme::rgba(7, 9, 10, 0.97f));
    for (float x = 325.0f; x < 560.0f; x += 4.0f) {
        const float alpha = 0.97f * (1.0f - (x - 325.0f) / 235.0f);
        backdrop->drawSolidRect(cocos2d::Vec2(x, 0), cocos2d::Vec2(x + 4.0f, 720), theme::rgba(7, 9, 10, alpha));
    }
    decor::skullAndCutlasses(backdrop, cocos2d::Vec2(kColumnX, 574), 62.0f);
    // A red glow behind the title, then a blood-red rule slashed under it.
    for (int i = 6; i >= 1; --i) {
        const float f = static_cast<float>(i) / 6.0f;
        backdrop->drawSolidCircle(cocos2d::Vec2(kColumnX, 460), 230.0f * f, 0.0f, 48, 1.0f, 0.35f,
                                  theme::rgba(179, 36, 27, 0.07f));
    }
    addChild(backdrop);
    auto rule = decor::canvas();
    decor::softShadow(rule, cocos2d::Rect(-110, -2, 220, 4), 1.0f, cocos2d::Vec2::ZERO, 14.0f,
                      theme::rgba(224, 64, 46, 0.6f));
    rule->drawSolidRect(cocos2d::Vec2(-110, -2), cocos2d::Vec2(110, 2), theme::kBlood);
    rule->setPosition(kColumnX, 404);
    rule->setRotation(-2.0f);
    addChild(rule);
    auto& loc = services_.localizer();

    auto title = LocalizedLabel::forKey(loc, TextKey::AppTitle, 62.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kTitle);
    title->setShadow(cocos2d::Color4B(0x3d, 0x0a, 0x05, 255), cocos2d::Size(0, -2));
    title->setMaxWidth(440.0f);
    title->setPosition(kColumnX, 458);
    addChild(title);

    // Planks nailed in a hurry: each button sits at its own small angle.
    const cocos2d::Size buttonSize(380, 64);
    GameServices* services = &services_;
    auto newGame = TextButton::forKey(
        loc, TextKey::MenuNewGame, buttonSize,
        [this] {
            services_.audio().onClick();
            showDifficulty();
        },
        TextButton::Style::Primary, 26.0f);
    newGame->setPosition(kColumnX, 336);
    newGame->setRotation(-1.0f);
    addChild(newGame);

    auto settings = TextButton::forKey(
        loc, TextKey::MenuSettings, buttonSize,
        [services] {
            services->audio().onClick();
            services->navigator().showSettings();
        },
        TextButton::Style::Secondary, 26.0f);
    settings->setPosition(kColumnX, 258);
    settings->setRotation(0.6f);
    addChild(settings);

    auto quit = TextButton::forKey(
        loc, TextKey::MenuQuit, buttonSize, [] { cocos2d::Director::getInstance()->end(); },
        TextButton::Style::Secondary, 26.0f);
    quit->setPosition(kColumnX, 180);
    quit->setRotation(-0.4f);
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
