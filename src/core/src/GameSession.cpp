#include "bs/core/GameSession.h"

#include <memory>
#include <stdexcept>

namespace bs {
namespace core {

GameSession::GameSession(Difficulty difficulty, Fleet computerFleet)
    : difficulty_(difficulty), computerFleet_(std::move(computerFleet)) {
    boards_[index(Side::Player)] = std::make_unique<Board>(playerFleet_);
    boards_[index(Side::Computer)] = std::make_unique<Board>(computerFleet_);
    for (const Side side : {Side::Player, Side::Computer}) {
        views_[index(side)] = std::make_unique<TargetView>(*boards_[index(side)]);
    }
}

Fleet& GameSession::playerFleet() {
    if (phase_ != Phase::Placement) {
        throw std::logic_error("the player's fleet is locked once the battle starts");
    }
    return playerFleet_;
}

StartStatus GameSession::startBattle() {
    if (phase_ != Phase::Placement) {
        return StartStatus::WrongPhase;
    }
    if (!playerFleet_.isComplete()) {
        return StartStatus::FleetIncomplete;
    }
    boards_[index(Side::Player)] = std::make_unique<Board>(playerFleet_);
    views_[index(Side::Player)] = std::make_unique<TargetView>(*boards_[index(Side::Player)]);
    phase_ = Phase::Battle;
    currentTurn_ = Side::Player;
    turnsTaken_[index(Side::Player)] = 1;
    return StartStatus::Started;
}

void GameSession::passTurnTo(Side side) {
    currentTurn_ = side;
    ++turnsTaken_[index(side)];
}

ShotOutcome GameSession::fire(Side shooter, Coord target) {
    ShotOutcome outcome;
    if (phase_ != Phase::Battle) {
        outcome.status = ActionStatus::RejectedWrongPhase;
        return outcome;
    }
    if (shooter != currentTurn_) {
        outcome.status = ActionStatus::RejectedNotYourTurn;
        return outcome;
    }
    if (!target.isOnBoard()) {
        outcome.status = ActionStatus::RejectedOffBoard;
        return outcome;
    }
    const Side opponent = opponentOf(shooter);
    Board& board = boardOf(opponent);
    if (board.isFiredUpon(target)) {
        outcome.status = ActionStatus::RejectedAlreadyFired;
        return outcome;
    }

    const ShotEffect effect = board.shoot(target);
    outcome.status = ActionStatus::Applied;
    outcome.result = effect.result;
    outcome.sunkShip = effect.sunkShip;
    for (const CellUpdate& update : effect.updates) {
        outcome.changes.push_back({opponent, update.coord, update.now});
    }

    if (board.remainingCells() == 0) {
        phase_ = Phase::Finished;
        winner_ = shooter;
        outcome.gameOver = true; // ends immediately, whatever extra shot was pending (FR-023)
    } else if (effect.result == ShotResult::Miss) {
        passTurnTo(opponent);
        outcome.turnPassed = true;
    }
    return outcome;
}

HelpOutcome GameSession::useHelp(Side user, Coord centre) {
    HelpOutcome outcome;
    if (phase_ != Phase::Battle) {
        outcome.status = ActionStatus::RejectedWrongPhase;
        return outcome;
    }
    if (user != currentTurn_) {
        outcome.status = ActionStatus::RejectedNotYourTurn;
        return outcome;
    }
    if (!centre.isOnBoard()) {
        outcome.status = ActionStatus::RejectedOffBoard;
        return outcome;
    }
    if (helpUsed_[index(user)]) {
        outcome.status = ActionStatus::RejectedAlreadyUsed;
        return outcome;
    }

    const Side opponent = opponentOf(user);
    Board& board = boardOf(opponent);
    const Coord cross[] = {centre,
                           {centre.row - 1, centre.col},
                           {centre.row + 1, centre.col},
                           {centre.row, centre.col - 1},
                           {centre.row, centre.col + 1}};
    for (const Coord c : cross) {
        if (c.isOnBoard() && !board.isFiredUpon(c)) {
            outcome.affected.push_back(c);
        }
    }
    if (outcome.affected.empty()) {
        outcome.status = ActionStatus::RejectedNoEffect; // not consumed (FR-029)
        return outcome;
    }

    // Resolve every cross cell first, then mark around sunk ships, so cross cells keep their real
    // result (Edge Cases).
    for (const Coord c : outcome.affected) {
        const ShotEffect effect = board.shoot(c, true);
        for (const CellUpdate& update : effect.updates) {
            outcome.changes.push_back({opponent, update.coord, update.now});
        }
        if (effect.sunkShip >= 0) {
            outcome.sunkShips.push_back(effect.sunkShip);
        }
    }
    for (const ShipId id : outcome.sunkShips) {
        for (const CellUpdate& update : board.markAround(id)) {
            outcome.changes.push_back({opponent, update.coord, update.now});
        }
    }

    helpUsed_[index(user)] = true;
    outcome.status = ActionStatus::Applied;
    if (board.remainingCells() == 0) {
        phase_ = Phase::Finished;
        winner_ = user;
        outcome.gameOver = true;
    } else {
        passTurnTo(opponent); // even if it hit (FR-030)
    }
    return outcome;
}

Side GameSession::winner() const {
    if (phase_ != Phase::Finished) {
        throw std::logic_error("winner() before the game is finished");
    }
    return winner_;
}

bool GameSession::helpAvailable(Side side) const { return !helpUsed_[index(side)]; }

int GameSession::turnsTaken(Side side) const { return turnsTaken_[index(side)]; }

const ITargetView& GameSession::viewOf(Side boardOwner) const { return *views_[index(boardOwner)]; }

const Fleet& GameSession::ownFleet(Side side) const {
    return side == Side::Player ? boards_[index(Side::Player)]->fleet() : computerFleet_;
}

} // namespace core
} // namespace bs
