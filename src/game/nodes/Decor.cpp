#include "nodes/Decor.h"

#include <algorithm>
#include <cmath>

#include "Theme.h"

using cocos2d::Color4F;
using cocos2d::Rect;
using cocos2d::Vec2;

namespace decor {
namespace {

constexpr float kPi = 3.14159265f;
constexpr float kRoot2 = 1.41421356f;
constexpr int kArcSegments = 6;

float cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }

float signedArea(const Poly& p) {
    float area = 0.0f;
    for (std::size_t i = 0; i < p.size(); ++i) {
        area += cross(p[i], p[(i + 1) % p.size()]);
    }
    return area / 2.0f;
}

Rect bounds(const Poly& p) {
    float x0 = p.front().x, x1 = x0, y0 = p.front().y, y1 = y0;
    for (const Vec2& v : p) {
        x0 = std::min(x0, v.x);
        x1 = std::max(x1, v.x);
        y0 = std::min(y0, v.y);
        y1 = std::max(y1, v.y);
    }
    return Rect(x0, y0, x1 - x0, y1 - y0);
}

Poly rectPoly(const Rect& r) {
    return {Vec2(r.getMinX(), r.getMinY()), Vec2(r.getMaxX(), r.getMinY()), Vec2(r.getMaxX(), r.getMaxY()),
            Vec2(r.getMinX(), r.getMaxY())};
}

void arc(Poly& out, const Vec2& centre, const Vec2& radius, float from, float to) {
    if (radius.x <= 0.0f || radius.y <= 0.0f) {
        out.push_back(centre);
        return;
    }
    for (int i = 0; i <= kArcSegments; ++i) {
        const float a = from + (to - from) * static_cast<float>(i) / static_cast<float>(kArcSegments);
        out.emplace_back(centre.x + radius.x * std::cos(a), centre.y + radius.y * std::sin(a));
    }
}

Color4F colourAt(float t, const Color4F& from, const Color4F& middle, float middleAt, const Color4F& to) {
    if (t <= middleAt) {
        return mix(from, middle, middleAt > 0.0f ? t / middleAt : 1.0f);
    }
    return mix(middle, to, middleAt < 1.0f ? (t - middleAt) / (1.0f - middleAt) : 1.0f);
}

} // namespace

cocos2d::DrawNode* canvas() {
    auto* draw = cocos2d::DrawNode::create();
    draw->setBlendFunc(cocos2d::BlendFunc::ALPHA_NON_PREMULTIPLIED); // colours here are straight alpha
    return draw;
}

Poly roundedRect(const Rect& rect, const Corners& c) {
    const float x0 = rect.getMinX(), x1 = rect.getMaxX(), y0 = rect.getMinY(), y1 = rect.getMaxY();
    Poly out;
    arc(out, Vec2(x1 - c.bottomRight.x, y0 + c.bottomRight.y), c.bottomRight, -kPi / 2, 0.0f);
    arc(out, Vec2(x1 - c.topRight.x, y1 - c.topRight.y), c.topRight, 0.0f, kPi / 2);
    arc(out, Vec2(x0 + c.topLeft.x, y1 - c.topLeft.y), c.topLeft, kPi / 2, kPi);
    arc(out, Vec2(x0 + c.bottomLeft.x, y0 + c.bottomLeft.y), c.bottomLeft, kPi, kPi * 1.5f);
    return out;
}

Poly clipConvex(const Poly& subject, const Poly& clip) {
    if (subject.size() < 3 || clip.size() < 3) {
        return {};
    }
    const float orientation = signedArea(clip) >= 0.0f ? 1.0f : -1.0f;
    Poly output = subject;
    for (std::size_t i = 0; i < clip.size() && !output.empty(); ++i) {
        const Vec2 a = clip[i];
        const Vec2 b = clip[(i + 1) % clip.size()];
        const auto inside = [&](const Vec2& p) { return cross(b - a, p - a) * orientation >= 0.0f; };
        const Poly input = output;
        output.clear();
        for (std::size_t j = 0; j < input.size(); ++j) {
            const Vec2 p = input[j];
            const Vec2 q = input[(j + 1) % input.size()];
            const bool pIn = inside(p);
            const bool qIn = inside(q);
            if (pIn) {
                output.push_back(p);
            }
            if (pIn != qIn) {
                const Vec2 d = q - p;
                const float denom = cross(b - a, d);
                if (std::fabs(denom) > 1e-6f) {
                    const float t = cross(b - a, a - p) / denom;
                    output.push_back(p + d * t);
                }
            }
        }
    }
    return output;
}

