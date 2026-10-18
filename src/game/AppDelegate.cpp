#include "AppDelegate.h"

#include "GameServices.h"
#include "bs/app/TextKeys.h"
#include "audio/include/AudioEngine.h"

namespace {
const cocos2d::Size kDesignSize(1280, 720);
}

AppDelegate::AppDelegate() = default;
AppDelegate::~AppDelegate() = default;

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs attrs = {8, 8, 8, 8, 24, 8, 0};
    cocos2d::GLView::setGLContextAttrs(attrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    auto director = cocos2d::Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview) {
        glview = cocos2d::GLViewImpl::createWithRect(
            "", cocos2d::Rect(0, 0, kDesignSize.width, kDesignSize.height), 1.0f, true);
        director->setOpenGLView(glview);
    }
    glview->setDesignResolutionSize(kDesignSize.width, kDesignSize.height, ResolutionPolicy::SHOW_ALL);
    director->setAnimationInterval(1.0f / 60);

    services_ = std::make_unique<GameServices>();
    updateWindowTitle(); // the window was created untitled; the title is localized text
    services_->localizer().subscribe([this] { updateWindowTitle(); });
    services_->navigator().showMenu();
    return true;
}

// The window title follows the language like every other text (FR-040).
void AppDelegate::updateWindowTitle() {
    auto glview = dynamic_cast<cocos2d::GLViewImpl*>(cocos2d::Director::getInstance()->getOpenGLView());
    if (glview != nullptr && glview->getWindow() != nullptr) {
        glfwSetWindowTitle(glview->getWindow(), services_->localizer().text(bs::app::TextKey::AppTitle).c_str());
    }
}

void AppDelegate::applicationDidEnterBackground() {
    cocos2d::Director::getInstance()->stopAnimation();
    cocos2d::AudioEngine::pauseAll();
}

void AppDelegate::applicationWillEnterForeground() {
    cocos2d::Director::getInstance()->startAnimation();
    cocos2d::AudioEngine::resumeAll();
}
