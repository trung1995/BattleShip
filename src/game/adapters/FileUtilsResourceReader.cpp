#include "adapters/FileUtilsResourceReader.h"

#include "cocos2d.h"

bool FileUtilsResourceReader::readText(const std::string& path, std::string& out) const {
    out = cocos2d::FileUtils::getInstance()->getStringFromFile(path);
    return !out.empty();
}
