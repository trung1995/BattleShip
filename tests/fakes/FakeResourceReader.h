#pragma once

#include <map>
#include <string>

#include "bs/app/Ports.h"

class FakeResourceReader final : public bs::app::IResourceReader {
public:
    bool readText(const std::string& path, std::string& out) const override {
        const auto it = files.find(path);
        if (it == files.end()) {
            return false;
        }
        out = it->second;
        return true;
    }

    std::map<std::string, std::string> files;
};
