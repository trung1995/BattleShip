#include "scenes/ConfirmDialog.h"

#include <new>

#include "Theme.h"
#include "nodes/Decor.h"
#include "nodes/LocalizedLabel.h"
#include "nodes/TextButton.h"

ConfirmDialog* ConfirmDialog::create(bs::app::ILocalizer& localizer, const Spec& spec,
                                     std::function<void()> onCancel, std::function<void()> onConfirm) {
    auto* dialog = new (std::nothrow) ConfirmDialog(std::move(onCancel), std::move(onConfirm));
    if (dialog != nullptr && dialog->initDialog(localizer, spec)) {
        dialog->autorelease();
        return dialog;
    }
    delete dialog;
    return nullptr;
}

// Cancel is the safe brass default; confirm is the red wax "destructive" choice.
bool ConfirmDialog::initDialog(bs::app::ILocalizer& localizer, const Spec& spec) {
    if (!LayerColor::initWithColor(cocos2d::Color4B(10, 6, 3, 179))) {
        return false;
    }
    auto sheet = decor::canvas();
    decor::panel(sheet, cocos2d::Rect(360, 236, 560, 248), 12.0f, decor::woodPanel());
    addChild(sheet);

    auto title = LocalizedLabel::forKey(localizer, spec.title, 36.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(480.0f);
    title->setPosition(640, 420);
    addChild(title);
    auto message = LocalizedLabel::forKey(localizer, spec.message, 20.0f);
    message->setTextColor(theme::kInk);
    message->setMaxWidth(480.0f);
    message->setPosition(640, 370);
    addChild(message);

    const cocos2d::Size buttonSize(200, 54);
    auto cancel = TextButton::forKey(
        localizer, spec.cancelLabel, buttonSize,
        [this] {
            auto callback = onCancel_;
            removeFromParent();
            if (callback) callback();
        },
        TextButton::Style::Primary);
    cancel->setPosition(533, 305);
    addChild(cancel);
    auto confirm = TextButton::forKey(
        localizer, spec.confirmLabel, buttonSize,
        [this] {
            auto callback = onConfirm_;
            removeFromParent();
            if (callback) callback();
        },
        TextButton::Style::Destructive);
    confirm->setPosition(747, 305);
    addChild(confirm);

    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
