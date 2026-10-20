#include "nodes/DockNode.h"

#include <new>
#include <string>

#include "Theme.h"
#include "bs/app/TextKeys.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/ShipNode.h"

namespace {
constexpr float kWidth = 540.0f;
constexpr float kHeight = 193.0f;
constexpr float kContentLeft = 32.0f;
constexpr float kColumnWidth = 226.0f;
constexpr float kColumnGap = 24.0f;
constexpr float kRowHeight = 40.0f;
constexpr float kRowCentres[] = {100.0f, 48.0f};
int lengthOfSlot(int slot) { return 4 - slot; }
} // namespace

DockNode* DockNode::create(bs::app::ILocalizer& localizer, float cellSize) {
    auto* node = new (std::nothrow) DockNode(localizer, cellSize);
    if (node != nullptr && node->initDock()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

cocos2d::Rect DockNode::slotRect(int slot) const {
    const float x = kContentLeft + static_cast<float>(slot % 2) * (kColumnWidth + kColumnGap);
    const float centre = kRowCentres[slot / 2];
    return cocos2d::Rect(x, centre - kRowHeight / 2, kColumnWidth, kRowHeight);
}

bool DockNode::initDock() {
    if (!Node::init()) {
        return false;
    }
    setContentSize(cocos2d::Size(kWidth, kHeight));
    auto sheet = decor::canvas();
    decor::timberFrame(sheet, cocos2d::Rect(0.0f, 0.0f, kWidth, kHeight), 10.0f, true);
    sheet->drawSolidRect(cocos2d::Vec2(kContentLeft, 130.0f), cocos2d::Vec2(kWidth - kContentLeft, 131.0f),
                         theme::kParchmentRule);
    addChild(sheet);

    auto title = LocalizedLabel::forKey(localizer_, bs::app::TextKey::PlacementDock, 23.0f, theme::Face::Caps);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(kWidth - kContentLeft * 2 - 80.0f);
    title->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    title->setPosition(kContentLeft, 151.0f);
    addChild(title);

    // Numerals only, which read the same in every language.
    progress_ = cocos2d::Label::createWithTTF("", theme::fontFile(theme::Face::Body), 17.0f);
    progress_->setTextColor(cocos2d::Color4B(theme::kInkRed));
    progress_->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
    progress_->setPosition(kWidth - kContentLeft, 149.0f);
    addChild(progress_);

    for (int slot = 0; slot < kSlots; ++slot) {
        const cocos2d::Rect r = slotRect(slot);
        const int length = lengthOfSlot(slot);
        shapes_[slot] = ShipNode::create(length, bs::core::Orientation::Horizontal, cellSize_);
        shapes_[slot]->setCascadeOpacityEnabled(true);
        shapes_[slot]->setPosition(r.getMinX(), r.getMidY() - cellSize_ / 2);
        addChild(shapes_[slot]);

        bs::app::ILocalizer* source = &localizer_;
        int* remaining = &remaining_[slot];
        counts_[slot] = LocalizedLabel::create(
            localizer_,
            [source, remaining] {
                return source->format(bs::app::TextKey::PlacementDockCount, {{"n", std::to_string(*remaining)}});
            },
            22.0f, theme::Face::Caps);
        counts_[slot]->setTextColor(theme::rgb(0x5e, 0x16, 0x0c));
        counts_[slot]->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
        counts_[slot]->setPosition(r.getMinX() + cellSize_ * static_cast<float>(length) + 14.0f, r.getMidY());
        addChild(counts_[slot]);
    }
    return true;
}

void DockNode::setShips(const std::vector<bs::core::ShipId>& unplaced) {
    unplaced_ = unplaced;
    for (int slot = 0; slot < kSlots; ++slot) remaining_[slot] = 0;
    for (const bs::core::ShipId id : unplaced_) {
        ++remaining_[4 - bs::core::fleetSpec()[static_cast<std::size_t>(id)]];
    }
    for (int slot = 0; slot < kSlots; ++slot) {
        const GLubyte opacity = remaining_[slot] > 0 ? 255 : 90;
        shapes_[slot]->setOpacity(opacity);
        counts_[slot]->setOpacity(opacity);
        counts_[slot]->refresh();
    }
    const int total = bs::core::kFleetSize;
    progress_->setString(std::to_string(total - static_cast<int>(unplaced_.size())) + " / " + std::to_string(total));
}

bs::core::ShipId DockNode::shipAt(const cocos2d::Vec2& worldPoint) const {
    const cocos2d::Vec2 local = convertToNodeSpace(worldPoint);
    for (int slot = 0; slot < kSlots; ++slot) {
        if (!slotRect(slot).containsPoint(local)) {
            continue;
        }
        for (const bs::core::ShipId id : unplaced_) {
            if (bs::core::fleetSpec()[static_cast<std::size_t>(id)] == lengthOfSlot(slot)) {
                return id;
            }
        }
    }
    return -1;
}

bool DockNode::contains(const cocos2d::Vec2& worldPoint) const {
    const cocos2d::Vec2 local = convertToNodeSpace(worldPoint);
    return cocos2d::Rect(0.0f, 0.0f, kWidth, kHeight).containsPoint(local);
}
