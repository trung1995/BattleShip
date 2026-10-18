#include "bs/core/Random.h"

#include <cassert>

namespace bs {
namespace core {

Mt19937Random::Mt19937Random(std::uint32_t seed) : engine_(seed) {}

// Rejection sampling over raw 32-bit output (research R6): identical on every standard library.
int Mt19937Random::nextInt(int lo, int hi) {
    assert(lo <= hi);
    const std::uint64_t kSpan = std::uint64_t{1} << 32;
    const std::uint64_t range =
        static_cast<std::uint64_t>(static_cast<std::int64_t>(hi) - static_cast<std::int64_t>(lo)) + 1;
    if (range == kSpan) {
        return static_cast<int>(static_cast<std::int64_t>(lo) + static_cast<std::int64_t>(engine_()));
    }
    const std::uint64_t limit = kSpan - (kSpan % range);
    std::uint64_t x = 0;
    do {
        x = static_cast<std::uint64_t>(engine_());
    } while (x >= limit);
    return static_cast<int>(static_cast<std::int64_t>(lo) + static_cast<std::int64_t>(x % range));
}

} // namespace core
} // namespace bs
