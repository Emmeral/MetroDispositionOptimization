
#pragma once

#include "Json.h"
#include "model/definitions.h"
#include <string>
namespace fb {

    struct GeneratorOptions {


        std::string model{};
        std::string output_folder{};
        std::string name{};
        int count{1};

        long seed{invalid<long>()};


        seconds_t min_snapshot_time{1};
        seconds_t max_snapshot_time{invalid<seconds_t>()};

        seconds_t max_incident_in_future{invalid<seconds_t>()};

        int max_incidents{10};
        int min_incidents{1};

        int max_extended_dwell_time{invalid<int>()};
        int min_extended_dwell_time{invalid<int>()};

        int max_extended_run_time{invalid<int>()};
        int min_extended_run_time{invalid<int>()};

        int max_course_dwell_time{invalid<int>()};
        int min_course_dwell_time{invalid<int>()};

        int max_late_departures{invalid<int>()};
        int min_late_departures{invalid<int>()};

        seconds_t late_departure_min_delay{60 * 1};
        seconds_t late_departure_max_delay{60 * 20};

        seconds_t interval_min_time{60 * 10};
        seconds_t interval_max_time{60 * 60};

        seconds_t course_dwell_min_extend{30};
        seconds_t course_dwell_max_extend{60 * 10};

        seconds_t extended_dwell_min_time = 60;
        seconds_t extended_dwell_max_time = 180;

        double extended_runtime_min_factor = 1.5;
        double extended_runtime_max_factor = 3;
    };

    std::ostream &operator<<(std::ostream &os, GeneratorOptions const &options);

    template<>
    std::string toJson(GeneratorOptions const &t);

}// namespace fb
