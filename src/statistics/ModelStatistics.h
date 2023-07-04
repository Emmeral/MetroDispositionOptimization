
#pragma once

#include "Json.h"
#include <cstddef>
#include <model/main/Model.h>

namespace fb {
    struct ModelStatistics {

        explicit ModelStatistics() = default;
        explicit ModelStatistics(Model const &model, long parse_time);

        long parse_time{0};

        size_t node_count{0};
        size_t link_count{0};
        size_t course_count{0};
        size_t duty_count{0};

        size_t schedule_size{0};
    };

    std::ostream &operator<<(std::ostream &os, ModelStatistics const &ms);

    template<>
    std::string toJson(ModelStatistics const &t);
}// namespace fb
