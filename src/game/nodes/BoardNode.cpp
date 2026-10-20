#include "nodes/BoardNode.h"

#include <cmath>
#include <new>
#include <string>

#include "Theme.h"
#include "nodes/Decor.h"
#include "nodes/Hull.h"

using bs::core::Coord;
using bs::core::Knowledge;
using bs::core::kBoardSize;
using cocos2d::Rect;
using cocos2d::Vec2;

namespace {
std::size_t index(Coord c) { return static_cast<std::size_t>(c.row * kBoardSize + c.col); }
constexpr float kHeader = 22.0f; // the numbered strip above and left of the grid
} // namespace

BoardNode* BoardNode::create(float cellSize) {
    auto* node = new (std::nothrow) BoardNode(cellSize);
    if (node != nullptr && node->init()) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

bool BoardNode::init() {
    if (!Node::init()) {
        return false;
    }
    knowledge_.fill(Knowledge::Unknown);
    setContentSize(cocos2d::Size(cellSize_ * kBoardSize, cellSize_ * kBoardSize));
    frame_ = decor::canvas();
    addChild(frame_);
    draw_ = decor::canvas();
    addChild(draw_);
    blast_ = decor::canvas();
    addChild(blast_, 2);
    drawFrame();
    redraw();
    return true;
}

// The timber frame, the scorched parchment and both axes, numbered 1-10: numerals read the same
// in every language (ui-screens.md).
void BoardNode::drawFrame() {
    const float side = cellSize_ * kBoardSize;
    const Rect outer(-kFrameLeft, -kFrameBottom, side + kFrameLeft + kFrameRight, side + kFrameBottom + kFrameTop);
    decor::timberFrame(frame_, outer, 10.0f, true);
    for (int i = 0; i < kBoardSize; ++i) {
        const std::string text = std::to_string(i + 1);
        auto column = cocos2d::Label::createWithTTF(text, theme::fontFile(theme::Face::Caps), 14.0f);
        column->setTextColor(cocos2d::Color4B(theme::kAxis));
        column->setPosition(cellSize_ * (static_cast<float>(i) + 0.5f), side + kHeader / 2 + 1.0f);
        addChild(column);
        auto row = cocos2d::Label::createWithTTF(text, theme::fontFile(theme::Face::Caps), 14.0f);
        row->setTextColor(cocos2d::Color4B(theme::kAxis));
        row->setPosition(-kHeader / 2, cellSize_ * (static_cast<float>(kBoardSize - i) - 0.5f));
        addChild(row);
    }
}

void BoardNode::setFleet(const bs::core::Fleet* fleet) {
    fleet_ = fleet;
    redraw();
}

void BoardNode::setSelectedShip(bs::core::ShipId id) {
    if (id != selected_) {
        selected_ = id;
        redraw();
    }
}

void BoardNode::setKnowledge(const bs::core::ITargetView& view) {
    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            knowledge_[index({r, c})] = view.at({r, c});
        }
    }
    redraw();
}

void BoardNode::applyChanges(const std::vector<bs::core::CellChange>& changes, bs::core::Side owner) {
    bool changed = false;
    for (const auto& change : changes) {
        if (change.board == owner) {
            knowledge_[index(change.coord)] = change.now;
            changed = true;
        }
    }
    if (changed) {
        redraw();
    }
}

void BoardNode::setPreview(const std::vector<Coord>& cells, bool legal) {
    preview_ = cells;
    previewLegal_ = legal;
    redraw();
}

void BoardNode::clearPreview() {
    if (!preview_.empty()) {
        preview_.clear();
        redraw();
    }
}

void BoardNode::setHover(Coord cell) {
    if (!hovering_ || !(hover_ == cell)) {
        hovering_ = true;
        hover_ = cell;
        redraw();
    }
}

void BoardNode::clearHover() {
    if (hovering_) {
        hovering_ = false;
        redraw();
    }
}

cocos2d::Vec2 BoardNode::cellOrigin(Coord cell) const {
    return cocos2d::Vec2(cellSize_ * static_cast<float>(cell.col),
                         cellSize_ * static_cast<float>(kBoardSize - 1 - cell.row));
}

bool BoardNode::cellAt(const cocos2d::Vec2& worldPoint, Coord& out) const {
    const cocos2d::Vec2 local = convertToNodeSpace(worldPoint);
    const int col = static_cast<int>(std::floor(local.x / cellSize_));
    const int row = kBoardSize - 1 - static_cast<int>(std::floor(local.y / cellSize_));
    out = Coord{row, col};
    return out.isOnBoard() && local.x >= 0.0f && local.y >= 0.0f;
}

Knowledge BoardNode::knowledgeAt(Coord cell) const { return knowledge_[index(cell)]; }

