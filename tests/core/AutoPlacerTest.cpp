#include <algorithm>
#include <chrono>
#include <vector>

#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/core/AutoPlacer.h"

using namespace bs::core;

namespace {
bool sameLayout(const Fleet& a, const Fleet& b) {
    for (ShipId id = 0; id < kFleetSize; ++id) {
        const ShipPlacement* pa = a.placementOf(id);
        const ShipPlacement* pb = b.placementOf(id);
        if ((pa == nullptr) != (pb == nullptr)) return false;
        if (pa != nullptr && (pa->origin != pb->origin || pa->orientation != pb->orientation))
            return false;
    }
    return true;
}
} // namespace

TEST(AutoPlacer, PlaceAllReturnsACompleteLegalFleet) {
    BacktrackingPlacer placer;
    Mt19937Random rng(1);
    const AutoPlaceResult result = placer.placeAll(rng);
    EXPECT_EQ(result.status, AutoPlaceStatus::Ok);
    EXPECT_TRUE(isLegalCompleteFleet(result.fleet));
}

TEST(AutoPlacer, SameSeedGivesTheSameFleet) {
    BacktrackingPlacer placer;
    Mt19937Random a(77);
    Mt19937Random b(77);
    EXPECT_TRUE(sameLayout(placer.placeAll(a).fleet, placer.placeAll(b).fleet));
}

TEST(AutoPlacer, DifferentSeedsUsuallyGiveDifferentFleets) {
    BacktrackingPlacer placer;
    Mt19937Random base(0);
    const Fleet reference = placer.placeAll(base).fleet;
    int different = 0;
    for (std::uint32_t seed = 1; seed <= 100; ++seed) {
        Mt19937Random rng(seed);
        different += sameLayout(placer.placeAll(rng).fleet, reference) ? 0 : 1;
    }
    EXPECT_GE(different, 95);
}

TEST(AutoPlacer, PlaceAllIsFast) {
    BacktrackingPlacer placer;
    std::vector<double> millis;
    for (std::uint32_t seed = 0; seed < 100; ++seed) {
        Mt19937Random rng(seed);
        const auto start = std::chrono::steady_clock::now();
        placer.placeAll(rng);
        const std::chrono::duration<double, std::milli> elapsed =
            std::chrono::steady_clock::now() - start;
        ASSERT_LT(elapsed.count(), 1000.0) << "seed " << seed; // SC-003
        millis.push_back(elapsed.count());
    }
    std::sort(millis.begin(), millis.end());
    EXPECT_LT(millis[millis.size() / 2], 5.0);
}

TEST(AutoPlacer, PlaceRemainingKeepsEveryHandPlacedShipExactly) {
    BacktrackingPlacer placer;
    for (std::uint32_t seed = 0; seed < 50; ++seed) {
        Mt19937Random rng(seed);
        Fleet partial = placer.placeAll(rng).fleet;
        // Keep the first (seed % 10) ships, remove the rest.
        const int keep = static_cast<int>(seed % 10);
        for (ShipId id = keep; id < kFleetSize; ++id) partial.remove(id);

        const AutoPlaceResult result = placer.placeRemaining(partial, rng);
        ASSERT_EQ(result.status, AutoPlaceStatus::Ok) << "seed " << seed;
        ASSERT_TRUE(isLegalCompleteFleet(result.fleet));
        for (ShipId id = 0; id < keep; ++id) {
            EXPECT_EQ(result.fleet.placementOf(id)->origin, partial.placementOf(id)->origin);
            EXPECT_EQ(result.fleet.placementOf(id)->orientation, partial.placementOf(id)->orientation);
        }
    }
}

TEST(AutoPlacer, LayoutWithoutRoomReallyHasNoRoom) {
    const Fleet partial = fleetWithNoRoomForTheFourShip();
    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            for (const Orientation o : {Orientation::Horizontal, Orientation::Vertical}) {
                ASSERT_NE(partial.canPlace(0, {{row, col}, 4, o}), PlacementError::None)
                    << row << "," << col;
            }
        }
    }
}

TEST(AutoPlacer, PlaceRemainingReportsCannotFitAndLeavesTheLayoutUnchanged) {
    BacktrackingPlacer placer;
    Mt19937Random rng(4);
    const Fleet partial = fleetWithNoRoomForTheFourShip();
    const AutoPlaceResult result = placer.placeRemaining(partial, rng);
    EXPECT_EQ(result.status, AutoPlaceStatus::CannotFit);
    EXPECT_TRUE(sameLayout(result.fleet, partial));
}

TEST(AutoPlacer, PlaceRemainingOnACompleteFleetChangesNothing) {
    BacktrackingPlacer placer;
    Mt19937Random rng(2);
    const Fleet complete = knownFleet();
    const AutoPlaceResult result = placer.placeRemaining(complete, rng);
    EXPECT_EQ(result.status, AutoPlaceStatus::Ok);
    EXPECT_TRUE(sameLayout(result.fleet, complete));
}
