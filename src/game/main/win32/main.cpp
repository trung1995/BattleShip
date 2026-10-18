#include "main.h"

#include "AppDelegate.h"
#include "cocos2d.h"

int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/,
                     int /*nCmdShow*/) {
    AppDelegate app;
    return cocos2d::Application::getInstance()->run();
}
