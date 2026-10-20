#pragma once

#include <array>
#include <vector>

#include "bs/core/GameSession.h"
#include "cocos2d.h"

// Draws one 10 x 10 sea chart: a scorched parchment grid in a timber frame with numbered axes,
// optional own ships, shot knowledge, a placement preview, a target hover and the Help strike. Pure drawing; it makes no
// game decisions. The node's origin is the grid's bottom-left corner; the frame extends beyond.
class BoardNode final : public cocos2d::Node {
public:
    static BoardNode* create(float cellSize);

    // The frame around the grid: how far it reaches left, below, right and above the grid.
    static constexpr float kFrameLeft = 36.0f;
    static constexpr float kFrameBottom = 20.0f;
    static constexpr float kFrameRight = 21.0f;
    static constexpr float kFrameTop = 38.0f;

    // Draw these ships (the owner's own board). nullptr hides ships.
    void setFleet(const bs::core::Fleet* fleet);
    void setSelectedShip(bs::core::ShipId id); // -1: none
    void setKnowledge(const bs::core::ITargetView& view);
    void applyChanges(const std::vector<bs::core::CellChange>& changes, bs::core::Side owner);
    void setPreview(const std::vector<bs::core::Coord>& cells, bool legal);
    void clearPreview();
    void setHover(bs::core::Coord cell); // a red target ring, or "not allowed" on a fired cell
    void clearHover();
    // Every cell of a Help cross flares at once, then fades to reveal its result (FR-048).
    void strike(const std::vector<bs::core::Coord>& cells);
    void redraw();

    // Maps a world position (e.g. a mouse click) to a board cell.
    bool cellAt(const cocos2d::Vec2& worldPoint, bs::core::Coord& out) const;
    cocos2d::Vec2 cellOrigin(bs::core::Coord cell) const; // bottom-left, local space
    float cellSize() const { return cellSize_; }

private:
    explicit BoardNode(float cellSize) : cellSize_(cellSize) {}
    bool init() override;
    void drawFrame();
    void drawShips();
    void drawWrecksFromKnowledge();
    void drawPreview();
    void drawHover();
    void drawMark(bs::core::Coord cell, bs::core::Knowledge knowledge);
    void drawStrike(float strength);
    bs::core::Knowledge knowledgeAt(bs::core::Coord cell) const;

    float cellSize_;
    cocos2d::DrawNode* frame_ = nullptr;
    cocos2d::DrawNode* draw_ = nullptr;
    cocos2d::DrawNode* blast_ = nullptr;
    std::vector<bs::core::Coord> strike_;
    float strikeAge_ = 0.0f;
    const bs::core::Fleet* fleet_ = nullptr;
    bs::core::ShipId selected_ = -1;
    std::array<bs::core::Knowledge, bs::core::kBoardSize * bs::core::kBoardSize> knowledge_{};
    std::vector<bs::core::Coord> preview_;
    bool previewLegal_ = true;
    bool hovering_ = false;
    bs::core::Coord hover_{0, 0};
};
