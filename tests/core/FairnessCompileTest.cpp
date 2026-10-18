#include <type_traits>

#include <gtest/gtest.h>

#include "bs/core/Ai.h"

using namespace bs::core;

// Principle III, enforced by types: a strategy's only window onto the opponent is the read-only
// public view. There is no Board or Fleet parameter through which it could see unrevealed ships.
using ChooseSignature = AiAction (ITargetingStrategy::*)(const ITargetView&, int, bool, bool, IRandom&);
static_assert(std::is_same<decltype(&ITargetingStrategy::choose), ChooseSignature>::value,
              "strategies must decide from the public ITargetView only");

// ITargetView exposes only cell knowledge, never ship positions.
static_assert(std::is_abstract<ITargetView>::value, "ITargetView is an interface");
static_assert(!std::is_base_of<Board, ITargetView>::value, "the view is not a Board");

TEST(Fairness, StrategiesSeeOnlyThePublicView) { SUCCEED(); }
