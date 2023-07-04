
#include "GraphBuilderOptions.h"
#include <iostream>

namespace fb {
    std::ostream &operator<<(std::ostream &stm, const GraphBuilderOptions &opt) {
        stm << "GraphBuilderOptions:\n";
        stm << "  -Extend                        : " << opt.extend << "\n";
        stm << "  -Static Impl. in same Direction: " << opt.build_static_implications_in_same_direction << "\n";
        stm << "  -Static Impl. in alt. Direction: " << opt.build_static_implications_in_alternating_directions << "\n";
        stm << "  -Max Premature Departure       : " << opt.max_premature_departure << "\n";
        stm << "  -Stop Pass after last incident : " << opt.stop_pass_choice_after_last_incident << "\n";
        stm << "  -Track after last incident     : " << opt.track_choice_after_last_incident << "\n";
        stm << "  -Headway Choice Max difference : " << opt.headway_choice_max_difference << "\n";
        stm << "  -Track Relation Max difference : " << opt.track_relation_max_difference << "\n";
        stm << "  -Track Relation only for links : " << opt.track_relation_only_for_link << "\n";
        stm << "  -Enable Stop Pass Choices      : " << opt.stop_pass_choices << "\n";
        stm << "  -Enable Track Choices          : " << opt.track_choices << "\n";
        stm << "  -Line Arrival Headway          : " << opt.line_arrival_headway << "\n";

        return stm;
    }
    template<>
    std::string toJson(const GraphBuilderOptions &t) {
        std::string str = "{";
        str += "\"extend\":" + toJson(t.extend) + ",";
        str += "\"build_static_implications_in_same_direction\":" +
               toJson(t.build_static_implications_in_same_direction) + ",";
        str += "\"build_static_implications_in_alternating_directions\":" +
               toJson(t.build_static_implications_in_alternating_directions) + ",";
        str += "\"max_premature_departure\":" + toJson(t.max_premature_departure) + ",";
        str += "\"stop_pass_choice_after_last_incident\":" + toJson(t.stop_pass_choice_after_last_incident) + ",";
        str += "\"track_choice_after_last_incident\":" + toJson(t.track_choice_after_last_incident) + ",";
        str += "\"headway_choice_max_difference\":" + toJson(t.headway_choice_max_difference) + ",";
        str += "\"track_relation_max_difference\":" + toJson(t.track_relation_max_difference) + ",";
        str += "\"track_relation_only_for_link\":" + toJson(t.track_relation_only_for_link) + ",";
        str += "\"stop_pass_choices\":" + toJson(t.stop_pass_choices) + ",";
        str += "\"track_choices\":" + toJson(t.track_choices) + ",";
        str += "\"line_arrival_headway\":" + toJson(t.line_arrival_headway);
        str += "}";
        return str;
    }
}// namespace fb