void fill(cocos2d::DrawNode* draw, const Poly& shape, const Color4F& color) {
    if (shape.size() >= 3) {
        draw->drawSolidPoly(shape.data(), static_cast<unsigned int>(shape.size()), color);
    }
}

void stroke(cocos2d::DrawNode* draw, const Poly& shape, float width, const Color4F& color) {
    for (std::size_t i = 0; i < shape.size(); ++i) {
        draw->drawSegment(shape[i], shape[(i + 1) % shape.size()], width / 2.0f, color);
    }
}

Color4F mix(const Color4F& a, const Color4F& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return Color4F(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t);
}

void gradient(cocos2d::DrawNode* draw, const Poly& shape, const Color4F& from, const Color4F& middle,
              float middleAt, const Color4F& to, bool horizontal) {
    if (shape.size() < 3) {
        return;
    }
    const Rect box = bounds(shape);
    const float length = horizontal ? box.size.width : box.size.height;
    constexpr float kBand = 2.0f;
    for (float d = 0.0f; d < length; d += kBand) {
        const float extent = std::min(kBand, length - d);
        // Overlap each band by half a pixel so no seams show between them.
        const Rect band = horizontal ? Rect(box.getMinX() + d, box.getMinY(), extent + 0.5f, box.size.height)
                                     : Rect(box.getMinX(), box.getMaxY() - d - extent - 0.5f, box.size.width,
                                            extent + 0.5f);
        const float t = (d + extent / 2.0f) / length;
        fill(draw, clipConvex(rectPoly(band), shape), colourAt(t, from, middle, middleAt, to));
    }
}

void hatch(cocos2d::DrawNode* draw, const Poly& shape, const Color4F& color, float period, float stripe,
           bool rising) {
    if (shape.size() < 3) {
        return;
    }
    const Rect box = bounds(shape);
    const float k = rising ? -1.0f : 1.0f; // stripes are lines of constant x + k*y
    const float sPeriod = period * kRoot2;
    const float sStripe = stripe * kRoot2;
    const float corners[] = {box.getMinX() + k * box.getMinY(), box.getMaxX() + k * box.getMinY(),
                             box.getMinX() + k * box.getMaxY(), box.getMaxX() + k * box.getMaxY()};
    const float sMin = *std::min_element(std::begin(corners), std::end(corners));
    const float sMax = *std::max_element(std::begin(corners), std::end(corners));
    const float x0 = box.getMinX(), x1 = box.getMaxX();
    for (float s = std::floor(sMin / sPeriod) * sPeriod; s < sMax; s += sPeriod) {
        const Poly band = {Vec2(x0, (s - x0) / k), Vec2(x1, (s - x1) / k), Vec2(x1, (s + sStripe - x1) / k),
                           Vec2(x0, (s + sStripe - x0) / k)};
        fill(draw, clipConvex(band, shape), color);
    }
}

void ring(cocos2d::DrawNode* draw, const Vec2& centre, float radius, float width, const Color4F& color,
          int segments) {
    const float outer = radius + width / 2.0f;
    const float inner = std::max(0.0f, radius - width / 2.0f);
    for (int i = 0; i < segments; ++i) {
        const float a = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(segments);
        const float b = 2.0f * kPi * static_cast<float>(i + 1) / static_cast<float>(segments);
        const Vec2 quad[] = {centre + Vec2(std::cos(a), std::sin(a)) * outer,
                             centre + Vec2(std::cos(b), std::sin(b)) * outer,
                             centre + Vec2(std::cos(b), std::sin(b)) * inner,
                             centre + Vec2(std::cos(a), std::sin(a)) * inner};
        draw->drawSolidPoly(quad, 4, color);
    }
}

