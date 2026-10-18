#pragma once

#include <string>
#include <utility>
#include <vector>

#include "bs/app/Services.h"
#include "bs/app/TextKeys.h"

namespace bs {
namespace app {

// Decisions for the settings screen (US5). Every change takes effect and persists immediately.
class SettingsController {
public:
    SettingsController(ISettings& settings, ILocalizer& localizer, IGameAudio& audio);

    void toggleSound();
    void toggleMusic();
    void selectLanguage(Language language); // live, no restart (FR-040)
    std::string filterNameInput(const std::string& utf8) const; // caps live input at 20 code points
    NameResult commitName(const std::string& utf8);             // FR-041
    std::vector<std::pair<Language, TextKey>> languages() const;

private:
    ISettings& settings_;
    ILocalizer& localizer_;
    IGameAudio& audio_;
};

} // namespace app
} // namespace bs
