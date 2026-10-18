#pragma once

#include "bs/core/GameSession.h"
#include "cocos2d.h"

class BoardNode;

// The Help cross under the cursor, clipped at the board edges, with already-fired cells drawn
// as skipped (a dashed frame). Pure drawing; add it as a child of the target BoardNode.
class HelpPreviewNode final : public cocos2d::Node {
public:
    static HelpPreviewNode* create(const BoardNode& board);

    void show(bs::core::Coord centre, const bs::core::ITargetView& view);
    void hide();

private:
    explicit HelpPreviewNode(const BoardNode& board) : board_(board) {}

    const BoardNode& board_;
    cocos2d::DrawNode* draw_ = nullptr;
};
