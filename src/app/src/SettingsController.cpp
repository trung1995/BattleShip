#include "bs/app/SettingsController.h"

#include "bs/app/SettingsService.h"
#include "bs/app/Utf8.h"

namespace bs {
namespace app {

SettingsController::SettingsController(ISettings& settings, ILocalizer& localizer, IGameAudio& audio)
    : settings_(settings), localizer_(localizer), audio_(audio) {}

void SettingsController::toggleSound() {
    settings_.setSoundEnabled(!settings_.soundEnabled());
    audio_.onSettingsChanged();
}

void SettingsController::toggleMusic() {
    settings_.setMusicEnabled(!settings_.musicEnabled());
    audio_.onSettingsChanged(); // stops or restarts the music at once
}

void SettingsController::selectLanguage(Language language) {
    settings_.setLanguage(language);
    localizer_.setLanguage(language);
}

std::string SettingsController::filterNameInput(const std::string& utf8) const {
    return truncateCodePoints(utf8, SettingsService::kMaxNameCodePoints);
}

NameResult SettingsController::commitName(const std::string& utf8) { return settings_.setPlayerName(utf8); }

std::vector<std::pair<Language, TextKey>> SettingsController::languages() const {
    return {{Language::En, TextKey::LangEn}, {Language::Ru, TextKey::LangRu}, {Language::Vi, TextKey::LangVi}};
}

} // namespace app
} // namespace bs
