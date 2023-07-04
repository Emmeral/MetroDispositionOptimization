#pragma once

#include <limits>
namespace fb {


    inline unsigned int reverseLowestNBits(unsigned int value, unsigned int n) {
        // remove the lowest n bits
        auto base = value & (std::numeric_limits<unsigned int>::max() << n);
        for (auto i = 0; i < n; ++i) {
            base += ((value >> (n - 1 - i)) & (0b1)) * (0b1 << i);
        }
        return base;
    }
}// namespace fb
