#pragma once

#include <array>
#include <memory>
#include <vector>

#include "bs/core/Board.h"
#include "bs/core/Fleet.h"
#include "bs/core/Types.h"

namespace bs {
namespace core {

enum class Phase { Placement, Battle, Finished };

// Read-only public view of one board: exactly what the opponent may know (Principle III).
class ITargetView {
public:
    virtual ~ITargetView() = default;
    virtual Knowledge at(Coord cell) const = 0;
};

// Projects a Board through Board::knowledgeAt; never reveals untouched ship cells.
class TargetView final : public ITargetView {
public:
    explicit TargetView(const Board& board) : board_(&board) {}
    Knowledge at(Coord cell) const override { return board_->knowledgeAt(cell); }

private:
    const Board* board_;
};

struct CellChange {
    Side board; // whose board changed
    Coord coord;
    Knowledge now;
};

enum class ActionStatus {
    Applied,
    RejectedWrongPhase,
    RejectedNotYourTurn,
    RejectedOffBoard,
    RejectedAlreadyFired,
    RejectedAlreadyUsed,
    RejectedNoEffect
};

struct ShotOutcome {
    ActionStatus status = ActionStatus::RejectedWrongPhase;
    ShotResult result = ShotResult::Miss;
    ShipId sunkShip = -1;
    std::vector<CellChange> changes;
    bool turnPassed = false;
    bool gameOver = false;
};

struct HelpOutcome {
    ActionStatus status = ActionStatus::RejectedWrongPhase;
    std::vector<Coord> affected; // on-board, previously untouched cross cells, in resolution order
    std::vector<ShipId> sunkShips;
    std::vector<CellChange> changes;
    bool gameOver = false;
};

enum class StartStatus { Started, FleetIncomplete, WrongPhase };

// One battle from placement to game over. Rejected commands leave all state unchanged.
class GameSession {
public:
    GameSession(Difficulty difficulty, Fleet computerFleet);
    GameSession(GameSession&&) = default;
    GameSession& operator=(GameSession&&) = default;
    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;

    // The player's fleet while arranging it. Throws std::logic_error outside Placement (FR-015).
    Fleet& playerFleet();
    StartStatus startBattle();

    ShotOutcome fire(Side shooter, Coord target);
    // The cross: the centre plus up, down, left, right (FR-026..FR-031), kHelpUsesPerGame times
    // per participant, in place of any shot of the user's turn, including an extra shot after a
    // hit. Skips off-board and fired cells; a centre with nothing left to hit is rejected and
    // consumes no use.
    HelpOutcome useHelp(Side user, Coord centre);

    Phase phase() const { return phase_; }
    Side currentTurn() const { return currentTurn_; }
    Side winner() const; // precondition: phase() == Finished
    int helpUsesLeft(Side side) const; // 0..kHelpUsesPerGame (FR-032)
    bool helpAvailable(Side side) const { return helpUsesLeft(side) > 0; }
    int turnsTaken(Side side) const; // times the turn has passed to `side` (FR-035)
    const ITargetView& viewOf(Side boardOwner) const;
    const Fleet& ownFleet(Side side) const;
    Difficulty difficulty() const { return difficulty_; }

private:
    static std::size_t index(Side side) { return side == Side::Player ? 0 : 1; }
    Board& boardOf(Side owner) { return *boards_[index(owner)]; }
    void passTurnTo(Side side);

    Difficulty difficulty_;
    Phase phase_ = Phase::Placement;
    Side currentTurn_ = Side::Player;
    Side winner_ = Side::Player;
    Fleet playerFleet_;
    Fleet computerFleet_;
    std::array<std::unique_ptr<Board>, 2> boards_;
    std::array<std::unique_ptr<TargetView>, 2> views_;
    std::array<int, 2> turnsTaken_{};
    std::array<int, 2> helpUsesLeft_{{kHelpUsesPerGame, kHelpUsesPerGame}};
};

} // namespace core
} // namespace bs
