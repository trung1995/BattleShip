#pragma once

#include <memory>
#include <vector>

#include "bs/app/Services.h"
#include "bs/app/TextKeys.h"
#include "bs/core/Ai.h"
#include "bs/core/GameSession.h"

namespace bs {
namespace app {

enum class Mode { AwaitingPlayer, PlayerHelpTargeting, ComputerActing, ConfirmExit, GameOver };

struct ActionFeedback {
    bs::core::ActionStatus status = bs::core::ActionStatus::RejectedWrongPhase;
    TextKey message = TextKey::BattleRejectNotYourTurn; // valid when hasMessage
    bool hasMessage = false;
    std::vector<bs::core::CellChange> changes; // render these in the same frame (SC-009)
};

struct TurnIndicator {
    bs::core::Side side = bs::core::Side::Player;
    bool gameOver = false;
};

// The battle screen's state machine. Time comes in through tick(), so pacing is testable
// without a clock (research R14).
class BattleController {
public:
    static constexpr double kDefaultComputerDelay = 0.8;

    BattleController(bs::core::GameSession& session,
                     std::unique_ptr<bs::core::ITargetingStrategy> strategy, bs::core::IRandom& rng,
                     IGameAudio& audio, double computerDelaySeconds = kDefaultComputerDelay);

    Mode mode() const { return mode_; }
    ActionFeedback onPlayerFire(bs::core::Coord target);
    // Help targeting (FR-026..FR-032): arm, then pick the centre, or disarm to cancel.
    void armHelp();
    void disarmHelp();
    ActionFeedback onPlayerHelp(bs::core::Coord centre);
    bool playerHelpAvailable() const;
    // Leaving an in-progress battle needs confirmation (FR-044). While the prompt is open the
    // battle is frozen; cancelling resumes the exact state, including the computer's delay.
    void requestExit();
    void cancelExit();
    bool confirmExit(); // true if the battle should be abandoned
    // Performs at most one computer action per computer delay.
    std::vector<bs::core::CellChange> tick(double dtSeconds);
    TurnIndicator turnIndicator() const;

private:
    void afterShot(const bs::core::ShotOutcome& outcome, bs::core::Side shooter);
    void afterHelp(const bs::core::HelpOutcome& outcome, bs::core::Side user);
    void finishOrPass(bool gameOver, bool turnPassed, bs::core::Side shooter);
    static ActionFeedback rejection(bs::core::ActionStatus status);

    bs::core::GameSession& session_;
    std::unique_ptr<bs::core::ITargetingStrategy> strategy_;
    bs::core::IRandom& rng_;
    IGameAudio& audio_;
    double computerDelay_;
    double elapsed_ = 0.0;
    bool computerFirstAction_ = true;
    Mode mode_ = Mode::AwaitingPlayer;
    Mode modeBeforeExit_ = Mode::AwaitingPlayer;
};

} // namespace app
} // namespace bs
