#pragma once

#include <deque>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "bs/core/Random.h"

// IRandom that returns queued values in order, then falls back to lo.
class ScriptedRandom final : public bs::core::IRandom {
public:
    ScriptedRandom() = default;
    ScriptedRandom(std::initializer_list<int> values) : values_(values) {}

    void push(int value) { values_.push_back(value); }
    int calls() const { return calls_; }
    // The (lo, hi) of every call, in order.
    const std::vector<std::pair<int, int>>& ranges() const { return ranges_; }

    int nextInt(int lo, int hi) override {
        ++calls_;
        ranges_.emplace_back(lo, hi);
        if (values_.empty()) {
            return lo;
        }
        const int value = values_.front();
        values_.pop_front();
        if (value < lo || value > hi) {
            ADD_FAILURE() << "ScriptedRandom value " << value << " outside [" << lo << ", " << hi
                          << "]";
            return lo;
        }
        return value;
    }

private:
    std::deque<int> values_;
    int calls_ = 0;
    std::vector<std::pair<int, int>> ranges_;
};
