#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "bs/app/Services.h"
#include "bs/app/TextKeys.h"

// ILocalizer whose text() returns the key name, so tests can assert on keys directly.
class FakeLocalizer final : public bs::app::ILocalizer {
public:
    bs::app::Language language() const override { return language_; }

    void setLanguage(bs::app::Language language) override {
        language_ = language;
        languageCalls.push_back(language);
        for (const auto& entry : subscribers_) {
            entry.second();
        }
    }

    std::string text(bs::app::TextKey key) const override { return bs::app::keyName(key); }

    std::string format(bs::app::TextKey key,
                       const std::map<std::string, std::string>& args) const override {
        std::string out = bs::app::keyName(key);
        for (const auto& arg : args) {
            out += "|" + arg.first + "=" + arg.second;
        }
        return out;
    }

    bs::app::SubscriptionId subscribe(std::function<void()> onChanged) override {
        subscribers_[nextId_] = std::move(onChanged);
        return nextId_++;
    }

    void unsubscribe(bs::app::SubscriptionId id) override { subscribers_.erase(id); }

    std::string displayName(bs::core::Side side, const bs::app::ISettings& settings) const override {
        if (side == bs::core::Side::Computer) {
            return text(bs::app::TextKey::NameComputer);
        }
        const std::string stored = settings.storedPlayerName();
        return stored.empty() ? text(bs::app::TextKey::NamePlayerDefault) : stored;
    }

    std::vector<bs::app::Language> languageCalls;

private:
    bs::app::Language language_ = bs::app::Language::En;
    std::map<bs::app::SubscriptionId, std::function<void()>> subscribers_;
    bs::app::SubscriptionId nextId_ = 1;
};