void crossMark(cocos2d::DrawNode* draw, const Vec2& centre, float half, float width, const Color4F& color) {
    draw->drawSegment(centre + Vec2(-half, -half), centre + Vec2(half, half), width / 2.0f, color);
    draw->drawSegment(centre + Vec2(-half, half), centre + Vec2(half, -half), width / 2.0f, color);
}

void dashedRect(cocos2d::DrawNode* draw, const Rect& rect, float dash, float gap, float width,
                const Color4F& color) {
    const Vec2 corners[] = {Vec2(rect.getMinX(), rect.getMinY()), Vec2(rect.getMaxX(), rect.getMinY()),
                            Vec2(rect.getMaxX(), rect.getMaxY()), Vec2(rect.getMinX(), rect.getMaxY())};
    for (int side = 0; side < 4; ++side) {
        const Vec2 a = corners[side];
        const Vec2 b = corners[(side + 1) % 4];
        const float length = a.distance(b);
        const Vec2 dir = (b - a) / length;
        const Vec2 normal(-dir.y * width / 2.0f, dir.x * width / 2.0f);
        for (float d = 0.0f; d < length; d += dash + gap) {
            const Vec2 p = a + dir * d;
            const Vec2 q = a + dir * std::min(d + dash, length);
            const Vec2 quad[] = {p - normal, q - normal, q + normal, p + normal};
            draw->drawSolidPoly(quad, 4, color);
        }
    }
}

void softShadow(cocos2d::DrawNode* draw, const Rect& rect, float radius, const Vec2& offset, float blur,
                const Color4F& color) {
    constexpr int kLayers = 8;
    const Color4F layer(color.r, color.g, color.b, color.a / static_cast<float>(kLayers) * 1.4f);
    for (int i = 0; i < kLayers; ++i) {
        const float grow = blur * (-0.2f + 0.7f * static_cast<float>(i) / static_cast<float>(kLayers - 1));
        const Rect r(rect.getMinX() + offset.x - grow, rect.getMinY() + offset.y - grow,
                     rect.size.width + grow * 2.0f, rect.size.height + grow * 2.0f);
        fill(draw, roundedRect(r, std::max(radius, radius + grow)), layer);
    }
}

void innerShadow(cocos2d::DrawNode* draw, const Rect& rect, float depth, const Color4F& color) {
    const int kSteps = std::max(12, static_cast<int>(depth / 1.5f)); // fine steps, so no bands show
    const float step = depth / static_cast<float>(kSteps);
    for (int i = 0; i < kSteps; ++i) {
        const float fade = 1.0f - (static_cast<float>(i) + 0.5f) / static_cast<float>(kSteps);
        const Color4F c(color.r, color.g, color.b, color.a * fade * fade);
        const float in = step * static_cast<float>(i);
        const float x0 = rect.getMinX() + in, x1 = rect.getMaxX() - in;
        const float y0 = rect.getMinY() + in, y1 = rect.getMaxY() - in;
        if (x1 - x0 <= 2.0f * step || y1 - y0 <= 2.0f * step) {
            break;
        }
        draw->drawSolidRect(Vec2(x0, y1 - step), Vec2(x1, y1), c);
        draw->drawSolidRect(Vec2(x0, y0), Vec2(x1, y0 + step), c);
        draw->drawSolidRect(Vec2(x0, y0 + step), Vec2(x0 + step, y1 - step), c);
        draw->drawSolidRect(Vec2(x1 - step, y0 + step), Vec2(x1, y1 - step), c);
    }
}

void storm(cocos2d::DrawNode* draw, const cocos2d::Size& size, const Vec2& at, const Color4F& light,
           const Color4F& deep, float reach) {
    draw->drawSolidRect(Vec2::ZERO, Vec2(size), deep);
    const Vec2 centre(size.width * at.x, size.height * at.y);
    // An ellipse sized to the farthest corner, as a CSS "ellipse farthest-corner" gradient.
    const float dx = std::max(centre.x, size.width - centre.x);
    const float dy = std::max(centre.y, size.height - centre.y);
    const float rx = dx * kRoot2;
    const float ry = dy * kRoot2;
    constexpr int kSteps = 40;
    for (int i = kSteps; i >= 1; --i) {
        const float f = reach * static_cast<float>(i) / static_cast<float>(kSteps);
        draw->drawSolidCircle(centre, rx * f, 0.0f, 96, 1.0f, ry / rx, mix(light, deep, f / reach));
    }
}

