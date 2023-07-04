
#pragma once


#include "Json.h"
#include "model/definitions.h"
#include "model/problems/ProblemInstance.h"
#include <cstddef>

namespace fb {
    struct ProblemStatistics {

        ProblemStatistics() = default;
        explicit ProblemStatistics(ProblemInstance const &problem, long parse_time);

        long parse_time{0};

        seconds_t snapshot_time{0};
        seconds_t last_incident{0};

        size_t esd_count{0};
        size_t ert_count{0};
        size_t ld_count{0};
        size_t etd_count{0};
    };

    std::ostream &operator<<(std::ostream &os, ProblemStatistics const &ps);

    template<>
    std::string toJson(ProblemStatistics const &ps);
}// namespace fb
