#include "tools/ScreenshotTour.h"

#include <utility>

#include "GameServices.h"
#include "Theme.h"
#include "bs/core/Fleet.h"
#include "nodes/BoardNode.h"

using bs::app::Language;
using bs::core::Side;
using cocos2d::Vec2;

namespace {
// The battle screen's target grid (BattleScene), bottom-left corner.
constexpr float kTargetGridX = 819.0f;
constexpr float kGridY = 162.0f;
} // namespace

ScreenshotTour::ScreenshotTour(GameServices& services, std::string folder)
    : services_(services), folder_(std::move(folder)) {}

void ScreenshotTour::start() {
    originalLanguage_ = static_cast<int>(services_.settings().language());
    for (const char* code : {"en", "ru", "vi", "uk"}) {
        addLanguage(code);
    }
    steps_.push_back([this] {
        const auto language = static_cast<Language>(originalLanguage_);
        services_.settings().setLanguage(language);
        services_.localizer().setLanguage(language);
        cocos2d::Director::getInstance()->getScheduler()->setTimeScale(1.0f);
        cocos2d::log("Screenshot tour finished: %s", folder_.c_str());
        cocos2d::Director::getInstance()->end();
        return 0.0f;
    });
    cocos2d::Director::getInstance()->getScheduler()->schedule([this](float dt) { next(dt); }, this, 0.0f, false,
                                                              "screenshot-tour");
}

void ScreenshotTour::next(float dt) {
    // Real seconds, not scaled: the defeat tour speeds the game up.
    wait_ -= dt / cocos2d::Director::getInstance()->getScheduler()->getTimeScale();
    while (wait_ <= 0.0f && at_ < steps_.size()) { // steps with nothing to wait for run back to back
        wait_ = steps_[at_++]();
    }
}

void ScreenshotTour::addLanguage(const std::string& code) {
    steps_.push_back([this, code] {
        const Language language = code == "ru" ? Language::Ru : code == "vi" ? Language::Vi : code == "uk" ? Language::Uk : Language::En;
        services_.settings().setLanguage(language);
        services_.localizer().setLanguage(language);
        services_.navigator().showMenu();
        return 0.8f;
    });
    steps_.push_back([this] {
        hover({286, 336}); // New Game in its hover state, as in the mockup
        return 0.2f;
    });
    shoot(code + "-01-menu");
    steps_.push_back([this] {
        services_.navigator().showNewGame();
        return 0.8f;
    });
    steps_.push_back([this] {
        hover({640, 267}); // the Hard row
        return 0.2f;
    });
    shoot(code + "-02-difficulty");
    steps_.push_back([this] {
        services_.navigator().showSettings();
        return 0.8f;
    });
    shoot(code + "-03-settings");
    steps_.push_back([this] {
        services_.navigator().showPlacement(bs::core::Difficulty::Hard);
        return 0.8f;
    });
    shoot(code + "-04-placement");
    addBattleTour(code);
    addDefeatTour(code);
}

void ScreenshotTour::addBattleTour(const std::string& code) {
    steps_.push_back([this] {
        click({771.5f, 209}); // Auto-place all
        return 0.2f;
    });
    steps_.push_back([this] {
        click({1048.5f, 209}); // Start battle
        fired_.assign(100, false);
        return 0.9f;
    });
    steps_.push_back([this] {
        int row = 0, col = 0;
        if (findComputerCell(true, row, col)) {
            click(targetCell(row, col)); // a hit: the extra shot
            fired_[static_cast<std::size_t>(row * 10 + col)] = true;
        }
        int er = 0, ec = 0;
        if (findComputerCell(false, er, ec)) {
            hover(targetCell(er, ec));
        }
        return 0.4f;
    });
    shoot(code + "-05-battle-extra-shot");
    steps_.push_back([this] {
        click({640, 395}); // Help, armed for the extra shot
        int row = 0, col = 0;
        findComputerCell(false, row, col);
        hover(targetCell(row, col));
        return 0.3f;
    });
    shoot(code + "-06-battle-help-armed");
    // Use the Help, then keep missing until the computer (Hard) unleashes its own.
    steps_.push_back([this] {
        int row = 0, col = 0;
        findComputerCell(false, row, col);
        click(targetCell(row, col));
        for (const Vec2 d : {Vec2(0, 0), Vec2(-1, 0), Vec2(1, 0), Vec2(0, -1), Vec2(0, 1)}) {
            const int r = row + static_cast<int>(d.x), c = col + static_cast<int>(d.y);
            if (r >= 0 && r < 10 && c >= 0 && c < 10) fired_[static_cast<std::size_t>(r * 10 + c)] = true;
        }
        return 0.6f;
    });
    for (int i = 0; i < 200; ++i) {
        steps_.push_back([this] {
            auto session = services_.currentSession;
            if (!session || session->phase() != bs::core::Phase::Battle) return 0.0f;
            if (session->helpUsesLeft(Side::Computer) < bs::core::kHelpUsesPerGame) {
                return 0.0f; // it happened: the next steps take the picture
            }
            if (session->currentTurn() == Side::Player) {
                int row = 0, col = 0;
                if (findComputerCell(false, row, col)) {
                    click(targetCell(row, col));
                    fired_[static_cast<std::size_t>(row * 10 + col)] = true;
                }
            }
            return 0.25f;
        });
    }
    steps_.push_back([] { return 0.15f; }); // mid-flare
    shoot(code + "-07-battle-computer-help");
    steps_.push_back([this] {
        key(cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE);
        return 0.4f;
    });
    shoot(code + "-08-leave-dialog");
    steps_.push_back([this] {
        click({640 - 107, 360 - 53}); // Stay
        return 1.8f;                   // let the computer finish its turn
    });
    // Sink every remaining enemy ship; hits keep the turn, so this ends in victory.
    steps_.push_back([this] {
        auto session = services_.currentSession;
        if (!session) return 0.0f;
        const bs::core::Fleet& fleet = session->ownFleet(Side::Computer);
        for (bs::core::ShipId id = 0; id < bs::core::kFleetSize; ++id) {
            for (const bs::core::Coord c : fleet.placementOf(id)->cells()) {
                if (session->phase() == bs::core::Phase::Battle && session->currentTurn() == Side::Player &&
                    session->viewOf(Side::Computer).at(c) == bs::core::Knowledge::Unknown) {
                    click(targetCell(c.row, c.col));
                }
            }
        }
        return 1.2f;
    });
    shoot(code + "-09-victory");
}

