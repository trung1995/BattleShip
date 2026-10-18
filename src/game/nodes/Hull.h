#pragma once

#include "cocos2d.h"

// A wooden hull with a rounded bow and brass rivets, one per deck. Shared by the boards, the
// dock and the dragged ship so a ship looks the same everywhere.
namespace hull {

enum class State { Intact, Selected, Lifted, Wreck };

// `cells` is the rectangle of the ship's cells in the DrawNode's space; the hull sits 4 px
// inside it. Vertical ships point their bow up; horizontal ships point it right.
void draw(cocos2d::DrawNode* draw, const cocos2d::Rect& cells, int length, bool vertical, State state);

} // namespace hull
