#pragma once

#include <functional>
#include <map>
#include <string>

#include "bs/app/Ports.h"
#include "bs/app/Services.h"
#include "bs/app/StringTable.h"

namespace bs {
namespace app {

// Loads i18n/<code>.lang through IResourceReader and serves translated text. A language change
// is applied synchronously and notifies every subscriber before setLanguage returns (FR-040).
class Localizer final : public ILocalizer {
public:
    Localizer(IResourceReader& reader, Language initial);

    Language language() const override { return language_; }
    void setLanguage(Language language) override;
    std::string text(TextKey key) const override;
    std::string format(TextKey key, const std::map<std::string, std::string>& args) const override;
    SubscriptionId subscribe(std::function<void()> onChanged) override;
    void unsubscribe(SubscriptionId id) override;
    std::string displayName(bs::core::Side side, const ISettings& settings) const override;

    static const char* languageCode(Language language);

private:
    void load();

    IResourceReader& reader_;
    Language language_;
    StringTable table_;
    std::map<SubscriptionId, std::function<void()>> subscribers_;
    SubscriptionId nextId_ = 1;
};

} // namespace app
} // namespace bs
