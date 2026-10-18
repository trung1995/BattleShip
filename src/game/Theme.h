#pragma once

#include "cocos2d.h"

// The "Captain's Chart Table" look: ink-and-parchment sea charts on a tarred wooden table,
// brass fittings and red sealing wax. Design resolution 1280 x 720.
namespace theme {

inline cocos2d::Color4F rgba(int r, int g, int b, float a = 1.0f) {
    return cocos2d::Color4F(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
                            static_cast<float>(b) / 255.0f, a);
}
inline cocos2d::Color3B rgb(int r, int g, int b) {
    return cocos2d::Color3B(static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b));
}

// Table and frames
const cocos2d::Color4F kTableCentre = rgba(0x3a, 0x26, 0x16);
const cocos2d::Color4F kTableEdge = rgba(0x1f, 0x14, 0x0c);
const cocos2d::Color4F kSeaStripeLight = rgba(0x1b, 0x2a, 0x2b);
const cocos2d::Color4F kSeaStripeDark = rgba(0x17, 0x24, 0x25);
const cocos2d::Color4F kNight = rgba(0x12, 0x0c, 0x07);
const cocos2d::Color4F kFrameWood = rgba(0x3a, 0x24, 0x14);
const cocos2d::Color4F kFrameEdge = rgba(0x5a, 0x3a, 0x20);

// Parchment
const cocos2d::Color4F kParchment = rgba(0xe8, 0xd4, 0xa6);
const cocos2d::Color4F kParchmentBurn = rgba(120, 80, 30, 0.55f);
const cocos2d::Color4F kParchmentRule = rgba(0xb8, 0x9c, 0x6c);
const cocos2d::Color4F kGridLine = rgba(110, 80, 45, 0.45f);
const cocos2d::Color4F kGridEdge = rgba(0x9c, 0x80, 0x58);

// Brass and wax
const cocos2d::Color4F kBrass = rgba(0xe0, 0xb0, 0x4e);
const cocos2d::Color4F kBrassDeep = rgba(0xc9, 0x97, 0x3f);
const cocos2d::Color4F kBrassDark = rgba(0x8a, 0x62, 0x24);
const cocos2d::Color4F kBrassGlow = rgba(224, 176, 78, 0.35f);
const cocos2d::Color4F kWax = rgba(0xa4, 0x30, 0x1f);
const cocos2d::Color4F kWaxDeep = rgba(0x8e, 0x2a, 0x1c);
const cocos2d::Color4F kWaxEdge = rgba(0xc0, 0x50, 0x3a);
const cocos2d::Color4F kWaxShadow = rgba(0x4e, 0x14, 0x0a);

// Cell marks
const cocos2d::Color4F kMissInk = rgba(0x2f, 0x4a, 0x4d);
const cocos2d::Color4F kMissWash = rgba(47, 74, 77, 0.14f);
const cocos2d::Color4F kMarkedDot = rgba(0x9a, 0x83, 0x60);
const cocos2d::Color4F kHitSeal = rgba(0xa4, 0x30, 0x1f);
const cocos2d::Color4F kHitSealEdge = rgba(0x5e, 0x16, 0x0c);
const cocos2d::Color4F kHitCross = rgba(0xf6, 0xe2, 0xb8);
const cocos2d::Color4F kSunkCross = rgba(0xc9, 0x8d, 0x68);
const cocos2d::Color4F kBlockedInk = rgba(0x6b, 0x52, 0x36);
const cocos2d::Color4F kPreviewLegal = rgba(64, 128, 92, 0.42f);
const cocos2d::Color4F kPreviewLegalEdge = rgba(0x2f, 0x6b, 0x48);
const cocos2d::Color4F kPreviewIllegal = rgba(164, 48, 31, 0.55f);
const cocos2d::Color4F kPreviewIllegalWash = rgba(164, 48, 31, 0.18f);
const cocos2d::Color4F kHoverWash = rgba(164, 48, 31, 0.12f);
const cocos2d::Color4F kHelpFill = rgba(224, 176, 78, 0.5f);
const cocos2d::Color4F kHelpEdge = rgba(0xb8, 0x86, 0x2f);
const cocos2d::Color4F kHelpCentre = rgba(224, 176, 78, 0.85f);
const cocos2d::Color4F kHelpSkipped = rgba(224, 176, 78, 0.12f);

// Ships
const cocos2d::Color4F kHullTop = rgba(0x94, 0x64, 0x3a);
const cocos2d::Color4F kHullMid = rgba(0x6a, 0x43, 0x22);
const cocos2d::Color4F kHullBottom = rgba(0x55, 0x35, 0x1a);
const cocos2d::Color4F kHullEdge = rgba(0x3e, 0x24, 0x10);
const cocos2d::Color4F kRivet = rgba(0xe3, 0xc7, 0x8c);
const cocos2d::Color4F kRivetEdge = rgba(0x6b, 0x4a, 0x22);
const cocos2d::Color4F kWreckLight = rgba(0x34, 0x21, 0x1a);
const cocos2d::Color4F kWreckDark = rgba(0x1f, 0x13, 0x0c);
const cocos2d::Color4F kWreckEdge = rgba(0x0e, 0x07, 0x04);
const cocos2d::Color4F kWreckRivet = rgba(0x4a, 0x34, 0x26);

// Text
const cocos2d::Color3B kInk = rgb(0x2b, 0x1d, 0x12);
const cocos2d::Color3B kInkBrown = rgb(0x6b, 0x4a, 0x22);
const cocos2d::Color3B kInkFaded = rgb(0x6b, 0x52, 0x36);
const cocos2d::Color3B kInkRed = rgb(0x5e, 0x16, 0x0c);
const cocos2d::Color3B kLight = rgb(0xec, 0xdc, 0xb4);
const cocos2d::Color3B kTitle = rgb(0xf0, 0xdf, 0xb4);
const cocos2d::Color3B kCream = rgb(0xf3, 0xe2, 0xb8);
const cocos2d::Color3B kWaxText = rgb(0xfb, 0xec, 0xcb);
const cocos2d::Color3B kMuted = rgb(0xbb, 0xa8, 0x82);
const cocos2d::Color3B kHint = rgb(0xc7, 0xb3, 0x8a);
const cocos2d::Color3B kGold = rgb(0xf0, 0xc2, 0x6a);
const cocos2d::Color3B kWarning = rgb(0xf0, 0x8a, 0x6e);
const cocos2d::Color3B kAxis = rgb(0x4a, 0x32, 0x20);

constexpr float kCellSize = 36.0f;

// Alegreya (body) and Alegreya SC (headings, buttons): OFL, Latin + Cyrillic + Vietnamese.
enum class Face { Body, Italic, Caps, CapsHeavy };
inline const char* fontFile(Face face) {
    switch (face) {
    case Face::Italic: return "fonts/Alegreya-Italic.ttf";
    case Face::Caps: return "fonts/AlegreyaSC-Bold.ttf";
    case Face::CapsHeavy: return "fonts/AlegreyaSC-ExtraBold.ttf";
    default: return "fonts/Alegreya-Regular.ttf";
    }
}

} // namespace theme
