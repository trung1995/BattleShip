#include "bs/app/SettingsService.h"

#include "bs/app/Utf8.h"

namespace bs {
namespace app {

namespace {
const char* const kSound = "settings.sound";
const char* const kMusic = "settings.music";
const char* const kLanguage = "settings.language";
const char* const kPlayerName = "settings.playerName";
} // namespace

SettingsService::SettingsService(ISettingsStore& store) : store_(store) {}

bool SettingsService::soundEnabled() const { return store_.getBool(kSound, true); }

void SettingsService::setSoundEnabled(bool enabled) {
    store_.setBool(kSound, enabled);
    store_.flush();
}

bool SettingsService::musicEnabled() const { return store_.getBool(kMusic, true); }

void SettingsService::setMusicEnabled(bool enabled) {
    store_.setBool(kMusic, enabled);
    store_.flush();
}

Language SettingsService::language() const {
    const std::string code = store_.getString(kLanguage, "en");
    if (code == "ru") {
        return Language::Ru;
    }
    if (code == "vi") {
        return Language::Vi;
    }
    return Language::En; // unknown values fall back to English
}

void SettingsService::setLanguage(Language language) {
    const char* code = language == Language::Ru ? "ru" : language == Language::Vi ? "vi" : "en";
    store_.setString(kLanguage, code);
    store_.flush();
}

std::string SettingsService::storedPlayerName() const { return store_.getString(kPlayerName, ""); }

NameResult SettingsService::setPlayerName(const std::string& utf8) {
    NameResult result;
    std::string name = trim(utf8);
    result.trimmed = name.size() != utf8.size();
    if (codePointCount(name) > kMaxNameCodePoints) {
        name = truncateCodePoints(name, kMaxNameCodePoints);
        result.truncated = true;
    }
    result.reset = name.empty();
    result.stored = name;
    store_.setString(kPlayerName, name);
    store_.flush();
    return result;
}

} // namespace app
} // namespace bs
