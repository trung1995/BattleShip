#include "nodes/HelpPreviewNode.h"

#include <new>

#include "Theme.h"
#include "nodes/BoardNode.h"
#include "nodes/Decor.h"

using cocos2d::Rect;
using cocos2d::Vec2;

HelpPreviewNode* HelpPreviewNode::create(const BoardNode& board) {
    auto* node = new (std::nothrow) HelpPreviewNode(board);
    if (node != nullptr && node->init()) {
        node->draw_ = decor::canvas();
        node->addChild(node->draw_);
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

// A brass cross; the centre is darker, and already-fired cells get a dashed "skipped" frame.
void HelpPreviewNode::show(bs::core::Coord centre, const bs::core::ITargetView& view) {
    draw_->clear();
    const float size = board_.cellSize() - 1.0f;
    const bs::core::Coord cross[] = {centre,
                                     {centre.row - 1, centre.col},
                                     {centre.row + 1, centre.col},
                                     {centre.row, centre.col - 1},
                                     {centre.row, centre.col + 1}};
    for (const bs::core::Coord c : cross) {
        if (!c.isOnBoard()) {
            continue; // clipped at the edge (FR-028)
        }
        const Vec2 o = board_.cellOrigin(c);
        if (view.at(c) != bs::core::Knowledge::Unknown) {
            draw_->drawSolidRect(o, o + Vec2(size, size), theme::kHelpSkipped);
            decor::dashedRect(draw_, Rect(o.x + 4.0f, o.y + 4.0f, size - 8.0f, size - 8.0f), 4.0f, 3.0f, 1.5f,
                              theme::kBrassDark);
        } else if (c == centre) {
            draw_->drawSolidRect(o, o + Vec2(size, size), theme::kHelpCentre);
            decor::stroke(draw_, decor::roundedRect(Rect(o.x + 1.0f, o.y + 1.0f, size - 2.0f, size - 2.0f), 0.0f),
                          2.0f, theme::kBrassDark);
        } else {
            draw_->drawSolidRect(o, o + Vec2(size, size), theme::kHelpFill);
            decor::stroke(draw_, decor::roundedRect(Rect(o.x + 0.75f, o.y + 0.75f, size - 1.5f, size - 1.5f), 0.0f),
                          1.5f, theme::kHelpEdge);
        }
    }
    setVisible(true);
}

void HelpPreviewNode::hide() {
    draw_->clear();
    setVisible(false);
}
