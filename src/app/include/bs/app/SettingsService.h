#pragma once

#include <cstddef>
#include <string>

#include "bs/app/Ports.h"
#include "bs/app/Services.h"

namespace bs {
namespace app {

// Player preferences, persisted through ISettingsStore on every change (FR-043).
// Defaults: sound on, music on, language `en`, name "" (displayed as name.player_default).
class SettingsService final : public ISettings {
public:
    static constexpr std::size_t kMaxNameCodePoints = 20;

    explicit SettingsService(ISettingsStore& store);

    bool soundEnabled() const override;
    void setSoundEnabled(bool enabled) override;
    bool musicEnabled() const override;
    void setMusicEnabled(bool enabled) override;
    Language language() const override;
    void setLanguage(Language language) override;
    std::string storedPlayerName() const override;
    // Trims; empty → reset to the default; more than 20 code points → truncated (FR-041).
    NameResult setPlayerName(const std::string& utf8) override;

private:
    ISettingsStore& store_;
};

} // namespace app
} // namespace bs
