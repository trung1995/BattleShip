#pragma once

#include "bs/core/Fleet.h"
#include "bs/core/Random.h"

namespace bs {
namespace core {

enum class AutoPlaceStatus { Ok, CannotFit };

struct AutoPlaceResult {
    AutoPlaceStatus status = AutoPlaceStatus::CannotFit;
    Fleet fleet;
};

// Automatic placement (FR-009..FR-012). New placers can be added without changing callers.
class IFleetPlacer {
public:
    virtual ~IFleetPlacer() = default;
    // Always returns Ok with a complete legal fleet.
    virtual AutoPlaceResult placeAll(IRandom& rng) const = 0;
    // Ok with a complete legal fleet that keeps every ship of `partial` in place, or CannotFit
    // with `fleet` equal to `partial`.
    virtual AutoPlaceResult placeRemaining(const Fleet& partial, IRandom& rng) const = 0;
};

// Randomized complete backtracking search, largest ship first (research R7). Because the search
// is complete, CannotFit means the remaining ships really cannot fit.
class BacktrackingPlacer final : public IFleetPlacer {
public:
    static constexpr long kNodeBudget = 2000000;

    AutoPlaceResult placeAll(IRandom& rng) const override;
    AutoPlaceResult placeRemaining(const Fleet& partial, IRandom& rng) const override;
};

} // namespace core
} // namespace bs
