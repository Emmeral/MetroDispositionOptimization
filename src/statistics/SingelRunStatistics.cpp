
#include "SingleRunStatistics.h"

namespace fb {


    std::ostream &operator<<(std::ostream &os, const SingleRunStatistics &stats) {

        os << "Total time: " << stats.total_time << "\n";
        os << "Completed with planned: " << stats.completed_with_planned << "\n";
        os << stats.problem_statistics << "\n";


        for (auto i = 0; i < stats.time_per_iteration.size(); i++) {
            os << "Iteration " << i + 1 << " time: " << stats.time_per_iteration[i] << "\n";
            os << stats.graph_statistics[i] << "\n";
            os << stats.bab_statistics[i] << "\n";
            os << "Snapshot time: " << stats.snapshot_times[i] << "\n";
            os << "\n";
        }

        return os;
    }
    template<>
    std::string fb::toJson(const SingleRunStatistics &t) {
        std::string s = "{";
        s += "\"total_time\":" + toJson(t.total_time) + ",";
        s += "\"problem_statistics\":" + toJson(t.problem_statistics) + ",";
        s += "\"time_per_iteration\":" + toJson(t.time_per_iteration) + ",";
        s += "\"bab_statistics\":" + toJson(t.bab_statistics) + ",";
        s += "\"graph_statistics\":" + toJson(t.graph_statistics) + ",";
        s += "\"snapshot_times\":" + toJson(t.snapshot_times) + ",";
        s += "\"completed_with_planned\":" + toJson(t.completed_with_planned);
        s += "}";
        return s;
    }
}// namespace fb