void rainSea(cocos2d::DrawNode* draw, const cocos2d::Size& size) {
    draw->drawSolidRect(Vec2::ZERO, Vec2(size), theme::kStripeDark);
    hatch(draw, rectPoly(Rect(Vec2::ZERO, size)), theme::kStripeLight, 28.0f, 14.0f, true);
}

namespace {
// The ragged edge of a torn sheet, as fractions along each side and how far each point bites in
// (fractions of the side's height or width), matching the mockup's clip path.
struct Tear {
    float at;
    float bite;
};
const Tear kTopTear[] = {{0.0f, 0.015f}, {0.06f, 0.0f}, {0.14f, 0.012f}, {0.23f, 0.002f}, {0.31f, 0.016f},
                         {0.42f, 0.0f}, {0.55f, 0.014f}, {0.63f, 0.003f}, {0.74f, 0.017f}, {0.86f, 0.002f},
                         {0.94f, 0.013f}, {1.0f, 0.0f}};
const Tear kBottomTear[] = {{0.0f, 0.012f}, {0.10f, 0.0f}, {0.21f, 0.013f}, {0.33f, 0.002f}, {0.45f, 0.015f},
                            {0.57f, 0.0f}, {0.68f, 0.012f}, {0.80f, 0.0f}, {0.91f, 0.014f}, {1.0f, 0.0f}};
const Tear kSideTear[] = {{0.0f, 0.0f}, {0.23f, 0.006f}, {0.49f, 0.0f}, {0.74f, 0.007f}, {1.0f, 0.0f}};
const Tear kOtherSideTear[] = {{0.0f, 0.0f}, {0.22f, 0.006f}, {0.47f, 0.0f}, {0.71f, 0.008f}, {1.0f, 0.0f}};
constexpr float kMaxBite = 0.017f;
} // namespace

void tornSheet(cocos2d::DrawNode* draw, const Rect& rect, const Color4F& paper, const Color4F& burn,
               float burnDepth) {
    softShadow(draw, rect, 2.0f, Vec2(0.0f, -30.0f), 60.0f, theme::rgba(0, 0, 0, 0.75f));
    const float x0 = rect.getMinX(), x1 = rect.getMaxX(), y0 = rect.getMinY(), y1 = rect.getMaxY();
    const float w = rect.size.width, h = rect.size.height;
    const float inTop = h * kMaxBite, inSide = w * kMaxBite;
    const Rect inner(x0 + inSide, y0 + inTop, w - inSide * 2.0f, h - inTop * 2.0f);
    draw->drawSolidRect(inner.origin, Vec2(inner.getMaxX(), inner.getMaxY()), paper);
    // The ragged rim is as scorched as the outermost edge of the burn inside it.
    const Color4F edgeColour = mix(paper, Color4F(burn.r, burn.g, burn.b, 1.0f), burn.a * 0.9f);
    // Each ragged edge is a strip of convex quads between the tear and the inner rectangle.
    const auto strip = [&](const Tear* tear, std::size_t n, bool horizontal, bool far) {
        for (std::size_t i = 0; i + 1 < n; ++i) {
            const Tear a = tear[i], b = tear[i + 1];
            Poly quad;
            if (horizontal) {
                const float edge = far ? y1 : y0, sign = far ? -1.0f : 1.0f, base = far ? inner.getMaxY() : inner.getMinY();
                const float ax = x0 + w * a.at, bx = x0 + w * b.at;
                quad = {Vec2(ax, base), Vec2(bx, base), Vec2(bx, edge + sign * h * b.bite), Vec2(ax, edge + sign * h * a.bite)};
            } else {
                const float edge = far ? x1 : x0, sign = far ? -1.0f : 1.0f, base = far ? inner.getMaxX() : inner.getMinX();
                const float ay = y1 - h * a.at, by = y1 - h * b.at;
                quad = {Vec2(base, ay), Vec2(base, by), Vec2(edge + sign * w * b.bite, by), Vec2(edge + sign * w * a.bite, ay)};
            }
            fill(draw, quad, edgeColour);
        }
    };
    strip(kTopTear, sizeof(kTopTear) / sizeof(Tear), true, true);
    strip(kBottomTear, sizeof(kBottomTear) / sizeof(Tear), true, false);
    strip(kSideTear, sizeof(kSideTear) / sizeof(Tear), false, false);
    strip(kOtherSideTear, sizeof(kOtherSideTear) / sizeof(Tear), false, true);
    innerShadow(draw, inner, burnDepth, burn);
}

