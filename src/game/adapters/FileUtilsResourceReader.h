#pragma once

#include "bs/app/Ports.h"

// IResourceReader over cocos2d::FileUtils (paths relative to the Resources folder).
class FileUtilsResourceReader final : public bs::app::IResourceReader {
public:
    bool readText(const std::string& path, std::string& out) const override;
};
