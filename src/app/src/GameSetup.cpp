#include "bs/app/GameSetup.h"

namespace bs {
namespace app {

GameSetup::GameSetup(const bs::core::IFleetPlacer& placer, bs::core::IRandom& rng)
    : placer_(placer), rng_(rng) {}

std::unique_ptr<bs::core::GameSession> GameSetup::newSession(bs::core::Difficulty difficulty) {
    return std::make_unique<bs::core::GameSession>(difficulty, placer_.placeAll(rng_).fleet);
}

} // namespace app
} // namespace bs
