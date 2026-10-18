#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>

namespace bs {
namespace app {

// A parsed language file: flat `key = value` lines (contracts/localization.md).
class StringTable {
public:
    struct ParseResult;

    static ParseResult parse(const std::string& text);

    const std::string* find(const std::string& key) const; // nullptr if missing
    std::set<std::string> placeholders(const std::string& key) const;
    const std::map<std::string, std::string>& entries() const { return entries_; }
    std::size_t size() const { return entries_.size(); }

private:
    std::map<std::string, std::string> entries_;
};

struct StringTable::ParseResult {
    bool ok = false;
    int errorLine = 0; // 1-based line of the first error
    std::string error;
    StringTable table;
};

} // namespace app
} // namespace bs