void ScreenshotTour::addDefeatTour(const std::string& code) {
    steps_.push_back([this] {
        services_.navigator().showPlacement(bs::core::Difficulty::Hard);
        return 0.8f;
    });
    steps_.push_back([this] {
        click({771.5f, 209});
        return 0.2f;
    });
    steps_.push_back([this] {
        click({1048.5f, 209});
        fired_.assign(100, false);
        cocos2d::Director::getInstance()->getScheduler()->setTimeScale(12.0f); // the computer fires fast
        return 0.9f;
    });
    for (int i = 0; i < 120; ++i) {
        steps_.push_back([this] {
            auto session = services_.currentSession;
            if (!session || session->phase() != bs::core::Phase::Battle) return 0.0f;
            if (session->currentTurn() == Side::Player) {
                int row = 0, col = 0;
                if (findComputerCell(false, row, col)) {
                    click(targetCell(row, col));
                    fired_[static_cast<std::size_t>(row * 10 + col)] = true;
                }
            }
            return 0.1f;
        });
    }
    steps_.push_back([] {
        cocos2d::Director::getInstance()->getScheduler()->setTimeScale(1.0f);
        return 1.0f;
    });
    shoot(code + "-10-defeat");
}

void ScreenshotTour::shoot(const std::string& name) {
    steps_.push_back([this, name] {
        cocos2d::utils::captureScreen([](bool, const std::string&) {}, folder_ + "/" + name + ".png");
        return 0.3f;
    });
}

void ScreenshotTour::click(const Vec2& at) const {
    auto* dispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
    auto* touch = new cocos2d::Touch();
    touch->setTouchInfo(0, at.x, 720.0f - at.y); // UI coordinates: y grows downwards
    cocos2d::EventTouch began;
    began.setEventCode(cocos2d::EventTouch::EventCode::BEGAN);
    began.setTouches({touch});
    dispatcher->dispatchEvent(&began);
    cocos2d::EventTouch ended;
    ended.setEventCode(cocos2d::EventTouch::EventCode::ENDED);
    ended.setTouches({touch});
    dispatcher->dispatchEvent(&ended);
    touch->release();
}

void ScreenshotTour::hover(const Vec2& at) const {
    cocos2d::EventMouse move(cocos2d::EventMouse::MouseEventType::MOUSE_MOVE);
    move.setCursorPosition(at.x, at.y);
    cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&move);
}

void ScreenshotTour::key(cocos2d::EventKeyboard::KeyCode code) const {
    cocos2d::EventKeyboard released(code, false);
    cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&released);
}

Vec2 ScreenshotTour::targetCell(int row, int col) const {
    const float size = theme::kCellSize;
    return Vec2(kTargetGridX + size * (static_cast<float>(col) + 0.5f),
                kGridY + size * (static_cast<float>(9 - row) + 0.5f));
}

// The first enemy cell not yet fired at that holds (or does not hold) a ship. The tour peeks at
// the computer's fleet only to stage its pictures; the game itself never does (Principle III).
bool ScreenshotTour::findComputerCell(bool ship, int& row, int& col) const {
    auto session = services_.currentSession;
    if (!session) return false;
    const bs::core::Fleet& fleet = session->ownFleet(Side::Computer);
    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 10; ++c) {
            const bs::core::Coord cell{r, c};
            if (fired_[static_cast<std::size_t>(r * 10 + c)] ||
                session->viewOf(Side::Computer).at(cell) != bs::core::Knowledge::Unknown) {
                continue;
            }
            if ((fleet.shipAt(cell) >= 0) == ship) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}
