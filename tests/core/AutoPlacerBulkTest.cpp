#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/AutoPlacer.h"

using namespace bs::core;

// SC-002 (full placement): every generated fleet is complete with the exact composition and
// obeys the no-overlap/no-touch rules.
TEST(AutoPlacerBulk, TenThousandFullPlacementsAreAllLegal) {
    BacktrackingPlacer placer;
    for (std::uint32_t seed = 0; seed < 10000; ++seed) {
        Mt19937Random rng(seed);
        const AutoPlaceResult result = placer.placeAll(rng);
        ASSERT_EQ(result.status, AutoPlaceStatus::Ok) << "seed " << seed;
        ASSERT_TRUE(isLegalCompleteFleet(result.fleet)) << "seed " << seed;
    }
}

// SC-002 (mixed placement): random partial layouts completed by placeRemaining.
TEST(AutoPlacerBulk, TenThousandMixedPlacementsAreAllLegal) {
    BacktrackingPlacer placer;
    for (std::uint32_t seed = 0; seed < 10000; ++seed) {
        Mt19937Random rng(seed);
        Fleet partial = placer.placeAll(rng).fleet;
        for (ShipId id = 0; id < kFleetSize; ++id) {
            if (rng.nextInt(0, 1) == 0) partial.remove(id);
        }
        const AutoPlaceResult result = placer.placeRemaining(partial, rng);
        ASSERT_EQ(result.status, AutoPlaceStatus::Ok) << "seed " << seed;
        ASSERT_TRUE(isLegalCompleteFleet(result.fleet)) << "seed " << seed;
        for (ShipId id = 0; id < kFleetSize; ++id) {
            if (const ShipPlacement* kept = partial.placementOf(id)) {
                ASSERT_EQ(result.fleet.placementOf(id)->origin, kept->origin) << "seed " << seed;
            }
        }
    }
}
