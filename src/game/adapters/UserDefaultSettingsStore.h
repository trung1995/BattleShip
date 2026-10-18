#pragma once

#include "bs/app/Ports.h"

// ISettingsStore over cocos2d::UserDefault, the engine's per-user store.
class UserDefaultSettingsStore final : public bs::app::ISettingsStore {
public:
    bool getBool(const std::string& key, bool fallback) const override;
    std::string getString(const std::string& key, const std::string& fallback) const override;
    void setBool(const std::string& key, bool value) override;
    void setString(const std::string& key, const std::string& value) override;
    void flush() override;
};