Rect timberFrame(cocos2d::DrawNode* draw, const Rect& outer, float frame, bool dropShadow) {
    if (dropShadow) {
        softShadow(draw, outer, 3.0f, Vec2(0.0f, -22.0f), 50.0f, theme::rgba(0, 0, 0, 0.75f));
    }
    const Poly shape = roundedRect(outer, 3.0f);
    gradient(draw, shape, theme::kFrameTop, theme::kFrameBottom);
    const auto inset = [&](float by) {
        return roundedRect(Rect(outer.getMinX() + by, outer.getMinY() + by, outer.size.width - by * 2.0f,
                                outer.size.height - by * 2.0f),
                           std::max(0.0f, 3.0f - by));
    };
    stroke(draw, inset(0.5f), 1.0f, theme::kFrameLine);
    stroke(draw, inset(2.0f), 2.0f, theme::kFrameBottom);
    stroke(draw, inset(3.5f), 1.0f, theme::kFrameInlay);
    const Rect paper(outer.getMinX() + frame, outer.getMinY() + frame, outer.size.width - frame * 2.0f,
                     outer.size.height - frame * 2.0f);
    draw->drawSolidRect(paper.origin, Vec2(paper.getMaxX(), paper.getMaxY()), theme::kParchment);
    innerShadow(draw, paper, 38.0f, theme::kParchmentBurn);
    return paper;
}

Rect goldFrame(cocos2d::DrawNode* draw, const Rect& outer, float frame) {
    softShadow(draw, outer, 3.0f, Vec2::ZERO, 80.0f, theme::rgba(224, 176, 78, 0.4f));
    softShadow(draw, outer, 3.0f, Vec2(0.0f, -30.0f), 80.0f, theme::rgba(0, 0, 0, 0.8f));
    const Poly shape = roundedRect(outer, 3.0f);
    gradient(draw, shape, theme::kGoldTarnish, theme::kGoldDark);
    stroke(draw, roundedRect(Rect(outer.getMinX() - 1.0f, outer.getMinY() - 1.0f, outer.size.width + 2.0f,
                                  outer.size.height + 2.0f),
                             4.0f),
           2.0f, theme::kGoldEdge);
    const Rect paper(outer.getMinX() + frame, outer.getMinY() + frame, outer.size.width - frame * 2.0f,
                     outer.size.height - frame * 2.0f);
    draw->drawSolidRect(paper.origin, Vec2(paper.getMaxX(), paper.getMaxY()), theme::rgba(0xe3, 0xcf, 0xa2));
    innerShadow(draw, paper, 36.0f, theme::rgba(90, 50, 10, 0.6f));
    return paper;
}

Poly ellipse(const Vec2& centre, float rx, float ry, float rotationDegrees, int segments) {
    const float turn = rotationDegrees * kPi / 180.0f;
    const float c = std::cos(turn), s = std::sin(turn);
    Poly out;
    for (int i = 0; i < segments; ++i) {
        const float a = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(segments);
        const Vec2 p(rx * std::cos(a), ry * std::sin(a));
        out.emplace_back(centre.x + p.x * c - p.y * s, centre.y + p.x * s + p.y * c);
    }
    return out;
}

