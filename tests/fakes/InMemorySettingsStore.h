#pragma once

#include <map>
#include <string>

#include "bs/app/Ports.h"

class InMemorySettingsStore final : public bs::app::ISettingsStore {
public:
    bool getBool(const std::string& key, bool fallback) const override {
        const auto it = bools.find(key);
        return it == bools.end() ? fallback : it->second;
    }
    std::string getString(const std::string& key, const std::string& fallback) const override {
        const auto it = strings.find(key);
        return it == strings.end() ? fallback : it->second;
    }
    void setBool(const std::string& key, bool value) override { bools[key] = value; }
    void setString(const std::string& key, const std::string& value) override {
        strings[key] = value;
    }
    void flush() override { ++flushCount; }

    std::map<std::string, bool> bools;
    std::map<std::string, std::string> strings;
    int flushCount = 0;
};
