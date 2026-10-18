#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace bs {
namespace core {

// Source of randomness for the rules core. Everything random goes through this interface so
// tests can script it and the same seed gives the same game everywhere (Principles I and VI).
class IRandom {
public:
    virtual ~IRandom() = default;

    // Uniform integer in [lo, hi], both inclusive. Precondition: lo <= hi.
    virtual int nextInt(int lo, int hi) = 0;
};

// std::mt19937 with a portable rejection-sampling mapping (research R6). Unlike
// std::uniform_int_distribution, the mapping is identical across standard libraries.
class Mt19937Random final : public IRandom {
public:
    explicit Mt19937Random(std::uint32_t seed);

    int nextInt(int lo, int hi) override;

private:
    std::mt19937 engine_;
};

// Fisher–Yates shuffle driven by IRandom.
template <class T> void shuffle(std::vector<T>& items, IRandom& rng) {
    for (std::size_t i = items.size(); i > 1; --i) {
        const auto j = static_cast<std::size_t>(rng.nextInt(0, static_cast<int>(i - 1)));
        std::swap(items[i - 1], items[j]);
    }
}

} // namespace core
} // namespace bs
