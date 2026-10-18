#pragma once

#include <memory>

#include "bs/core/AutoPlacer.h"
#include "bs/core/GameSession.h"

namespace bs {
namespace app {

// Creates a new game when the placement screen opens: the computer's fleet is placed here and
// the player arranges theirs directly in session.playerFleet().
class GameSetup {
public:
    GameSetup(const bs::core::IFleetPlacer& placer, bs::core::IRandom& rng);

    std::unique_ptr<bs::core::GameSession> newSession(bs::core::Difficulty difficulty);

private:
    const bs::core::IFleetPlacer& placer_;
    bs::core::IRandom& rng_;
};

} // namespace app
} // namespace bs
