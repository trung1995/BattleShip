#pragma once

#include "cocos2d.h"

// "Blood & Black Water" (redesign v2): a night storm on the Caribbean. Tar-black hulls,
// scorched charts on torn parchment, blood-red wax and tarnished gold. Panels and banners sit
// slightly off-kilter. Design resolution 1280 x 720.
namespace theme {

inline cocos2d::Color4F rgba(int r, int g, int b, float a = 1.0f) {
    return cocos2d::Color4F(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
                            static_cast<float>(b) / 255.0f, a);
}
inline cocos2d::Color3B rgb(int r, int g, int b) {
    return cocos2d::Color3B(static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b));
}

// Storm backdrops
const cocos2d::Color4F kStormLight = rgba(0x1c, 0x2e, 0x2d); // the lantern-lit sky, top left
const cocos2d::Color4F kStormDeep = rgba(0x0a, 0x0d, 0x0e);
const cocos2d::Color4F kStripeLight = rgba(0x10, 0x1a, 0x1b); // the menu's rain-streaked sea
const cocos2d::Color4F kStripeDark = rgba(0x0c, 0x14, 0x15);
const cocos2d::Color4F kVoid = rgba(0x07, 0x09, 0x0a);
const cocos2d::Color4F kBloodSky = rgba(0x3a, 0x0c, 0x08); // computer's Help, defeat

// Board and dock frames: scorched timber with a blood-red inlay
const cocos2d::Color4F kFrameTop = rgba(0x2a, 0x16, 0x10);
const cocos2d::Color4F kFrameBottom = rgba(0x12, 0x08, 0x06);
const cocos2d::Color4F kFrameLine = rgba(0x4a, 0x22, 0x16);
const cocos2d::Color4F kFrameInlay = rgba(0x6b, 0x1d, 0x14);

// Parchment
const cocos2d::Color4F kParchment = rgba(0xd9, 0xc4, 0x9a);
const cocos2d::Color4F kParchmentBurn = rgba(60, 22, 6, 0.85f);
const cocos2d::Color4F kParchmentRule = rgba(0xa8, 0x8a, 0x5c);
const cocos2d::Color4F kGridLine = rgba(90, 55, 30, 0.5f);
const cocos2d::Color4F kGridEdge = rgba(0x7a, 0x5a, 0x38);

// Blood-red wax
const cocos2d::Color4F kBlood = rgba(0xb3, 0x24, 0x1b);
const cocos2d::Color4F kBloodBright = rgba(0xd2, 0x3a, 0x26);
const cocos2d::Color4F kBloodDeep = rgba(0x8a, 0x1a, 0x10);
const cocos2d::Color4F kBloodDark = rgba(0x5e, 0x16, 0x0c);
const cocos2d::Color4F kBloodShadow = rgba(0x3d, 0x0a, 0x05);
const cocos2d::Color4F kEmber = rgba(0xe0, 0x40, 0x2e);
const cocos2d::Color4F kEmberEdge = rgba(0xf0, 0x6a, 0x50);

// Tarnished gold
const cocos2d::Color4F kGoldLight = rgba(0xd9, 0xad, 0x5a);
const cocos2d::Color4F kGoldDeep = rgba(0x9a, 0x6e, 0x26);
const cocos2d::Color4F kGoldEdge = rgba(0xf0, 0xcf, 0x86);
const cocos2d::Color4F kGoldPale = rgba(0xf3, 0xd3, 0x8c);
const cocos2d::Color4F kGoldPip = rgba(0xe0, 0xb0, 0x4e);
const cocos2d::Color4F kGoldTarnish = rgba(0xc7, 0x9a, 0x3e);
const cocos2d::Color4F kGoldDark = rgba(0x6e, 0x4c, 0x18);
const cocos2d::Color4F kGoldLedge = rgba(0x4a, 0x30, 0x10);

// Tar
const cocos2d::Color4F kTar = rgba(0x1e, 0x14, 0x10);
const cocos2d::Color4F kTarHover = rgba(0x2a, 0x1c, 0x14);
const cocos2d::Color4F kTarEdge = rgba(0x5a, 0x3a, 0x26);
const cocos2d::Color4F kTarLine = rgba(0x3a, 0x26, 0x18);
const cocos2d::Color4F kTarLedge = rgba(0x05, 0x03, 0x02);
const cocos2d::Color4F kTarSpent = rgba(0x16, 0x0e, 0x0a);
const cocos2d::Color4F kTarSpentEdge = rgba(0x4a, 0x30, 0x20);
const cocos2d::Color4F kPipEmpty = rgba(0x6b, 0x52, 0x36);

