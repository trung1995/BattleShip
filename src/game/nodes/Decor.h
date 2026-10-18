#pragma once

#include <vector>

#include "cocos2d.h"

// Drawing helpers for the chart-table look: rounded and elliptical-cornered shapes, banded
// gradients, hatching, soft shadows and parchment panels. Everything draws into a DrawNode, so
// the engine needs no image assets. Pure drawing; no game logic.
namespace decor {

using Poly = std::vector<cocos2d::Vec2>;

// A DrawNode that blends the straight-alpha colours used throughout this file.
cocos2d::DrawNode* canvas();

// Corner radii as (horizontal, vertical); a zero radius gives a square corner.
struct Corners {
    cocos2d::Vec2 bottomLeft, bottomRight, topRight, topLeft;
    static Corners all(float r) { return {{r, r}, {r, r}, {r, r}, {r, r}}; }
};

Poly roundedRect(const cocos2d::Rect& rect, const Corners& corners);
inline Poly roundedRect(const cocos2d::Rect& rect, float radius) {
    return roundedRect(rect, Corners::all(radius));
}
// The part of `subject` inside the convex polygon `clip`.
Poly clipConvex(const Poly& subject, const Poly& clip);

void fill(cocos2d::DrawNode* draw, const Poly& shape, const cocos2d::Color4F& color);
void stroke(cocos2d::DrawNode* draw, const Poly& shape, float width, const cocos2d::Color4F& color);

cocos2d::Color4F mix(const cocos2d::Color4F& a, const cocos2d::Color4F& b, float t);

// Fills a convex shape with a gradient running from `from` (top, or left when horizontal) to
// `to`, passing through `middle` at `middleAt` (0..1).
void gradient(cocos2d::DrawNode* draw, const Poly& shape, const cocos2d::Color4F& from,
              const cocos2d::Color4F& middle, float middleAt, const cocos2d::Color4F& to,
              bool horizontal = false);
inline void gradient(cocos2d::DrawNode* draw, const Poly& shape, const cocos2d::Color4F& from,
                     const cocos2d::Color4F& to, bool horizontal = false) {
    gradient(draw, shape, from, mix(from, to, 0.5f), 0.5f, to, horizontal);
}

// Diagonal stripes inside a convex shape. `rising` stripes run bottom-left to top-right.
void hatch(cocos2d::DrawNode* draw, const Poly& shape, const cocos2d::Color4F& color, float period,
           float stripe, bool rising = false);

void ring(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius, float width,
          const cocos2d::Color4F& color, int segments = 32);
void crossMark(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float half, float width,
               const cocos2d::Color4F& color);
void dashedRect(cocos2d::DrawNode* draw, const cocos2d::Rect& rect, float dash, float gap, float width,
                const cocos2d::Color4F& color);

// A blurred shadow under a rounded rectangle, built from stacked translucent layers.
void softShadow(cocos2d::DrawNode* draw, const cocos2d::Rect& rect, float radius,
                const cocos2d::Vec2& offset, float blur, const cocos2d::Color4F& color);
// Darkens the inside edges of a rectangle (parchment burn, inset shadows).
void innerShadow(cocos2d::DrawNode* draw, const cocos2d::Rect& rect, float depth,
                 const cocos2d::Color4F& color);

// The tarred-wood table: a radial glow centred at 50% / 40% from the top.
void table(cocos2d::DrawNode* draw, const cocos2d::Size& size);
// The menu's night sea: dark teal diagonal stripes.
void nightSea(cocos2d::DrawNode* draw, const cocos2d::Size& size);

// Emblems drawn in place of artwork: a brass compass rose (menu), a gold star (victory) and a
// broken mast (defeat).
void compassRose(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);
void victoryEmblem(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);
void defeatEmblem(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);

struct PanelStyle {
    cocos2d::Color4F frame;
    cocos2d::Color4F frameEdge;
    cocos2d::Color4F paper;
    cocos2d::Color4F burn;
    bool brassFrame = false; // a gold gradient frame with a bright outline (victory)
};
PanelStyle woodPanel();
// A parchment sheet in a wooden frame. `outer` includes the frame; returns the parchment rect.
cocos2d::Rect panel(cocos2d::DrawNode* draw, const cocos2d::Rect& outer, float frame,
                    const PanelStyle& style, bool dropShadow = true, float burnDepth = 34.0f);

} // namespace decor
