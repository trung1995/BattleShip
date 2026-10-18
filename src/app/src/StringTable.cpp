#include "bs/app/StringTable.h"

#include <cctype>
#include <sstream>

namespace bs {
namespace app {

namespace {

std::string trimAscii(const std::string& s) {
    const char* spaces = " \t\r\n";
    const std::size_t begin = s.find_first_not_of(spaces);
    if (begin == std::string::npos) {
        return "";
    }
    const std::size_t end = s.find_last_not_of(spaces);
    return s.substr(begin, end - begin + 1);
}

// key := [a-z0-9_]+ { "." [a-z0-9_]+ }
bool isValidKey(const std::string& key) {
    if (key.empty() || key.front() == '.' || key.back() == '.') {
        return false;
    }
    char previous = '\0';
    for (const char c : key) {
        const bool segmentChar = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
        if (c == '.' ? previous == '.' : !segmentChar) {
            return false;
        }
        previous = c;
    }
    return true;
}

std::string unescape(const std::string& value) {
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size() && value[i + 1] == 'n') {
            out += '\n';
            ++i;
        } else {
            out += value[i];
        }
    }
    return out;
}

} // namespace

StringTable::ParseResult StringTable::parse(const std::string& text) {
    ParseResult result;
    std::string body = text;
    if (body.compare(0, 3, "\xEF\xBB\xBF") == 0) {
        body.erase(0, 3);
    }

    std::istringstream lines(body);
    std::string line;
    int lineNumber = 0;
    while (std::getline(lines, line)) {
        ++lineNumber;
        const std::string trimmed = trimAscii(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }
        const std::size_t equals = trimmed.find('=');
        if (equals == std::string::npos) {
            result.errorLine = lineNumber;
            result.error = "missing '=' in line: " + trimmed;
            return result;
        }
        const std::string key = trimAscii(trimmed.substr(0, equals));
        if (!isValidKey(key)) {
            result.errorLine = lineNumber;
            result.error = "invalid key: '" + key + "'";
            return result;
        }
        if (result.table.entries_.count(key) != 0) {
            result.errorLine = lineNumber;
            result.error = "duplicate key: " + key;
            return result;
        }
        result.table.entries_[key] = unescape(trimAscii(trimmed.substr(equals + 1)));
    }
    result.ok = true;
    return result;
}

const std::string* StringTable::find(const std::string& key) const {
    const auto it = entries_.find(key);
    return it == entries_.end() ? nullptr : &it->second;
}

std::set<std::string> StringTable::placeholders(const std::string& key) const {
    std::set<std::string> names;
    const std::string* value = find(key);
    if (value == nullptr) {
        return names;
    }
    std::size_t pos = 0;
    while ((pos = value->find('{', pos)) != std::string::npos) {
        const std::size_t close = value->find('}', pos + 1);
        if (close == std::string::npos) {
            break;
        }
        names.insert(value->substr(pos + 1, close - pos - 1));
        pos = close + 1;
    }
    return names;
}

} // namespace app
} // namespace bs
