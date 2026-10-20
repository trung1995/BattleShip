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

// Cancel is the safe gold default; confirm is the blood-red "destructive" choice. The dialog is
// a torn sheet over the dimmed battle, tilted a little.
bool ConfirmDialog::initDialog(bs::app::ILocalizer& localizer, const Spec& spec) {
    if (!LayerColor::initWithColor(cocos2d::Color4B(4, 5, 6, 199))) {
        return false;
    }
    auto sheet = cocos2d::Node::create();
    sheet->setPosition(640, 360);
    sheet->setRotation(-0.8f);
    addChild(sheet);
    auto paper = decor::canvas();
    decor::tornSheet(paper, cocos2d::Rect(-290, -116, 580, 232), theme::kParchment, theme::kParchmentBurn, 40.0f);
    sheet->addChild(paper);

    auto title = LocalizedLabel::forKey(localizer, spec.title, 36.0f, theme::Face::CapsHeavy);
    title->setTextColor(theme::kInk);
    title->setMaxWidth(500.0f);
    title->setPosition(0, 60);
    sheet->addChild(title);
    auto message = LocalizedLabel::forKey(localizer, spec.message, 20.0f);
    message->setTextColor(theme::kInk);
    message->setMaxWidth(500.0f);
    message->setPosition(0, 10);
    sheet->addChild(message);

    const cocos2d::Size buttonSize(200, 54);
    auto cancel = TextButton::forKey(
        localizer, spec.cancelLabel, buttonSize,
        [this] {
            auto callback = onCancel_;
            removeFromParent();
            if (callback) callback();
        },
        TextButton::Style::Safe);
    cancel->setPosition(-107, -53);
    sheet->addChild(cancel);
    auto confirm = TextButton::forKey(
        localizer, spec.confirmLabel, buttonSize,
        [this] {
            auto callback = onConfirm_;
            removeFromParent();
            if (callback) callback();
        },
        TextButton::Style::Destructive);
    confirm->setPosition(107, -53);
    sheet->addChild(confirm);

    auto blocker = cocos2d::EventListenerTouchOneByOne::create();
    blocker->setSwallowTouches(true);
    blocker->onTouchBegan = [](cocos2d::Touch*, cocos2d::Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(blocker, this);
    return true;
}
