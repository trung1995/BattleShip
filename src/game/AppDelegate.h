#pragma once

#include <memory>

#include "cocos2d.h"

class GameServices;

class AppDelegate final : public cocos2d::Application {
public:
    AppDelegate();
    ~AppDelegate() override;

    void initGLContextAttrs() override;
    bool applicationDidFinishLaunching() override;
    void applicationDidEnterBackground() override;
    void applicationWillEnterForeground() override;

private:
    void updateWindowTitle();

    std::unique_ptr<GameServices> services_;
};
