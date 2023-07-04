
#include "ModelStatistics.h"

namespace fb {
    ModelStatistics::ModelStatistics(Model const &model, long parse_time) {

        this->parse_time = parse_time;

        node_count = model.nodes.size();
        link_count = model.links.size();
        course_count = model.courses.size();
        duty_count = model.duties.size();


        for (auto const &c: model.courses) {
            schedule_size += c.schedule.size();
        }
    }

    std::ostream &operator<<(std::ostream &os, const ModelStatistics &ms) {
        os << "Model Statistics:\n";
        os << "  Parse Time: " << ms.parse_time << "\n";
        os << "  Node Count: " << ms.node_count << "\n";
        os << "  Link Count: " << ms.link_count << "\n";
        os << "  Course Count: " << ms.course_count << "\n";
        os << "  Duty Count: " << ms.duty_count << "\n";
        return os;
    }
    template<>
    std::string toJson(const ModelStatistics &t) {
        std::string str = "{";
        str += "\"parse_time\":" + toJson(t.parse_time) + ",";
        str += "\"node_count\":" + toJson(t.node_count) + ",";
        str += "\"link_count\":" + toJson(t.link_count) + ",";
        str += "\"course_count\":" + toJson(t.course_count) + ",";
        str += "\"duty_count\":" + toJson(t.duty_count) + ",";
        str += "\"schedule_size\":" + toJson(t.schedule_size);
        str += "}";
        return str;
    }


}// namespace fb
