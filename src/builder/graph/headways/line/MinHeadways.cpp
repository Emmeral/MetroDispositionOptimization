
#include "MinHeadways.h"

namespace fb {


    MinHeadways::MinHeadways(const std::array<seconds_t, 16> &headways) {

        for (auto i = 0; i < 16; ++i) {
            auto new_index = (i & (0x1)) + ((i & (0x2)) >> 1) * 3 + ((i & 0x4) >> 2) * 9 + ((i & 0x8) >> 3) * 27;
            headways_[new_index] = headways[i];
        }

        for (auto i = 0; i < 81; ++i) {

            if ((i % 81) > 53) {
                headways_[i] = std::min(headways_[i - 27], headways_[i - 54]);
            } else if ((i % 27) > 17) {
                headways_[i] = std::min(headways_[i - 9], headways_[i - 18]);
            } else if ((i % 9) > 5) {
                headways_[i] = std::min(headways_[i - 3], headways_[i - 6]);
            } else if ((i % 3) == 2) {
                headways_[i] = std::min(headways_[i - 1], headways_[i - 2]);
            }
        }
    }


}// namespace fb
