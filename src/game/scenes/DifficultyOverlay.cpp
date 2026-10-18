#include "scenes/DifficultyOverlay.h"

#include <functional>
#include <new>

#include "GameServices.h"
#include "Theme.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"

using bs::app::TextKey;
using bs::core::Difficulty;
using cocos2d::Vec2;

namespace {

// One level on the sheet: diamond pips that repeat the level without relying on text, its name
// and a one-line description. Dark wood with a brass edge on hover.
class DifficultyRow final : public cocos2d::Node {
public:
    static DifficultyRow* create(bs::app::ILocalizer& localizer, TextKey name, TextKey description, int level,
                                 std::function<void()> onClick) {
        auto* row = new (std::nothrow) DifficultyRow(level, std::move(onClick));
        if (row != nullptr && row->initRow(localizer, name, description)) {
            row->autorelease();
            return row;
        }
        delete row;
        return nullptr;
    }

private:
    static constexpr float kWidth = 504.0f;
    static constexpr float kHeight = 77.0f;

    DifficultyRow(int level, std::function<void()> onClick) : level_(level), onClick_(std::move(onClick)) {}

    bool initRow(bs::app::ILocalizer& localizer, TextKey name, TextKey description) {
        if (!Node::init()) {
            return false;
        }
        setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        setContentSize(cocos2d::Size(kWidth, kHeight));
        background_ = decor::canvas();
        addChild(background_);
        const float textX = 18.0f + 3.0f * 17.0f + 18.0f;
        name_ = LocalizedLabel::forKey(localizer, name, 25.0f, theme::Face::Caps);
        name_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        name_->setMaxWidth(kWidth - textX - 18.0f);
        name_->setPosition(textX, 50.0f);
        addChild(name_);
        description_ = LocalizedLabel::forKey(localizer, description, 18.0f, theme::Face::Italic);
        description_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        description_->setMaxWidth(kWidth - textX - 18.0f);
        description_->setPosition(textX, 24.0f);
        addChild(description_);
        redraw();

        auto touch = cocos2d::EventListenerTouchOneByOne::create();
        touch->setSwallowTouches(true);
        touch->onTouchBegan = [this](cocos2d::Touch* t, cocos2d::Event*) { return contains(t->getLocation()); };
        touch->onTouchEnded = [this](cocos2d::Touch* t, cocos2d::Event*) {
            if (contains(t->getLocation()) && onClick_) onClick_();
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);
        auto mouse = cocos2d::EventListenerMouse::create();
        mouse->onMouseMove = [this](cocos2d::EventMouse* e) {
            const bool inside = contains(Vec2(e->getCursorX(), e->getCursorY()));
            if (inside != hovered_) {
                hovered_ = inside;
                redraw();
            }
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);
        return true;
    }

    bool contains(const Vec2& worldPoint) const {
        return cocos2d::Rect(Vec2::ZERO, getContentSize()).containsPoint(convertToNodeSpace(worldPoint));
    }

    void redraw() {
        background_->clear();
        const cocos2d::Rect box(0.0f, 0.0f, kWidth, kHeight);
        const decor::Poly shape = decor::roundedRect(box, 4.0f);
        if (hovered_) {
            decor::fill(background_, decor::roundedRect(cocos2d::Rect(-3.0f, -3.0f, kWidth + 6.0f, kHeight + 6.0f), 7.0f),
                        theme::rgba(224, 176, 78, 0.3f));
            decor::fill(background_, shape, theme::rgba(0x3b, 0x26, 0x15));
            decor::stroke(background_, shape, 1.0f, theme::kBrassDeep);
        } else {
            decor::fill(background_, shape, theme::rgba(255, 248, 225, 0.35f));
            decor::stroke(background_, shape, 1.0f, theme::kParchmentRule);
        }
        for (int i = 0; i < 3; ++i) {
            const Vec2 c(18.0f + 8.5f + 17.0f * static_cast<float>(i), kHeight / 2);
            const Vec2 diamond[] = {c + Vec2(0.0f, 8.5f), c + Vec2(8.5f, 0.0f), c + Vec2(0.0f, -8.5f),
                                    c + Vec2(-8.5f, 0.0f)};
            if (i < level_) {
                background_->drawSolidPoly(diamond, 4, theme::kWax);
            }
            background_->drawPoly(diamond, 4, true, theme::rgba(0x6b, 0x4a, 0x22));
        }
        name_->setTextColor(hovered_ ? theme::kCream : theme::kInk);
        description_->setTextColor(hovered_ ? theme::rgb(0xcd, 0xb9, 0x8e) : theme::kInkFaded);
    }

    int level_;
    std::function<void()> onClick_;
    cocos2d::DrawNode* background_ = nullptr;
    LocalizedLabel* name_ = nullptr;
    LocalizedLabel* description_ = nullptr;
    bool hovered_ = false;
};

} // namespace

DifficultyOverlay* DifficultyOverlay::create(GameServices& services) {
    auto* layer = new (std::nothrow) DifficultyOverlay(services);
    if (layer != nullptr && layer->init()) {
        layer->autorelease();
        return layer;
    }
    delete layer;
    return nullptr;
}

bool DifficultyOverlay::init() {
    if (!LayerColor::initWithColor(cocos2d::Color4B(10, 6, 3, 184))) {
        return false;
    }
    auto& loc = services_.localizer();
    GameServices* services = &services_;

    auto sheet = decor::canvas();
    decor::panel(sheet, cocos2d::Rect(340, 120, 600, 480), 12.0f, decor::woodPanel());
    addChild(sheet);

    auto title = LocalizedLabel::forKey(loc, TextKey::DifficultyTitle, 38.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(520.0f);
    title->setPosition(640, 535);
    addChild(title);

    const struct {
        TextKey name;
        TextKey description;
        Difficulty difficulty;
        int level;
        float y;
    } levels[] = {{TextKey::DifficultyEasy, TextKey::DifficultyEasyDesc, Difficulty::Easy, 1, 453.0f},
                  {TextKey::DifficultyMedium, TextKey::DifficultyMediumDesc, Difficulty::Medium, 2, 362.0f},
                  {TextKey::DifficultyHard, TextKey::DifficultyHardDesc, Difficulty::Hard, 3, 271.0f}};
    for (const auto& level : levels) {
        const Difficulty difficulty = level.difficulty;
        auto row = DifficultyRow::create(loc, level.name, level.description, level.level, [services, difficulty] {
            services->audio().onClick();
            services->navigator().showPlacement(difficulty);
        });
        row->setPosition(640, level.y);
        addChild(row);
    }

    auto back = TextButton::forKey(
        loc, TextKey::CommonBack, cocos2d::Size(200, 50),
        [this] {
            services_.audio().onClick();
            removeFromParent();
        },
        TextButton::Style::Secondary, 21.0f);
    back->setPosition(640, 186);
    addChild(back);

    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
