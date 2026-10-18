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
    constexpr int kSteps = 12;
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

void table(cocos2d::DrawNode* draw, const cocos2d::Size& size) {
    draw->drawSolidRect(Vec2::ZERO, Vec2(size), theme::kTableEdge);
    const Vec2 centre(size.width * 0.5f, size.height * 0.6f);
    const float rx = size.width * 0.5f * kRoot2;
    const float ry = size.height * 0.6f * kRoot2;
    constexpr int kSteps = 36;
    for (int i = kSteps; i >= 1; --i) {
        const float f = 0.7f * static_cast<float>(i) / static_cast<float>(kSteps);
        const Color4F c = mix(theme::kTableCentre, theme::kTableEdge, f / 0.7f);
        draw->drawSolidCircle(centre, rx * f, 0.0f, 72, 1.0f, ry / rx, c);
    }
}

void nightSea(cocos2d::DrawNode* draw, const cocos2d::Size& size) {
    draw->drawSolidRect(Vec2::ZERO, Vec2(size), theme::kSeaStripeDark);
    hatch(draw, rectPoly(Rect(Vec2::ZERO, size)), theme::kSeaStripeLight, 32.0f, 16.0f, true);
}

namespace {
// One point of a star: a kite from the centre to `length`, split into a light and a dark half.
void starPoint(cocos2d::DrawNode* draw, const Vec2& centre, float angle, float length, float halfWidth,
               const Color4F& light, const Color4F& dark) {
    const Vec2 dir(std::cos(angle), std::sin(angle));
    const Vec2 side(-dir.y, dir.x);
    const Vec2 tip = centre + dir * length;
    const Vec2 lightHalf[] = {centre, centre + side * halfWidth + dir * halfWidth, tip};
    const Vec2 darkHalf[] = {centre, tip, centre - side * halfWidth + dir * halfWidth};
    draw->drawSolidPoly(lightHalf, 3, light);
    draw->drawSolidPoly(darkHalf, 3, dark);
}
} // namespace

void compassRose(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    draw->drawSolidCircle(centre, radius, 0.0f, 64, theme::rgba(18, 12, 7, 0.6f));
    ring(draw, centre, radius, 2.0f, theme::rgba(0xb8, 0x86, 0x2f), 64);
    ring(draw, centre, radius - 9.0f, 1.0f, theme::rgba(0xb8, 0x86, 0x2f, 0.7f), 64);
    for (int i = 0; i < 32; ++i) { // the degree ticks between the rings
        const float a = 2.0f * kPi * static_cast<float>(i) / 32.0f;
        const Vec2 dir(std::cos(a), std::sin(a));
        draw->drawSegment(centre + dir * (radius - 8.0f), centre + dir * (radius - (i % 4 == 0 ? 2.0f : 5.0f)),
                          0.6f, theme::rgba(0xc9, 0xa3, 0x5a, 0.8f));
    }
    for (int i = 0; i < 4; ++i) {
        starPoint(draw, centre, kPi / 4 + kPi / 2 * static_cast<float>(i), (radius - 14.0f) * 0.62f, 7.0f,
                  theme::rgba(0xc9, 0x97, 0x3f), theme::kBrassDark);
    }
    for (int i = 0; i < 4; ++i) {
        starPoint(draw, centre, kPi / 2 * static_cast<float>(i), radius - 14.0f, 9.0f, theme::kBrass,
                  theme::rgba(0xa9, 0x7a, 0x2e));
    }
    draw->drawSolidCircle(centre, 4.0f, 0.0f, 16, theme::kWax);
}

void victoryEmblem(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    ring(draw, centre, radius, 2.0f, theme::rgba(0xa9, 0x7a, 0x2e), 64);
    for (int i = 0; i < 5; ++i) {
        starPoint(draw, centre, kPi / 2 + 2.0f * kPi / 5.0f * static_cast<float>(i), radius - 8.0f, 11.0f,
                  theme::kBrass, theme::rgba(0xa9, 0x7a, 0x2e));
    }
    draw->drawSolidCircle(centre, 5.0f, 0.0f, 16, theme::kWax);
}

void defeatEmblem(cocos2d::DrawNode* draw, const Vec2& centre, float radius) {
    const Color4F ink = theme::kWaxDeep;
    ring(draw, centre, radius, 2.0f, ink, 64);
    // A mast snapped halfway, its top hanging off to the side with a torn pennant.
    draw->drawSegment(centre + Vec2(0.0f, -radius * 0.62f), centre + Vec2(0.0f, radius * 0.05f), 2.5f, ink);
    draw->drawSegment(centre + Vec2(-radius * 0.4f, -radius * 0.62f), centre + Vec2(radius * 0.4f, -radius * 0.62f),
                      2.0f, ink);
    draw->drawSegment(centre + Vec2(2.0f, radius * 0.12f), centre + Vec2(radius * 0.42f, radius * 0.55f), 2.5f, ink);
    draw->drawSegment(centre + Vec2(-radius * 0.28f, -radius * 0.18f), centre + Vec2(radius * 0.22f, -radius * 0.18f),
                      1.5f, ink);
    const Vec2 pennant[] = {centre + Vec2(radius * 0.42f, radius * 0.55f), centre + Vec2(radius * 0.1f, radius * 0.6f),
                            centre + Vec2(radius * 0.3f, radius * 0.4f)};
    draw->drawSolidPoly(pennant, 3, ink);
}

PanelStyle woodPanel() {
    return {theme::kFrameWood, theme::kFrameEdge, theme::kParchment, theme::kParchmentBurn, false};
}

Rect panel(cocos2d::DrawNode* draw, const Rect& outer, float frame, const PanelStyle& style, bool dropShadow,
           float burnDepth) {
    if (dropShadow) {
        softShadow(draw, outer, 6.0f, Vec2(0.0f, -26.0f), 70.0f, theme::rgba(0, 0, 0, 0.6f));
    }
    const Poly shape = roundedRect(outer, 6.0f);
    if (style.brassFrame) {
        softShadow(draw, outer, 6.0f, Vec2::ZERO, 60.0f, theme::rgba(224, 176, 78, 0.3f));
        gradient(draw, shape, style.frame, style.frameEdge);
        stroke(draw, shape, 2.0f, theme::rgba(0xf0, 0xc2, 0x6a));
    } else {
        fill(draw, shape, style.frame);
        const Rect edge(outer.getMinX() + 1.0f, outer.getMinY() + 1.0f, outer.size.width - 2.0f,
                        outer.size.height - 2.0f);
        stroke(draw, roundedRect(edge, 5.0f), 2.0f, style.frameEdge);
    }
    const Rect paper(outer.getMinX() + frame, outer.getMinY() + frame, outer.size.width - frame * 2.0f,
                     outer.size.height - frame * 2.0f);
    draw->drawSolidRect(paper.origin, Vec2(paper.getMaxX(), paper.getMaxY()), style.paper);
    innerShadow(draw, paper, burnDepth, style.burn);
    return paper;
}

} // namespace decor
