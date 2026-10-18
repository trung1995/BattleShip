#include <gtest/gtest.h>

#include "Fleets.h"
#include "bs/app/GameSetup.h"
#include "bs/app/PlacementController.h"
#include "bs/core/AutoPlacer.h"

using namespace bs::app;
using namespace bs::core;

TEST(GameSetup, NewSessionIsInPlacementWithAComputerFleetAndNoPlayerShips) {
    BacktrackingPlacer placer;
    Mt19937Random rng(5);
    GameSetup setup(placer, rng);
    auto session = setup.newSession(Difficulty::Hard);
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->phase(), Phase::Placement);
    EXPECT_EQ(session->difficulty(), Difficulty::Hard);
    EXPECT_TRUE(isLegalCompleteFleet(session->ownFleet(Side::Computer)));
    EXPECT_EQ(session->playerFleet().missingShips().size(), 10u);
}

TEST(GameSetup, SameSeedGivesTheSameComputerFleet) {
    BacktrackingPlacer placer;
    Mt19937Random a(9);
    Mt19937Random b(9);
    auto first = GameSetup(placer, a).newSession(Difficulty::Easy);
    auto second = GameSetup(placer, b).newSession(Difficulty::Easy);
    for (ShipId id = 0; id < kFleetSize; ++id) {
        EXPECT_EQ(first->ownFleet(Side::Computer).placementOf(id)->origin,
                  second->ownFleet(Side::Computer).placementOf(id)->origin);
    }
}

TEST(GameSetup, PlacementControllerEditsTheSessionsOwnPlayerFleet) {
    BacktrackingPlacer placer;
    Mt19937Random rng(3);
    GameSetup setup(placer, rng);
    auto session = setup.newSession(Difficulty::Easy);
    PlacementController placement(session->playerFleet(), placer, rng);
    placement.autoPlaceAll();
    EXPECT_EQ(session->startBattle(), StartStatus::Started);
}
