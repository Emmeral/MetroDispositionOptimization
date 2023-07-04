//
// Created by flobe on 31.03.2023.
//

#include "definitions.h"
#include "fmt/format.h"

namespace fb {

    std::string readable(seconds_t sec) {

        auto day_count = 0;
        while (sec > SECONDS_PER_DAY) {
            sec = sec - SECONDS_PER_DAY;
            ++day_count;
        }

        if (day_count == 0) {
            return fmt::format("{:0>2}:{:0>2}:{:0>2}", sec / 3600, (sec % 3600) / 60, (sec % 60));
        } else {
            return fmt::format("{}d{:0>2}:{:0>2}:{:0>2}", day_count, sec / 3600, (sec % 3600) / 60, (sec % 60));
        }
    }
}// namespace fb