void pips(cocos2d::DrawNode* draw, const Vec2& centre, int total, int filled, float diameter, float gap,
          const Color4F& on, const Color4F& onEdge, const Color4F& offEdge, const Color4F& glow) {
    const float span = diameter * static_cast<float>(total) + gap * static_cast<float>(total - 1);
    for (int i = 0; i < total; ++i) {
        const Vec2 c(centre.x - span / 2 + diameter / 2 + (diameter + gap) * static_cast<float>(i), centre.y);
        const float r = diameter / 2;
        if (i < filled) {
            if (glow.a > 0.0f) {
                draw->drawSolidCircle(c, r + 4.0f, 0.0f, 20, Color4F(glow.r, glow.g, glow.b, glow.a * 0.35f));
                draw->drawSolidCircle(c, r + 2.0f, 0.0f, 20, Color4F(glow.r, glow.g, glow.b, glow.a * 0.5f));
            }
            draw->drawSolidCircle(c, r, 0.0f, 20, onEdge);
            draw->drawSolidCircle(c, r - 1.0f, 0.0f, 20, on);
        } else {
            ring(draw, c, r - 0.75f, 1.5f, offEdge, 20);
        }
    }
}

namespace {
const Color4F kBone = theme::rgba(0xe9, 0xdc, 0xc0);

void cutlass(cocos2d::DrawNode* draw, const Vec2& hilt, float angle, float length, const Color4F& steel,
             const Color4F& brass) {
    const Vec2 dir(std::cos(angle), std::sin(angle));
    const Vec2 side(-dir.y, dir.x);
    // A curved blade: a fan of quads that bows towards `side` as it nears the tip.
    constexpr int kSegments = 8;
    for (int i = 0; i < kSegments; ++i) {
        const float t0 = static_cast<float>(i) / kSegments, t1 = static_cast<float>(i + 1) / kSegments;
        const auto at = [&](float t, float halfWidth) {
            const float bow = length * 0.12f * t * t;
            return hilt + dir * (length * t) + side * (bow + halfWidth);
        };
        const float w0 = 5.0f * (1.0f - t0 * 0.6f), w1 = 5.0f * (1.0f - t1 * 0.6f);
        const Poly quad = {at(t0, -w0 * 0.3f), at(t1, -w1 * 0.3f), at(t1, w1), at(t0, w0)};
        fill(draw, quad, steel);
    }
    draw->drawSegment(hilt - side * 9.0f, hilt + side * 9.0f, 2.2f, brass); // the guard
    draw->drawSegment(hilt, hilt - dir * 13.0f, 2.6f, theme::rgba(0x3a, 0x26, 0x18)); // the grip
    draw->drawSolidCircle(hilt - dir * 14.0f, 3.2f, 0.0f, 12, brass);
}

void skull(cocos2d::DrawNode* draw, const Vec2& centre, float size, const Color4F& bone, const Color4F& socket) {
    for (const float grow : {2.0f, 0.0f}) { // a dark outline first, so the bone reads on any ground
        const Color4F ink = grow > 0.0f ? socket : bone;
        fill(draw, ellipse(centre + Vec2(0.0f, size * 0.18f), size * 0.62f + grow, size * 0.56f + grow), ink); // cranium
        fill(draw,
             roundedRect(Rect(centre.x - size * 0.36f - grow, centre.y - size * 0.5f - grow, size * 0.72f + grow * 2.0f,
                              size * 0.5f + grow * 2.0f),
                         size * 0.12f),
             ink); // jaw
    }
    fill(draw, ellipse(centre + Vec2(-size * 0.24f, size * 0.08f), size * 0.17f, size * 0.19f), socket);
    fill(draw, ellipse(centre + Vec2(size * 0.24f, size * 0.08f), size * 0.17f, size * 0.19f), socket);
    const Vec2 nose[] = {centre + Vec2(0.0f, -size * 0.06f), centre + Vec2(-size * 0.08f, -size * 0.2f),
                         centre + Vec2(size * 0.08f, -size * 0.2f)};
    draw->drawSolidPoly(nose, 3, socket);
    for (int i = -2; i <= 2; ++i) { // teeth
        const float x = centre.x + static_cast<float>(i) * size * 0.11f;
        draw->drawSegment(Vec2(x, centre.y - size * 0.3f), Vec2(x, centre.y - size * 0.46f), 0.8f, socket);
    }
}
} // namespace

