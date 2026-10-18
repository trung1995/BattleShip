#include "adapters/UserDefaultSettingsStore.h"

#include "cocos2d.h"

bool UserDefaultSettingsStore::getBool(const std::string& key, bool fallback) const {
    return cocos2d::UserDefault::getInstance()->getBoolForKey(key.c_str(), fallback);
}

std::string UserDefaultSettingsStore::getString(const std::string& key,
                                                const std::string& fallback) const {
    return cocos2d::UserDefault::getInstance()->getStringForKey(key.c_str(), fallback);
}

void UserDefaultSettingsStore::setBool(const std::string& key, bool value) {
    cocos2d::UserDefault::getInstance()->setBoolForKey(key.c_str(), value);
}

void UserDefaultSettingsStore::setString(const std::string& key, const std::string& value) {
    cocos2d::UserDefault::getInstance()->setStringForKey(key.c_str(), value);
}

void UserDefaultSettingsStore::flush() { cocos2d::UserDefault::getInstance()->flush(); }
