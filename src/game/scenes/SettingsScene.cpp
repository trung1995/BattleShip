#include "scenes/SettingsScene.h"

#include <new>
#include <string>
#include <utility>

#include "GameServices.h"
#include "Theme.h"
#include "bs/app/Utf8.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"
#include "nodes/ToggleSwitch.h"
#include "ui/UITextField.h"

using bs::app::Language;
using bs::app::TextKey;

// Coordinates are local to the sheet, whose centre sits at the middle of the screen.
namespace {
constexpr float kLeft = -316.0f; // the parchment's content edges
constexpr float kRight = 316.0f;
constexpr std::size_t kNameLimit = 20;
const cocos2d::Rect kField(kLeft, -123.0f, kRight - kLeft, 52.0f);
} // namespace

SettingsScene* SettingsScene::create(GameServices& services) {
    auto* scene = new (std::nothrow) SettingsScene(services);
    if (scene != nullptr && scene->init()) {
        scene->autorelease();
        return scene;
    }
    delete scene;
    return nullptr;
}

bool SettingsScene::init() {
    if (!Scene::init()) {
        return false;
    }
    controller_ = std::make_unique<bs::app::SettingsController>(services_.settings(), services_.localizer(),
                                                               services_.audio());
    auto backdrop = decor::canvas();
    decor::storm(backdrop, cocos2d::Size(1280, 720), cocos2d::Vec2(0.3f, 1.0f), theme::kStormLight, theme::kStormDeep);
    addChild(backdrop);
    sheet_ = cocos2d::Node::create();
    sheet_->setPosition(640, 360);
    sheet_->setRotation(0.4f);
    addChild(sheet_);
    auto paper = decor::canvas();
    decor::tornSheet(paper, cocos2d::Rect(-360, -260, 720, 520), theme::kParchment, theme::kParchmentBurn);
    for (const float y : {120.0f, 58.0f, -15.0f}) {
        paper->drawSolidRect(cocos2d::Vec2(kLeft, y), cocos2d::Vec2(kRight, y + 1.0f), theme::kParchmentRule);
    }
    sheet_->addChild(paper);
    auto& loc = services_.localizer();

    auto title = LocalizedLabel::forKey(loc, TextKey::SettingsTitle, 40.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(600.0f);
    title->setPosition(0, 206);
    sheet_->addChild(title);

    sound_ = addToggleRow(TextKey::SettingsSound, 149.0f, &bs::app::ISettings::soundEnabled, [this] {
        controller_->toggleSound();
        services_.audio().onClick();
        refresh();
    }, soundState_);
    music_ = addToggleRow(TextKey::SettingsMusic, 87.0f, &bs::app::ISettings::musicEnabled, [this] {
        services_.audio().onClick();
        controller_->toggleMusic();
        refresh();
    }, musicState_);

    // Language names are always shown in their own language (contracts/ui-screens.md).
    addRowLabel(TextKey::SettingsLanguage, 20.0f);
    const float widths[] = {108.0f, 108.0f, 120.0f, 128.0f};
    const float groupLeft = kRight - (widths[0] + widths[1] + widths[2] + widths[3]);
    auto group = decor::canvas();
    decor::stroke(group, decor::roundedRect(cocos2d::Rect(groupLeft - 0.5f, -0.5f, kRight - groupLeft + 1.0f, 41.0f), 3.0f),
                  1.0f, theme::rgba(0x3a, 0x1a, 0x12));
    sheet_->addChild(group, 1);
    const auto languages = controller_->languages();
    float x = groupLeft;
    for (std::size_t i = 0; i < languages.size(); ++i) {
        const Language language = languages[i].first;
        languageButtons_[i] = TextButton::forKey(
            loc, languages[i].second, cocos2d::Size(widths[i], 40.0f),
            [this, language] {
                services_.audio().onClick();
                controller_->selectLanguage(language); // every label updates via its subscription
                refresh();
            },
            TextButton::Style::SegmentOff, 19.0f);
        languageButtons_[i]->setPosition(x + widths[i] / 2, 20.0f);
        sheet_->addChild(languageButtons_[i]);
        x += widths[i];
    }

    addRowLabel(TextKey::SettingsPlayerName, -48.0f);
    addNameField(kField.getMidY());

    // Leaving the name empty sails under the default name of the current language (FR-041).
    GameServices* services = &services_;
    auto nameHint = LocalizedLabel::create(
        loc,
        [services] {
            return services->localizer().format(TextKey::SettingsNameHint,
                                                {{"name", services->localizer().text(TextKey::NamePlayerDefault)}});
        },
        17.0f, theme::Face::Italic);
    nameHint->setTextColor(theme::kInkBrown);
    nameHint->setMaxWidth(kRight - kLeft);
    nameHint->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    nameHint->setPosition(kLeft, -142.0f);
    sheet_->addChild(nameHint);

    auto backButton = TextButton::forKey(loc, TextKey::CommonBack, cocos2d::Size(200, 52), [this] { back(); },
                                         TextButton::Style::Secondary, 22.0f);
    backButton->setPosition(0, -201);
    sheet_->addChild(backButton);

    auto keys = cocos2d::EventListenerKeyboard::create();
    keys->onKeyReleased = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event*) {
        if (code == cocos2d::EventKeyboard::KeyCode::KEY_ENTER ||
            code == cocos2d::EventKeyboard::KeyCode::KEY_KP_ENTER) {
            name_->didNotSelectSelf(); // detach from the keyboard, which commits
        } else if (code == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE) {
            back();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keys, this);

    refresh();
    return true;
}

void SettingsScene::addRowLabel(TextKey key, float y) {
    auto label = LocalizedLabel::forKey(services_.localizer(), key, 23.0f);
    label->setTextColor(theme::kInk);
    label->setMaxWidth(300.0f);
    label->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    label->setPosition(kLeft, y);
    sheet_->addChild(label);
}

ToggleSwitch* SettingsScene::addToggleRow(TextKey key, float y, bool (bs::app::ISettings::*isOn)() const,
                                          std::function<void()> onClick, LocalizedLabel*& stateLabel) {
    addRowLabel(key, y);
    auto toggle = ToggleSwitch::create(std::move(onClick));
    toggle->setPosition(kRight - 34.0f, y);
    sheet_->addChild(toggle);

    GameServices* services = &services_;
    auto state = LocalizedLabel::create(
        services_.localizer(),
        [services, isOn] {
            return services->localizer().text((services->settings().*isOn)() ? TextKey::SettingsOn
                                                                              : TextKey::SettingsOff);
        },
        18.0f, theme::Face::Caps);
    state->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
    state->setPosition(kRight - 80.0f, y);
    sheet_->addChild(state);
    stateLabel = state;
    return toggle;
}

void SettingsScene::addNameField(float y) {
    nameFrame_ = decor::canvas();
    sheet_->addChild(nameFrame_);

    name_ = cocos2d::ui::TextField::create("", theme::fontFile(theme::Face::Body), 23);
    name_->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    name_->setTouchAreaEnabled(true);
    name_->setTouchSize(cocos2d::Size(kField.size.width - 16.0f, kField.size.height));
    name_->setCursorEnabled(true);
    name_->setTextColor(cocos2d::Color4B(theme::kInk));
    name_->setPlaceHolderColor(cocos2d::Color4B(0x7a, 0x5d, 0x3c, 255));
    name_->setString(services_.settings().storedPlayerName());
    name_->setPosition(cocos2d::Vec2(kLeft + 16.0f, y));
    name_->addEventListener([this](cocos2d::Ref*, cocos2d::ui::TextField::EventType type) {
        switch (type) {
        case cocos2d::ui::TextField::EventType::INSERT_TEXT: {
            const std::string filtered = controller_->filterNameInput(name_->getString());
            if (filtered != name_->getString()) name_->setString(filtered); // 20 characters max
            break;
        }
        case cocos2d::ui::TextField::EventType::ATTACH_WITH_IME:
            nameFocused_ = true;
            break;
        case cocos2d::ui::TextField::EventType::DETACH_WITH_IME:
            nameFocused_ = false;
            commitName();
            break;
        default:
            break;
        }
        drawNameField();
    });
    sheet_->addChild(name_, 1);

    nameCount_ = cocos2d::Label::createWithTTF("", theme::fontFile(theme::Face::Body), 15.0f);
    nameCount_->setTextColor(cocos2d::Color4B(0x7a, 0x5d, 0x3c, 255));
    nameCount_->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
    nameCount_->setPosition(kRight - 16.0f, y);
    sheet_->addChild(nameCount_, 1);
    drawNameField();
}

// Pale parchment field; a blood-red ring while typing, with a live "n / 20" count.
void SettingsScene::drawNameField() {
    nameFrame_->clear();
    const decor::Poly shape = decor::roundedRect(kField, 2.0f);
    if (nameFocused_) {
        decor::fill(nameFrame_,
                    decor::roundedRect(cocos2d::Rect(kField.getMinX() - 3.0f, kField.getMinY() - 3.0f,
                                                     kField.size.width + 6.0f, kField.size.height + 6.0f),
                                       5.0f),
                    theme::rgba(179, 36, 27, 0.5f));
    }
    decor::fill(nameFrame_, shape, theme::rgba(0xef, 0xe1, 0xc2));
    nameFrame_->drawSolidRect(cocos2d::Vec2(kField.getMinX() + 1.0f, kField.getMaxY() - 4.0f),
                              cocos2d::Vec2(kField.getMaxX() - 1.0f, kField.getMaxY() - 1.0f),
                              theme::rgba(60, 30, 10, 0.18f));
    decor::stroke(nameFrame_, shape, 1.0f, theme::rgba(0x3a, 0x1a, 0x12));
    nameCount_->setString(std::to_string(bs::app::codePointCount(name_->getString())) + " / " +
                          std::to_string(kNameLimit));
    nameCount_->setVisible(nameFocused_ || !name_->getString().empty());
}

void SettingsScene::commitName() {
    const bs::app::NameResult result = controller_->commitName(name_->getString());
    name_->setString(result.stored); // trimmed; empty shows the placeholder and the default name
}

void SettingsScene::refresh() {
    sound_->setOn(services_.settings().soundEnabled());
    music_->setOn(services_.settings().musicEnabled());
    const std::pair<LocalizedLabel*, bool> states[] = {{soundState_, services_.settings().soundEnabled()},
                                                       {musicState_, services_.settings().musicEnabled()}};
    for (const auto& state : states) {
        state.first->refresh();
        state.first->setTextColor(state.second ? theme::kInkRed : theme::kInkFaded);
    }
    const auto languages = controller_->languages();
    for (std::size_t i = 0; i < languages.size(); ++i) {
        languageButtons_[i]->setStyle(languages[i].first == services_.settings().language()
                                          ? TextButton::Style::SegmentOn
                                          : TextButton::Style::SegmentOff);
    }
    name_->setPlaceHolder(services_.localizer().text(TextKey::SettingsNamePlaceholder));
    drawNameField();
}

void SettingsScene::onEnter() {
    Scene::onEnter();
    subscription_ = services_.localizer().subscribe([this] { refresh(); });
    services_.audio().enterScreen(bs::app::Screen::Settings);
}

void SettingsScene::onExit() {
    services_.localizer().unsubscribe(subscription_);
    Scene::onExit();
}

void SettingsScene::back() {
    commitName();
    services_.audio().onClick();
    services_.navigator().showMenu();
}
