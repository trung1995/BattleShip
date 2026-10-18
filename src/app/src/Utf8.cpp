#include "bs/app/Utf8.h"

#include <cstdint>

namespace bs {
namespace app {

namespace {

// Length of the sequence starting with `lead`; invalid lead bytes count as one byte.
std::size_t sequenceLength(unsigned char lead) {
    if (lead < 0x80) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 1;
}

std::uint32_t decodeAt(const std::string& s, std::size_t pos, std::size_t length) {
    const auto lead = static_cast<unsigned char>(s[pos]);
    if (length == 1) {
        return lead;
    }
    std::uint32_t cp = lead & (0xFFu >> (length + 1));
    for (std::size_t i = 1; i < length; ++i) {
        cp = (cp << 6) | (static_cast<unsigned char>(s[pos + i]) & 0x3Fu);
    }
    return cp;
}

bool isWhitespace(std::uint32_t cp) {
    return (cp >= 0x09 && cp <= 0x0D) || cp == 0x20 || cp == 0x85 || cp == 0xA0 || cp == 0x1680 ||
           (cp >= 0x2000 && cp <= 0x200A) || cp == 0x2028 || cp == 0x2029 || cp == 0x202F ||
           cp == 0x205F || cp == 0x3000;
}

bool isContinuation(unsigned char byte) { return (byte & 0xC0) == 0x80; }

} // namespace

std::size_t codePointCount(const std::string& utf8) {
    std::size_t count = 0;
    for (std::size_t pos = 0; pos < utf8.size();
         pos += sequenceLength(static_cast<unsigned char>(utf8[pos]))) {
        ++count;
    }
    return count;
}

std::string truncateCodePoints(const std::string& utf8, std::size_t maxCodePoints) {
    std::size_t pos = 0;
    for (std::size_t count = 0; pos < utf8.size() && count < maxCodePoints; ++count) {
        pos += sequenceLength(static_cast<unsigned char>(utf8[pos]));
    }
    return utf8.substr(0, pos < utf8.size() ? pos : utf8.size());
}

std::string trim(const std::string& utf8) {
    std::size_t begin = 0;
    while (begin < utf8.size()) {
        std::size_t length = sequenceLength(static_cast<unsigned char>(utf8[begin]));
        if (begin + length > utf8.size() || !isWhitespace(decodeAt(utf8, begin, length))) {
            break;
        }
        begin += length;
    }

    std::size_t end = utf8.size();
    while (end > begin) {
        std::size_t start = end - 1;
        while (start > begin && isContinuation(static_cast<unsigned char>(utf8[start]))) {
            --start;
        }
        if (!isWhitespace(decodeAt(utf8, start, end - start))) {
            break;
        }
        end = start;
    }
    return utf8.substr(begin, end - begin);
}

} // namespace app
} // namespace bs