void BoardNode::redraw() {
    draw_->clear();
    const float side = cellSize_ * kBoardSize;
    for (int i = 1; i <= kBoardSize; ++i) {
        const float p = cellSize_ * static_cast<float>(i);
        draw_->drawSolidRect(Vec2(p - 1.0f, 0.0f), Vec2(p, side), theme::kGridLine);
        draw_->drawSolidRect(Vec2(0.0f, side - p), Vec2(side, side - p + 1.0f), theme::kGridLine);
    }
    draw_->drawSolidRect(Vec2(-1.0f, 0.0f), Vec2(0.0f, side + 1.0f), theme::kGridEdge);
    draw_->drawSolidRect(Vec2(-1.0f, side), Vec2(side, side + 1.0f), theme::kGridEdge);

    drawHover();
    drawShips();
    drawPreview();
    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            drawMark({r, c}, knowledge_[index({r, c})]);
        }
    }
}

void BoardNode::drawShips() {
    if (fleet_ == nullptr) {
        drawWrecksFromKnowledge();
        return;
    }
    for (bs::core::ShipId id = 0; id < bs::core::kFleetSize; ++id) {
        const bs::core::ShipPlacement* p = fleet_->placementOf(id);
        if (p == nullptr) {
            continue;
        }
        bool sunk = true;
        for (const Coord c : p->cells()) {
            sunk = sunk && knowledgeAt(c) == Knowledge::Sunk;
        }
        const bool vertical = p->orientation == bs::core::Orientation::Vertical;
        const Coord last = p->cells().back();
        const Vec2 bottomLeft = cellOrigin(vertical ? last : p->origin);
        const Rect cells(bottomLeft.x, bottomLeft.y, cellSize_ * static_cast<float>(vertical ? 1 : p->length),
                         cellSize_ * static_cast<float>(vertical ? p->length : 1));
        const hull::State state = sunk ? hull::State::Wreck
                                       : (id == selected_ ? hull::State::Selected : hull::State::Intact);
        hull::draw(draw_, cells, p->length, vertical, state);
    }
}

// The opponent's ships are never known, but a sunk one is: its sunk cells form a line that
// touches no other ship, so each connected run of sunk cells is one wreck.
void BoardNode::drawWrecksFromKnowledge() {
    std::array<bool, kBoardSize * kBoardSize> seen{};
    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            if (seen[index({r, c})] || knowledgeAt({r, c}) != Knowledge::Sunk) {
                continue;
            }
            int length = 0;
            bool vertical = false;
            for (Coord at{r, c}; at.isOnBoard() && knowledgeAt(at) == Knowledge::Sunk; ++at.col) {
                seen[index(at)] = true;
                ++length;
            }
            if (length == 1) {
                for (Coord at{r + 1, c}; at.isOnBoard() && knowledgeAt(at) == Knowledge::Sunk; ++at.row) {
                    seen[index(at)] = true;
                    ++length;
                    vertical = true;
                }
            }
            const Vec2 bottomLeft = cellOrigin(vertical ? Coord{r + length - 1, c} : Coord{r, c});
            const Rect cells(bottomLeft.x, bottomLeft.y, cellSize_ * static_cast<float>(vertical ? 1 : length),
                             cellSize_ * static_cast<float>(vertical ? length : 1));
            hull::draw(draw_, cells, length, vertical, hull::State::Wreck);
        }
    }
}

void BoardNode::drawPreview() {
    for (const Coord c : preview_) {
        if (!c.isOnBoard()) {
            continue;
        }
        const Vec2 o = cellOrigin(c);
        const Rect cell(o.x, o.y, cellSize_ - 1.0f, cellSize_ - 1.0f);
        const decor::Poly shape = decor::roundedRect(cell, 0.0f);
        if (previewLegal_) {
            decor::fill(draw_, shape, theme::kPreviewLegal);
            decor::stroke(draw_, decor::roundedRect(Rect(o.x + 0.75f, o.y + 0.75f, cellSize_ - 2.5f, cellSize_ - 2.5f), 0.0f),
                          1.5f, theme::kPreviewLegalEdge);
        } else {
            decor::fill(draw_, shape, theme::kPreviewIllegalWash);
            decor::hatch(draw_, shape, theme::kPreviewIllegal, 8.0f, 4.0f);
            decor::stroke(draw_, decor::roundedRect(Rect(o.x + 0.75f, o.y + 0.75f, cellSize_ - 2.5f, cellSize_ - 2.5f), 0.0f),
                          1.5f, theme::kBloodDeep);
        }
    }
}

