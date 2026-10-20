#pragma once

#include <memory>
#include <vector>

#include "bs/core/GameSession.h"
#include "bs/core/Random.h"

namespace bs {
namespace core {

struct AiAction {
    enum class Kind { Shot, Help };
    Kind kind;
    Coord target;
};

// One implementation per difficulty (Open/Closed). Strategies see only the public view of the
// opponent's board, never its ships (Principle III).
class ITargetingStrategy {
public:
    virtual ~ITargetingStrategy() = default;
    // Called once per computer action. `turnIndex` is turnsTaken(own side); `firstActionOfTurn`
    // is true on the first action after the turn passed. Must return a legal action: a Shot at
    // an Unknown cell, or a Help with an Unknown cell in its cross and only if helpUsesLeft > 0.
    virtual AiAction choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                            int helpUsesLeft, IRandom& rng) = 0;
};

// Easy: a uniformly random Unknown cell (FR-033).
class RandomStrategy final : public ITargetingStrategy {
public:
    AiAction choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                    int helpUsesLeft, IRandom& rng) override;
};

// Medium: random until a hit, then finish that ship (FR-034). With one hit it tries the four
// orthogonal neighbours; once hits line up it fires only at the ends of the line. Wounded ships
// are finished in the order their first hit was observed, from this strategy's own history.
class HuntTargetStrategy final : public ITargetingStrategy {
public:
    AiAction choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                    int helpUsesLeft, IRandom& rng) override;

    // Updates the hit history from the view; true if a hit-but-unsunk ship exists.
    bool observe(const ITargetView& view);

private:
    std::vector<Coord> hitOrder_;
};

// Hard's Help schedule (FR-035): kHelpUsesPerGame distinct target turns drawn from [1, 20] once,
// and the centre choice. A use is due once its target turn is reached and it is not yet spent;
// a use deferred by a wounded ship, or by another use on the same turn, simply stays due.
class HelpPlanner {
public:
    static constexpr int kLatestTurn = 20;

    explicit HelpPlanner(IRandom& rng);

    // Sorted ascending.
    const std::vector<int>& targetTurns() const { return targetTurns_; }
    bool shouldUse(int turnIndex, bool firstActionOfTurn, int helpUsesLeft, bool woundedShip) const;
    // A random cell whose whole on-board cross is Unknown; otherwise one whose cross has the
    // most Unknown cells.
    Coord chooseCentre(const ITargetView& view, IRandom& rng) const;

private:
    std::vector<int> targetTurns_;
};

// Hard: Medium plus three well-aimed Helps.
class HardStrategy final : public ITargetingStrategy {
public:
    explicit HardStrategy(IRandom& rng) : planner_(rng) {}

    AiAction choose(const ITargetView& view, int turnIndex, bool firstActionOfTurn,
                    int helpUsesLeft, IRandom& rng) override;
    const std::vector<int>& helpTurns() const { return planner_.targetTurns(); }

private:
    HuntTargetStrategy hunt_;
    HelpPlanner planner_;
};

// A uniformly random Unknown cell, or (0,0) if none is left.
Coord randomUnknownCell(const ITargetView& view, IRandom& rng);

std::unique_ptr<ITargetingStrategy> makeStrategy(Difficulty difficulty, IRandom& rng);

} // namespace core
} // namespace bs
