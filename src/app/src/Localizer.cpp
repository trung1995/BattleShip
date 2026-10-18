#include "bs/app/Localizer.h"

#include <vector>

#include "bs/app/TextKeys.h"

namespace bs {
namespace app {

Localizer::Localizer(IResourceReader& reader, Language initial)
    : reader_(reader), language_(initial) {
    load();
}

const char* Localizer::languageCode(Language language) {
    switch (language) {
    case Language::En:
        return "en";
    case Language::Ru:
        return "ru";
    case Language::Vi:
        return "vi";
    }
    return "en";
}

void Localizer::load() {
    std::string contents;
    table_ = StringTable();
    if (reader_.readText(std::string("i18n/") + languageCode(language_) + ".lang", contents)) {
        StringTable::ParseResult result = StringTable::parse(contents);
        if (result.ok) {
            table_ = result.table;
        }
    }
}

void Localizer::setLanguage(Language language) {
    language_ = language;
    load();
    // Copy the IDs first: a subscriber may unsubscribe itself while being notified.
    std::vector<SubscriptionId> ids;
    for (const auto& entry : subscribers_) {
        ids.push_back(entry.first);
    }
    for (const SubscriptionId id : ids) {
        const auto it = subscribers_.find(id);
        if (it != subscribers_.end()) {
            it->second();
        }
    }
}

std::string Localizer::text(TextKey key) const {
    const std::string* value = table_.find(keyName(key));
    return value != nullptr ? *value : std::string(keyName(key));
}

std::string Localizer::format(TextKey key, const std::map<std::string, std::string>& args) const {
    std::string out = text(key);
    for (const auto& arg : args) {
        const std::string placeholder = "{" + arg.first + "}";
        std::size_t pos = 0;
        while ((pos = out.find(placeholder, pos)) != std::string::npos) {
            out.replace(pos, placeholder.size(), arg.second);
            pos += arg.second.size();
        }
    }
    return out;
}

SubscriptionId Localizer::subscribe(std::function<void()> onChanged) {
    subscribers_[nextId_] = std::move(onChanged);
    return nextId_++;
}

void Localizer::unsubscribe(SubscriptionId id) { subscribers_.erase(id); }

std::string Localizer::displayName(bs::core::Side side, const ISettings& settings) const {
    if (side == bs::core::Side::Computer) {
        return text(TextKey::NameComputer);
    }
    const std::string stored = settings.storedPlayerName();
    return stored.empty() ? text(TextKey::NamePlayerDefault) : stored;
}

} // namespace app
} // namespace bs
