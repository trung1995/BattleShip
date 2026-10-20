#include "nodes/Hull.h"

#include "Theme.h"
#include "nodes/Decor.h"

using cocos2d::Rect;
using cocos2d::Vec2;

namespace hull {

void draw(cocos2d::DrawNode* draw, const Rect& cells, int length, bool vertical, State state) {
    const Rect box(cells.getMinX() + 4.0f, cells.getMinY() + 4.0f, cells.size.width - 8.0f,
                   cells.size.height - 8.0f);
    decor::Corners corners = decor::Corners::all(10.0f);
    if (length > 1 && vertical) {
        corners = {{6.0f, 6.0f}, {6.0f, 6.0f}, {14.0f, 20.0f}, {14.0f, 20.0f}}; // bow at the top
    } else if (length > 1) {
        corners = {{6.0f, 6.0f}, {20.0f, 14.0f}, {20.0f, 14.0f}, {6.0f, 6.0f}}; // bow to the right
    }
    const decor::Poly shape = decor::roundedRect(box, corners);

    const auto offsetShape = [&](const Vec2& by, float grow) {
        return decor::roundedRect(Rect(box.getMinX() + by.x - grow, box.getMinY() + by.y - grow,
                                       box.size.width + grow * 2.0f, box.size.height + grow * 2.0f),
                                  corners);
    };
    switch (state) {
    case State::Lifted: // held high above the chart: a long, soft shadow
        decor::fill(draw, offsetShape(Vec2(4.0f, -18.0f), 5.0f), theme::rgba(20, 10, 5, 0.2f));
        decor::fill(draw, offsetShape(Vec2(3.0f, -16.0f), 1.0f), theme::rgba(20, 10, 5, 0.3f));
        break;
    case State::Selected: // a blood-red ring and glow
        decor::fill(draw, offsetShape(Vec2::ZERO, 9.0f), theme::rgba(224, 64, 46, 0.18f));
        decor::fill(draw, offsetShape(Vec2::ZERO, 6.0f), theme::rgba(224, 64, 46, 0.32f));
        decor::fill(draw, offsetShape(Vec2(0.0f, -2.0f), 0.0f), theme::rgba(20, 10, 5, 0.45f));
        break;
    case State::Intact:
        decor::fill(draw, offsetShape(Vec2(0.0f, -2.0f), 0.0f), theme::rgba(20, 10, 5, 0.45f));
        break;
    case State::Wreck:
        break;
    }

    if (state == State::Wreck) {
        decor::fill(draw, shape, theme::kWreckDark);
        decor::hatch(draw, shape, theme::kWreckLight, 8.0f, 4.0f);
        decor::stroke(draw, shape, 1.0f, theme::rgba(0, 0, 0));
    } else {
        decor::gradient(draw, shape, theme::kHullTop, theme::kHullMid, 0.6f, theme::kHullBottom, vertical);
        decor::stroke(draw, shape, 1.0f, theme::kHullEdge);
        if (state == State::Selected) {
            decor::stroke(draw, offsetShape(Vec2::ZERO, 2.0f), 2.0f, theme::kEmber);
        }
    }

    const cocos2d::Color4F rivet = state == State::Wreck ? theme::kWreckRivet : theme::kRivet;
    const cocos2d::Color4F rivetEdge = state == State::Wreck ? theme::rgba(0, 0, 0) : theme::kRivetEdge;
    const float along = vertical ? box.size.height : box.size.width;
    for (int i = 0; i < length; ++i) {
        const float d = along * (static_cast<float>(i) + 0.5f) / static_cast<float>(length);
        const Vec2 centre = vertical ? Vec2(box.getMidX(), box.getMaxY() - d) : Vec2(box.getMinX() + d, box.getMidY());
        draw->drawSolidCircle(centre, 4.0f, 0.0f, 16, rivetEdge);
        draw->drawSolidCircle(centre, 3.0f, 0.0f, 16, rivet);
    }
}

} // namespace hull
