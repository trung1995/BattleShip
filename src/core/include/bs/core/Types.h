#pragma once

namespace bs {
namespace core {

// The two equal sides of the rules. Only the session knows which one is the human.
enum class Side { Player, Computer };

enum class ShotResult { Miss, Hit, Sunk };

enum class Difficulty { Easy, Medium, Hard };

inline Side opponentOf(Side side) { return side == Side::Player ? Side::Computer : Side::Player; }

} // namespace core
} // namespace bs
