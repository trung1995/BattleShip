#pragma once

#include <vector>

#include "cocos2d.h"

// Drawing helpers for the "Blood & Black Water" look: rounded and elliptical-cornered shapes, banded
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

// A storm sky: an elliptical glow of `light` at (`at.x`, `at.y`) as fractions of the size,
// measured from the bottom-left, fading into `deep` by `reach` (0..1 of the farthest corner).
void storm(cocos2d::DrawNode* draw, const cocos2d::Size& size, const cocos2d::Vec2& at,
           const cocos2d::Color4F& light, const cocos2d::Color4F& deep, float reach = 0.62f);
// The menu's rain-streaked night sea: dark teal diagonal stripes.
void rainSea(cocos2d::DrawNode* draw, const cocos2d::Size& size);

// A torn parchment sheet: ragged edges, a scorched inner burn and a heavy drop shadow. Draw it
// into its own node and rotate that node a little; nothing in this look sits straight.
void tornSheet(cocos2d::DrawNode* draw, const cocos2d::Rect& rect, const cocos2d::Color4F& paper,
               const cocos2d::Color4F& burn, float burnDepth = 44.0f);
// Scorched timber with a blood-red inlay (boards and the dock). `outer` includes the frame;
// returns the parchment rect inside it.
cocos2d::Rect timberFrame(cocos2d::DrawNode* draw, const cocos2d::Rect& outer, float frame = 10.0f,
                          bool dropShadow = true);
// The victory frame: tarnished gold with a bright outline and a glow. Returns the paper rect.
cocos2d::Rect goldFrame(cocos2d::DrawNode* draw, const cocos2d::Rect& outer, float frame = 10.0f);

// Emblems drawn in place of artwork: skull and crossed cutlasses (menu), a treasure chest
// (victory) and Davy Jones' drowned skull (defeat).
void skullAndCutlasses(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);
void treasureChest(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);
void drownedSkull(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, float radius);

// A small filled ellipse, optionally rotated (pips, the hit splatter).
Poly ellipse(const cocos2d::Vec2& centre, float rx, float ry, float rotationDegrees = 0.0f, int segments = 24);
// A row of `total` round pips, `filled` of them lit, centred on `centre` (Help uses).
void pips(cocos2d::DrawNode* draw, const cocos2d::Vec2& centre, int total, int filled, float diameter, float gap,
          const cocos2d::Color4F& on, const cocos2d::Color4F& onEdge, const cocos2d::Color4F& offEdge,
          const cocos2d::Color4F& glow);

} // namespace decor
