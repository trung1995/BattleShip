#include "bs/app/BattleController.h"

namespace bs {
namespace app {

using bs::core::ActionStatus;
using bs::core::Side;

BattleController::BattleController(bs::core::GameSession& session,
                                   std::unique_ptr<bs::core::ITargetingStrategy> strategy,
                                   bs::core::IRandom& rng, IGameAudio& audio,
                                   double computerDelaySeconds)
    : session_(session), strategy_(std::move(strategy)), rng_(rng), audio_(audio),
      computerDelay_(computerDelaySeconds) {}

ActionFeedback BattleController::rejection(ActionStatus status) {
    ActionFeedback feedback;
    feedback.status = status;
    switch (status) {
    case ActionStatus::RejectedNotYourTurn:
        feedback.message = TextKey::BattleRejectNotYourTurn;
        feedback.hasMessage = true;
        break;
    case ActionStatus::RejectedAlreadyFired:
        feedback.message = TextKey::BattleRejectAlreadyFired;
        feedback.hasMessage = true;
        break;
    case ActionStatus::RejectedNoEffect:
        feedback.message = TextKey::BattleRejectHelpNoEffect;
        feedback.hasMessage = true;
        break;
    default:
        break;
    }
    return feedback;
}

ActionFeedback BattleController::onPlayerFire(bs::core::Coord target) {
    if (mode_ == Mode::GameOver) {
        return rejection(ActionStatus::RejectedWrongPhase);
    }
    if (mode_ != Mode::AwaitingPlayer) {
        return rejection(ActionStatus::RejectedNotYourTurn);
    }
    const bs::core::ShotOutcome outcome = session_.fire(Side::Player, target);
    if (outcome.status != ActionStatus::Applied) {
        return rejection(outcome.status);
    }
    afterShot(outcome, Side::Player);
    ActionFeedback feedback;
    feedback.status = ActionStatus::Applied;
    feedback.changes = outcome.changes;
    return feedback;
}

void BattleController::afterShot(const bs::core::ShotOutcome& outcome, Side shooter) {
    audio_.onShot(outcome.result);
    finishOrPass(outcome.gameOver, outcome.turnPassed, shooter);
}

void BattleController::afterHelp(const bs::core::HelpOutcome& outcome, Side user) {
    audio_.onHelp();
    finishOrPass(outcome.gameOver, !outcome.gameOver, user);
}

void BattleController::finishOrPass(bool gameOver, bool turnPassed, Side shooter) {
    if (gameOver) {
        mode_ = Mode::GameOver;
        audio_.onGameOver(session_.winner() == Side::Player);
    } else if (turnPassed) {
        mode_ = shooter == Side::Player ? Mode::ComputerActing : Mode::AwaitingPlayer;
        elapsed_ = 0.0;
        computerFirstAction_ = true;
    }
}

void BattleController::armHelp() {
    if (mode_ == Mode::AwaitingPlayer && playerHelpAvailable()) {
        mode_ = Mode::PlayerHelpTargeting;
    }
}

void BattleController::disarmHelp() {
    if (mode_ == Mode::PlayerHelpTargeting) {
        mode_ = Mode::AwaitingPlayer;
    }
}

ActionFeedback BattleController::onPlayerHelp(bs::core::Coord centre) {
    if (mode_ != Mode::PlayerHelpTargeting) {
        return rejection(ActionStatus::RejectedWrongPhase);
    }
    const bs::core::HelpOutcome outcome = session_.useHelp(Side::Player, centre);
    if (outcome.status != ActionStatus::Applied) {
        return rejection(outcome.status); // stays in targeting mode
    }
    afterHelp(outcome, Side::Player);
    ActionFeedback feedback;
    feedback.status = ActionStatus::Applied;
    feedback.changes = outcome.changes;
    return feedback;
}

bool BattleController::playerHelpAvailable() const { return session_.helpAvailable(Side::Player); }

void BattleController::requestExit() {
    if (mode_ == Mode::GameOver || mode_ == Mode::ConfirmExit) {
        return; // game over leaves without confirmation (US6 scenario 4)
    }
    modeBeforeExit_ = mode_;
    mode_ = Mode::ConfirmExit;
}

void BattleController::cancelExit() {
    if (mode_ == Mode::ConfirmExit) {
        mode_ = modeBeforeExit_; // elapsed_ was left untouched, so the delay resumes as it was
    }
}

bool BattleController::confirmExit() { return mode_ == Mode::ConfirmExit; }

std::vector<bs::core::CellChange> BattleController::tick(double dtSeconds) {
    if (mode_ != Mode::ComputerActing) {
        return {};
    }
    elapsed_ += dtSeconds;
    if (elapsed_ < computerDelay_) {
        return {};
    }
    elapsed_ = 0.0; // one action per delay, however much time has passed

    const bs::core::AiAction action = strategy_->choose(
        session_.viewOf(Side::Player), session_.turnsTaken(Side::Computer), computerFirstAction_,
        session_.helpAvailable(Side::Computer), rng_);
    computerFirstAction_ = false;

    if (action.kind == bs::core::AiAction::Kind::Help) {
        const bs::core::HelpOutcome outcome = session_.useHelp(Side::Computer, action.target);
        if (outcome.status != ActionStatus::Applied) {
            return {};
        }
        afterHelp(outcome, Side::Computer);
        return outcome.changes;
    }
    const bs::core::ShotOutcome outcome = session_.fire(Side::Computer, action.target);
    if (outcome.status != ActionStatus::Applied) {
        return {};
    }
    afterShot(outcome, Side::Computer);
    return outcome.changes;
}

TurnIndicator BattleController::turnIndicator() const {
    TurnIndicator indicator;
    indicator.gameOver = mode_ == Mode::GameOver;
    indicator.side = session_.currentTurn();
    return indicator;
}

} // namespace app
} // namespace bs
