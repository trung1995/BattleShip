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
// and a one-line description. Tar with a blood-red edge and glow on hover.
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
    static constexpr float kWidth = 560.0f;
    static constexpr float kHeight = 81.0f;

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
        name_->setPosition(textX, 52.0f);
        addChild(name_);
        description_ = LocalizedLabel::forKey(localizer, description, 18.0f, theme::Face::Italic);
        description_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        description_->setMaxWidth(kWidth - textX - 18.0f);
        description_->setPosition(textX, 25.0f);
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
        const decor::Poly shape = decor::roundedRect(box, 3.0f);
        if (hovered_) {
            decor::softShadow(background_, box, 3.0f, Vec2::ZERO, 24.0f, theme::rgba(179, 36, 27, 0.4f));
            decor::fill(background_, decor::roundedRect(cocos2d::Rect(-3.0f, -3.0f, kWidth + 6.0f, kHeight + 6.0f), 6.0f),
                        theme::rgba(179, 36, 27, 0.35f));
            decor::fill(background_, shape, theme::kTar);
            decor::stroke(background_, shape, 1.0f, theme::kBlood);
        } else {
            decor::fill(background_, shape, theme::rgba(255, 248, 225, 0.3f));
            decor::stroke(background_, shape, 1.0f, theme::kParchmentRule);
        }
        for (int i = 0; i < 3; ++i) {
            const Vec2 c(18.0f + 8.5f + 17.0f * static_cast<float>(i), kHeight / 2);
            const Vec2 diamond[] = {c + Vec2(0.0f, 8.5f), c + Vec2(8.5f, 0.0f), c + Vec2(0.0f, -8.5f),
                                    c + Vec2(-8.5f, 0.0f)};
            if (i < level_) {
                background_->drawSolidPoly(diamond, 4, theme::kBlood);
            }
            background_->drawPoly(diamond, 4, true, theme::kBloodDark);
        }
        name_->setTextColor(hovered_ ? theme::rgb(0xf3, 0xe2, 0xc0) : theme::kInk);
        description_->setTextColor(hovered_ ? theme::rgb(0xc4, 0xb0, 0x8a) : theme::kInkBrown);
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
    if (!LayerColor::initWithColor(cocos2d::Color4B(4, 5, 6, 204))) {
        return false;
    }
    auto& loc = services_.localizer();
    GameServices* services = &services_;

    // A torn sheet, tilted a little; everything on it tilts with it.
    auto sheet = cocos2d::Node::create();
    sheet->setPosition(640, 360);
    sheet->setRotation(-0.6f);
    addChild(sheet);
    auto paper = decor::canvas();
    decor::tornSheet(paper, cocos2d::Rect(-320, -238, 640, 476), theme::kParchment, theme::kParchmentBurn);
    sheet->addChild(paper);

    auto title = LocalizedLabel::forKey(loc, TextKey::DifficultyTitle, 38.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(560.0f);
    title->setPosition(0, 181);
    sheet->addChild(title);

    const struct {
        TextKey name;
        TextKey description;
        Difficulty difficulty;
        int level;
        float y;
    } levels[] = {{TextKey::DifficultyEasy, TextKey::DifficultyEasyDesc, Difficulty::Easy, 1, 97.0f},
                  {TextKey::DifficultyMedium, TextKey::DifficultyMediumDesc, Difficulty::Medium, 2, 2.0f},
                  {TextKey::DifficultyHard, TextKey::DifficultyHardDesc, Difficulty::Hard, 3, -93.0f}};
    for (const auto& level : levels) {
        const Difficulty difficulty = level.difficulty;
        auto row = DifficultyRow::create(loc, level.name, level.description, level.level, [services, difficulty] {
            services->audio().onClick();
            services->navigator().showPlacement(difficulty);
        });
        row->setPosition(0, level.y);
        sheet->addChild(row);
    }

    auto back = TextButton::forKey(
        loc, TextKey::CommonBack, cocos2d::Size(200, 50),
        [this] {
            services_.audio().onClick();
            removeFromParent();
        },
        TextButton::Style::Secondary, 21.0f);
    back->setPosition(0, -180);
    sheet->addChild(back);

    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
