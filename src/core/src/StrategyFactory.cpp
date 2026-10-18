#include "bs/core/Ai.h"

namespace bs {
namespace core {

std::unique_ptr<ITargetingStrategy> makeStrategy(Difficulty difficulty, IRandom& rng) {
    switch (difficulty) {
    case Difficulty::Easy:
        return std::make_unique<RandomStrategy>();
    case Difficulty::Medium:
        return std::make_unique<HuntTargetStrategy>();
    case Difficulty::Hard:
        break;
    }
    return std::make_unique<HardStrategy>(rng);
}

} // namespace core
} // namespace bs
