#pragma once

#include <vector>

#include "bs/app/Services.h"

namespace bs {
namespace app {

// Every user-facing string key. Each must exist in Resources/i18n/{en,ru,vi}.lang
// (LocalizationCompletenessTest, Principle IV).
#define BS_TEXT_KEYS(X)                                                                           \
    X(AppTitle, "app.title")                                                                      \
    X(NameComputer, "name.computer")                                                              \
    X(NamePlayerDefault, "name.player_default")                                                    \
    X(LangEn, "lang.en")                                                                          \
    X(LangRu, "lang.ru")                                                                          \
    X(LangVi, "lang.vi")                                                                          \
    X(MenuNewGame, "menu.new_game")                                                               \
    X(MenuSettings, "menu.settings")                                                              \
    X(MenuQuit, "menu.quit")                                                                      \
    X(CommonBack, "common.back")                                                                  \
    X(PlacementTitle, "placement.title")                                                          \
    X(PlacementAutoAll, "placement.auto_all")                                                     \
    X(PlacementStart, "placement.start")                                                          \
    X(PlacementErrorFleetIncomplete, "placement.error.fleet_incomplete")                          \
    X(PlacementAutoRemaining, "placement.auto_remaining")                                         \
    X(PlacementRotate, "placement.rotate")                                                        \
    X(PlacementDock, "placement.dock")                                                            \
    X(PlacementDockCount, "placement.dock.count")                                                 \
    X(PlacementErrorOutOfBounds, "placement.error.out_of_bounds")                                 \
    X(PlacementErrorOverlaps, "placement.error.overlaps")                                         \
    X(PlacementErrorTouches, "placement.error.touches")                                           \
    X(PlacementErrorCannotFit, "placement.error.cannot_fit")                                      \
    X(PlacementHint, "placement.hint")                                                            \
    X(BattleTurnPlayer, "battle.turn.player")                                                     \
    X(BattleTurnComputer, "battle.turn.computer")                                                 \
    X(BattleBoardOwn, "battle.board.own")                                                         \
    X(BattleBoardTarget, "battle.board.target")                                                   \
    X(BattleRejectAlreadyFired, "battle.reject.already_fired")                                    \
    X(BattleRejectNotYourTurn, "battle.reject.not_your_turn")                                     \
    X(BattleRejectHelpNoEffect, "battle.reject.help_no_effect")                                   \
    X(BattleHelp, "battle.help")                                                                  \
    X(BattleHelpUsed, "battle.help.used")                                                         \
    X(BattleHelpHint, "battle.help.hint")                                                         \
    X(BattleComputerHelp, "battle.computer_help")                                                 \
    X(BattleWinner, "battle.winner")                                                              \
    X(GameOverNewGame, "gameover.new_game")                                                       \
    X(GameOverMenu, "gameover.menu")                                                              \
    X(DifficultyTitle, "difficulty.title")                                                        \
    X(DifficultyEasy, "difficulty.easy")                                                          \
    X(DifficultyMedium, "difficulty.medium")                                                      \
    X(DifficultyHard, "difficulty.hard")                                                          \
    X(DifficultyEasyDesc, "difficulty.easy.desc")                                                 \
    X(DifficultyMediumDesc, "difficulty.medium.desc")                                             \
    X(DifficultyHardDesc, "difficulty.hard.desc")                                                 \
    X(GameOverVictory, "gameover.victory")                                                        \
    X(GameOverDefeat, "gameover.defeat")                                                          \
    X(SettingsTitle, "settings.title")                                                            \
    X(SettingsSound, "settings.sound")                                                            \
    X(SettingsMusic, "settings.music")                                                            \
    X(SettingsLanguage, "settings.language")                                                      \
    X(SettingsPlayerName, "settings.player_name")                                                 \
    X(SettingsOn, "settings.on")                                                                  \
    X(SettingsOff, "settings.off")                                                                \
    X(SettingsNamePlaceholder, "settings.name_placeholder")                                       \
    X(ExitTitle, "exit.title")                                                                    \
    X(ExitMessage, "exit.message")                                                                \
    X(ExitStay, "exit.stay")                                                                      \
    X(ExitLeave, "exit.leave")                                                                    \
    X(BattleMenu, "battle.menu")

enum class TextKey : int {
#define BS_TEXT_KEY_ENUM(id, name) id,
    BS_TEXT_KEYS(BS_TEXT_KEY_ENUM)
#undef BS_TEXT_KEY_ENUM
};

const char* keyName(TextKey key);
std::vector<TextKey> allTextKeys();

} // namespace app
} // namespace bs