void skullAndCutlasses(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    draw->drawSolidCircle(centre, radius, 0.0f, 64, theme::rgba(7, 9, 10, 0.75f));
    ring(draw, centre, radius - 1.0f, 2.0f, theme::kBloodDeep, 64);
    ring(draw, centre, radius - 7.0f, 1.0f, theme::rgba(0xb3, 0x24, 0x1b, 0.6f), 64);
    const Color4F steel = theme::rgba(0xc9, 0xc2, 0xb2);
    cutlass(draw, centre + Vec2(-radius * 0.5f, -radius * 0.55f), kPi / 4, radius * 1.25f, steel, theme::kGoldTarnish);
    cutlass(draw, centre + Vec2(radius * 0.5f, -radius * 0.55f), kPi * 3 / 4, radius * 1.25f, steel, theme::kGoldTarnish);
    skull(draw, centre + Vec2(0.0f, radius * 0.06f), radius * 0.42f, kBone, theme::rgba(0x1e, 0x14, 0x10));
}

void treasureChest(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    ring(draw, centre, radius - 1.0f, 2.0f, theme::kGoldDeep, 64);
    const float w = radius * 1.15f, h = radius * 0.62f;
    const Rect body(centre.x - w / 2, centre.y - h * 0.75f, w, h);
    // Gold spilling out from under the lid.
    for (int i = 0; i < 7; ++i) {
        const float x = body.getMinX() + w * (0.15f + 0.12f * static_cast<float>(i));
        draw->drawSolidCircle(Vec2(x, body.getMaxY() + 3.0f + static_cast<float>(i % 2) * 3.0f), 4.0f, 0.0f, 12,
                              theme::kGoldPip);
    }
    fill(draw, roundedRect(body, 3.0f), theme::rgba(0x5a, 0x34, 0x18));
    const Rect lid(body.getMinX() - 2.0f, body.getMaxY() + 2.0f, w + 4.0f, h * 0.55f);
    fill(draw, roundedRect(lid, Corners{{2.0f, 2.0f}, {2.0f, 2.0f}, {w * 0.3f, h * 0.4f}, {w * 0.3f, h * 0.4f}}),
         theme::rgba(0x6e, 0x40, 0x1c));
    for (const float fx : {0.2f, 0.8f}) { // iron bands
        const float x = body.getMinX() + w * fx;
        draw->drawSolidRect(Vec2(x - 3.0f, body.getMinY()), Vec2(x + 3.0f, lid.getMaxY() - 2.0f), theme::kGoldDark);
    }
    draw->drawSolidRect(Vec2(body.getMinX(), body.getMaxY() - 2.0f), Vec2(body.getMaxX(), body.getMaxY() + 2.0f),
                        theme::kGoldDark);
    fill(draw, roundedRect(Rect(centre.x - 6.0f, body.getMaxY() - 12.0f, 12.0f, 14.0f), 2.0f), theme::kGoldPip); // lock
    draw->drawSolidCircle(Vec2(centre.x, body.getMaxY() - 6.0f), 1.8f, 0.0f, 8, theme::rgba(0x1e, 0x14, 0x10));
}

void drownedSkull(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    ring(draw, centre, radius - 1.0f, 2.0f, theme::kBloodDeep, 64);
    // Seaweed trailing from the skull, then the skull itself, stained with brine.
    const Color4F weed = theme::rgba(0x2a, 0x3a, 0x26);
    for (const float fx : {-0.4f, 0.0f, 0.4f}) {
        const Vec2 root = centre + Vec2(radius * fx, -radius * 0.2f);
        Vec2 at = root;
        for (int i = 0; i < 5; ++i) {
            const Vec2 next = at + Vec2((i % 2 == 0 ? 3.0f : -3.0f), -radius * 0.11f);
            draw->drawSegment(at, next, 1.8f, weed);
            at = next;
        }
    }
    skull(draw, centre + Vec2(0.0f, radius * 0.12f), radius * 0.78f, theme::rgba(0xef, 0xe1, 0xc2),
          theme::rgba(0x4a, 0x0c, 0x06));
}

} // namespace decor
