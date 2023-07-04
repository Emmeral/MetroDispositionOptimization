#pragma once


#include "Json.h"
#include "definitions.h"
#include <ostream>
namespace fb {

    struct DetailedPenalty {

        penalty_t total() const { return delay_penalty + skipped_stop_penalty + frequency_penalty; }

        penalty_t delay_penalty;
        penalty_t skipped_stop_penalty;
        penalty_t frequency_penalty;
    };

    std::ostream &operator<<(std::ostream &os, const DetailedPenalty &penalty);

    template<>
    std::string toJson(const DetailedPenalty &t);
}// namespace fb
