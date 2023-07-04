
#pragma once

#include "Json.h"
#include "model/definitions.h"
namespace fb {


    struct GraphBuilderOptions {

        static const seconds_t DEFAULT_PREMATURE_DEPARTURE = 60 * 5;

        seconds_t extend{std::numeric_limits<seconds_t>::max()};

        seconds_t max_premature_departure{DEFAULT_PREMATURE_DEPARTURE};

        bool build_static_implications_in_same_direction{true};
        bool build_static_implications_in_alternating_directions{true};
        bool build_static_implications_duty_consistency{true};
        bool build_trivial_implications{true};


        seconds_t headway_choice_max_difference{std::numeric_limits<seconds_t>::max()};
        seconds_t stop_pass_choice_after_last_incident{std::numeric_limits<seconds_t>::max()};
        seconds_t track_choice_after_last_incident{std::numeric_limits<seconds_t>::max()};

        bool stop_pass_choices{true};
        bool track_choices{true};

        seconds_t line_arrival_headway{0};

        seconds_t track_relation_max_difference{0};
        bool track_relation_only_for_link{false};
    };

    std::ostream &operator<<(std::ostream &stm, GraphBuilderOptions const &opt);

    template<>
    std::string toJson(GraphBuilderOptions const &t);

}// namespace fb
