#pragma once

#include <functional>
#include <string>
#include <vector>

#include "cocos2d.h"

class GameServices;

// A developer tool for the layout pass (tasks T111, T129): when BS_SCREENSHOTS names a folder,
// the game plays itself through every screen and state in English, Russian and Vietnamese,
// renders each into a PNG there, restores the language and quits. It only clicks, hovers and
// presses keys like a player would, so it exercises the real scenes.
class ScreenshotTour {
public:
    ScreenshotTour(GameServices& services, std::string folder);
    void start();

private:
    using Step = std::function<float()>; // does something, returns seconds to wait before the next step

    void addLanguage(const std::string& code);
    void addBattleTour(const std::string& code);
    void addDefeatTour(const std::string& code);
    void shoot(const std::string& name);
    void next(float dt);

    void click(const cocos2d::Vec2& at) const;
    void hover(const cocos2d::Vec2& at) const;
    void key(cocos2d::EventKeyboard::KeyCode code) const;
    cocos2d::Vec2 targetCell(int row, int col) const;
    bool findComputerCell(bool ship, int& row, int& col) const;

    GameServices& services_;
    std::string folder_;
    std::vector<Step> steps_;
    std::size_t at_ = 0;
    float wait_ = 0.0f;
    int originalLanguage_ = 0;
    std::vector<bool> fired_;
};
