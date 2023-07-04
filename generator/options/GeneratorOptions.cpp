
#include "GeneratorOptions.h"

namespace fb {
    std::ostream &operator<<(std::ostream &os, const GeneratorOptions &options) {

        os << "GeneratorOptions:" << std::endl;
        os << "model: " << options.model << std::endl;
        os << "output_folder: " << options.output_folder << std::endl;
        os << "count: " << options.count << std::endl;
        os << "seed: " << options.seed << std::endl;
        os << "min_snapshot_time: " << options.min_snapshot_time << std::endl;
        os << "max_snapshot_time: " << options.max_snapshot_time << std::endl;
        os << "max_incident_in_future: " << options.max_incident_in_future << std::endl;
        os << "max_incidents: " << options.max_incidents << std::endl;
        os << "min_incidents: " << options.min_incidents << std::endl;
        os << "max_extended_dwell_time: " << options.max_extended_dwell_time << std::endl;
        os << "min_extended_dwell_time: " << options.min_extended_dwell_time << std::endl;
        os << "max_extended_run_time: " << options.max_extended_run_time << std::endl;
        os << "min_extended_run_time: " << options.min_extended_run_time << std::endl;
        os << "max_course_dwell_time: " << options.max_course_dwell_time << std::endl;
        os << "min_course_dwell_time: " << options.min_course_dwell_time << std::endl;
        os << "max_late_departures: " << options.max_late_departures << std::endl;
        os << "min_late_departures: " << options.min_late_departures << std::endl;
        os << "late_departure_min_delay: " << options.late_departure_min_delay << std::endl;
        os << "late_departure_max_delay: " << options.late_departure_max_delay << std::endl;
        os << "interval_min_time: " << options.interval_min_time << std::endl;
        os << "interval_max_time: " << options.interval_max_time << std::endl;
        os << "course_dwell_min_extend: " << options.course_dwell_min_extend << std::endl;
        os << "course_dwell_max_extend: " << options.course_dwell_max_extend << std::endl;
        os << "extended_dwell_min_time: " << options.extended_dwell_min_time << std::endl;
        os << "extended_dwell_max_time: " << options.extended_dwell_max_time << std::endl;
        os << "extended_runtime_min_factor: " << options.extended_runtime_min_factor << std::endl;
        os << "extended_runtime_max_factor: " << options.extended_runtime_max_factor << std::endl;
        return os;
    }
    template<>
    std::string fb::toJson(const GeneratorOptions &t) {
        std::string str = "{";
        str += "\"model\": " + toJson(t.model) + ",";
        str += "\"output_folder\": " + toJson(t.output_folder) + ",";
        str += "\"count\": " + toJson(t.count) + ",";
        str += "\"seed\": " + toJson(t.seed) + ",";
        str += "\"min_snapshot_time\": " + toJson(t.min_snapshot_time) + ",";
        str += "\"max_snapshot_time\": " + toJson(t.max_snapshot_time) + ",";
        str += "\"max_incident_in_future\": " + toJson(t.max_incident_in_future) + ",";
        str += "\"max_incidents\": " + toJson(t.max_incidents) + ",";
        str += "\"min_incidents\": " + toJson(t.min_incidents) + ",";
        str += "\"max_extended_dwell_time\": " + toJson(t.max_extended_dwell_time) + ",";
        str += "\"min_extended_dwell_time\": " + toJson(t.min_extended_dwell_time) + ",";
        str += "\"max_extended_run_time\": " + toJson(t.max_extended_run_time) + ",";
        str += "\"min_extended_run_time\": " + toJson(t.min_extended_run_time) + ",";
        str += "\"max_course_dwell_time\": " + toJson(t.max_course_dwell_time) + ",";
        str += "\"min_course_dwell_time\": " + toJson(t.min_course_dwell_time) + ",";
        str += "\"max_late_departures\": " + toJson(t.max_late_departures) + ",";
        str += "\"min_late_departures\": " + toJson(t.min_late_departures) + ",";
        str += "\"late_departure_min_delay\": " + toJson(t.late_departure_min_delay) + ",";
        str += "\"late_departure_max_delay\": " + toJson(t.late_departure_max_delay) + ",";
        str += "\"interval_min_time\": " + toJson(t.interval_min_time) + ",";
        str += "\"interval_max_time\": " + toJson(t.interval_max_time) + ",";
        str += "\"course_dwell_min_extend\": " + toJson(t.course_dwell_min_extend) + ",";
        str += "\"course_dwell_max_extend\": " + toJson(t.course_dwell_max_extend) + ",";
        str += "\"extended_dwell_min_time\": " + toJson(t.extended_dwell_min_time) + ",";
        str += "\"extended_dwell_max_time\": " + toJson(t.extended_dwell_max_time) + ",";
        str += "\"extended_runtime_min_factor\": " + toJson(t.extended_runtime_min_factor) + ",";
        str += "\"extended_runtime_max_factor\": " + toJson(t.extended_runtime_max_factor);
        str += "}";
        return str;
    }
}// namespace fb