// Ships: tar-black hulls with gold rivets
const cocos2d::Color4F kHullTop = rgba(0x4a, 0x30, 0x22);
const cocos2d::Color4F kHullMid = rgba(0x2a, 0x1a, 0x12);
const cocos2d::Color4F kHullBottom = rgba(0x1a, 0x10, 0x0a);
const cocos2d::Color4F kHullEdge = rgba(0x0e, 0x07, 0x04);
const cocos2d::Color4F kRivet = rgba(0xc7, 0x9a, 0x3e);
const cocos2d::Color4F kRivetEdge = rgba(0x4a, 0x30, 0x10);
const cocos2d::Color4F kWreckLight = rgba(0x2a, 0x14, 0x10);
const cocos2d::Color4F kWreckDark = rgba(0x0e, 0x06, 0x05);
const cocos2d::Color4F kWreckRivet = rgba(0x3a, 0x24, 0x18);

// Cell marks
const cocos2d::Color4F kMissInk = rgba(0x1f, 0x3a, 0x3c);
const cocos2d::Color4F kMissWash = rgba(31, 58, 60, 0.18f);
const cocos2d::Color4F kMarkedDot = rgba(0x8a, 0x73, 0x56);
const cocos2d::Color4F kHitEdge = rgba(0x4a, 0x0c, 0x06);
const cocos2d::Color4F kHitCross = rgba(0xf8, 0xe8, 0xc6);
const cocos2d::Color4F kSunkCross = rgba(0xc4, 0x57, 0x4a);
const cocos2d::Color4F kBlockedInk = rgba(0x5c, 0x46, 0x30);
const cocos2d::Color4F kPreviewLegal = rgba(40, 110, 90, 0.42f);
const cocos2d::Color4F kPreviewLegalEdge = rgba(0x1f, 0x5a, 0x48);
const cocos2d::Color4F kPreviewIllegal = rgba(179, 36, 27, 0.6f);
const cocos2d::Color4F kPreviewIllegalWash = rgba(179, 36, 27, 0.18f);
const cocos2d::Color4F kHoverWash = rgba(179, 36, 27, 0.14f);
const cocos2d::Color4F kHelpFill = rgba(224, 176, 78, 0.5f);
const cocos2d::Color4F kHelpEdge = rgba(0x9a, 0x6e, 0x26);
const cocos2d::Color4F kHelpCentre = rgba(224, 176, 78, 0.88f);
const cocos2d::Color4F kHelpSkipped = rgba(224, 176, 78, 0.12f);
const cocos2d::Color4F kBlastCore = rgba(255, 170, 60, 0.85f);
const cocos2d::Color4F kBlastRim = rgba(224, 64, 46, 0.45f);

// Text
const cocos2d::Color3B kInk = rgb(0x1d, 0x14, 0x10);       // on parchment
const cocos2d::Color3B kInkBrown = rgb(0x5c, 0x46, 0x30);
const cocos2d::Color3B kInkFaded = rgb(0x6b, 0x52, 0x36);
const cocos2d::Color3B kInkRed = rgb(0x8a, 0x1a, 0x10);
const cocos2d::Color3B kInkBlood = rgb(0x4a, 0x0c, 0x06);
const cocos2d::Color3B kInkGold = rgb(0x6e, 0x4c, 0x18);
const cocos2d::Color3B kAxis = rgb(0x3a, 0x24, 0x16);
const cocos2d::Color3B kBone = rgb(0xe9, 0xdc, 0xc0);      // on dark
const cocos2d::Color3B kBoneBright = rgb(0xf6, 0xe8, 0xc4);
const cocos2d::Color3B kCream = rgb(0xf8, 0xe8, 0xc6);     // on blood red
const cocos2d::Color3B kTitle = rgb(0xef, 0xe1, 0xc2);
const cocos2d::Color3B kMuted = rgb(0xa8, 0x98, 0x7a);
const cocos2d::Color3B kHint = rgb(0xb4, 0xa3, 0x82);
const cocos2d::Color3B kSpent = rgb(0x7a, 0x6a, 0x50);
const cocos2d::Color3B kWarning = rgb(0xf0, 0x7a, 0x62);
const cocos2d::Color3B kEmberText = rgb(0xc4, 0x57, 0x4a);
const cocos2d::Color3B kGoldText = rgb(0xf0, 0xcf, 0x86);

constexpr float kCellSize = 36.0f;

// Playfair Display SC (headings, buttons, banners) and Alegreya (body): both OFL, with Latin,
// Cyrillic (Russian and Ukrainian) and Vietnamese.
enum class Face { Body, Italic, Caps, CapsHeavy };
inline const char* fontFile(Face face) {
    switch (face) {
    case Face::Italic: return "fonts/Alegreya-Italic.ttf";
    case Face::Caps: return "fonts/PlayfairDisplaySC-Bold.ttf";
    case Face::CapsHeavy: return "fonts/PlayfairDisplaySC-Black.ttf";
    default: return "fonts/Alegreya-Regular.ttf";
    }
}

} // namespace theme
