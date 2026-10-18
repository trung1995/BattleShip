#pragma once

#include <functional>

#include "bs/app/Services.h"
#include "bs/app/TextKeys.h"
#include "cocos2d.h"

// A reusable modal: dimmed background that swallows clicks, a title, a message, two buttons.
class ConfirmDialog final : public cocos2d::LayerColor {
public:
    struct Spec {
        bs::app::TextKey title;
        bs::app::TextKey message;
        bs::app::TextKey cancelLabel;
        bs::app::TextKey confirmLabel;
    };

    static ConfirmDialog* create(bs::app::ILocalizer& localizer, const Spec& spec,
                                 std::function<void()> onCancel, std::function<void()> onConfirm);

private:
    ConfirmDialog(std::function<void()> onCancel, std::function<void()> onConfirm)
        : onCancel_(std::move(onCancel)), onConfirm_(std::move(onConfirm)) {}
    bool initDialog(bs::app::ILocalizer& localizer, const Spec& spec);

    std::function<void()> onCancel_;
    std::function<void()> onConfirm_;
};