void BoardNode::drawHover() {
    if (!hovering_ || !hover_.isOnBoard()) {
        return;
    }
    const Vec2 o = cellOrigin(hover_);
    if (knowledgeAt(hover_) == Knowledge::Unknown) { // a red target frame in enemy waters
        draw_->drawSolidRect(o, o + Vec2(cellSize_ - 1.0f, cellSize_ - 1.0f), theme::kHoverWash);
        decor::stroke(draw_, decor::roundedRect(Rect(o.x + 1.0f, o.y + 1.0f, cellSize_ - 3.0f, cellSize_ - 3.0f), 0.0f),
                      2.0f, theme::kBlood);
    }
}

void BoardNode::strike(const std::vector<Coord>& cells) {
    constexpr float kDuration = 0.45f; // under 0.5 s, so the next computer shot never waits on it
    strike_ = cells;
    strikeAge_ = 0.0f;
    drawStrike(1.0f);
    schedule(
        [this](float dt) {
            strikeAge_ += dt;
            const float left = 1.0f - strikeAge_ / kDuration;
            drawStrike(left);
            if (left <= 0.0f) {
                unschedule("strike");
            }
        },
        "strike");
}

// An orange flare with an ember edge on each struck cell.
void BoardNode::drawStrike(float strength) {
    blast_->clear();
    if (strength <= 0.0f) {
        return;
    }
    const auto fade = [strength](const cocos2d::Color4F& c) { return cocos2d::Color4F(c.r, c.g, c.b, c.a * strength); };
    for (const Coord c : strike_) {
        if (!c.isOnBoard()) {
            continue;
        }
        const Vec2 o = cellOrigin(c);
        const Vec2 centre = o + Vec2((cellSize_ - 1.0f) / 2, (cellSize_ - 1.0f) / 2);
        blast_->drawSolidRect(o, o + Vec2(cellSize_ - 1.0f, cellSize_ - 1.0f), fade(theme::kBlastRim));
        blast_->drawSolidCircle(centre, cellSize_ * 0.42f, 0.0f, 24, fade(theme::rgba(255, 120, 50, 0.55f)));
        blast_->drawSolidCircle(centre, cellSize_ * 0.28f, 0.0f, 24, fade(theme::kBlastCore));
        decor::stroke(blast_, decor::roundedRect(Rect(o.x + 0.75f, o.y + 0.75f, cellSize_ - 2.5f, cellSize_ - 2.5f), 0.0f),
                      1.5f, fade(theme::kEmber));
    }
}

void BoardNode::drawMark(Coord cell, Knowledge knowledge) {
    const Vec2 o = cellOrigin(cell);
    const Vec2 centre = o + Vec2((cellSize_ - 1.0f) / 2, (cellSize_ - 1.0f) / 2);
    switch (knowledge) {
    case Knowledge::Unknown:
        break;
    case Knowledge::Miss: // a dark-teal ink ring: a splash in empty water
        draw_->drawSolidCircle(centre, 8.0f, 0.0f, 24, theme::kMissWash);
        decor::ring(draw_, centre, 7.0f, 2.0f, theme::kMissInk);
        break;
    case Knowledge::Marked: // a tiny faded dot next to a wreck, never mistaken for a miss
        draw_->drawSolidCircle(centre, 2.5f, 0.0f, 12, theme::kMarkedDot);
        break;
    case Knowledge::Hit: { // a blood splatter, slightly askew, with a cross
        decor::fill(draw_, decor::ellipse(centre, 18.0f, 17.0f, 8.0f), theme::rgba(179, 36, 27, 0.22f));
        decor::fill(draw_, decor::ellipse(centre, 15.5f, 14.5f, 8.0f), theme::kHitEdge);
        decor::fill(draw_, decor::ellipse(centre, 14.5f, 13.5f, 8.0f), theme::kBloodDeep);
        decor::fill(draw_, decor::ellipse(centre + Vec2(-1.5f, 1.5f), 11.0f, 10.0f, 8.0f), theme::kBlood);
        decor::fill(draw_, decor::ellipse(centre + Vec2(-3.0f, 3.0f), 6.5f, 6.0f, 8.0f), theme::kBloodBright);
        decor::crossMark(draw_, centre, 5.5f, 2.6f, theme::kHitCross);
        break;
    }
    case Knowledge::Sunk: // a red cross over the charred wreck
        decor::crossMark(draw_, centre, 6.0f, 2.8f, theme::kSunkCross);
        break;
    }
    if (hovering_ && cell == hover_ && knowledge != Knowledge::Unknown) {
        // "not allowed": a slashed ring over a cell that was already fired at
        decor::ring(draw_, centre, 10.0f, 2.0f, theme::kBlockedInk);
        draw_->drawSegment(centre + Vec2(-4.0f, -6.0f), centre + Vec2(4.0f, 6.0f), 1.0f, theme::kBlockedInk);
    }
}
