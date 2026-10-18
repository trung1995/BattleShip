#pragma once

#include <cstddef>
#include <string>

namespace bs {
namespace app {

// Number of Unicode code points in a UTF-8 string (research R11).
std::size_t codePointCount(const std::string& utf8);

// The first `maxCodePoints` code points; never splits a multibyte sequence.
std::string truncateCodePoints(const std::string& utf8, std::size_t maxCodePoints);

// Removes ASCII and Unicode whitespace from both ends.
std::string trim(const std::string& utf8);

} // namespace app
} // namespace bs
