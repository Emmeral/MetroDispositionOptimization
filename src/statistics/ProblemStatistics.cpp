
#include "ProblemStatistics.h"
namespace fb {
    ProblemStatistics::ProblemStatistics(const ProblemInstance &problem, long parse_time) {

        this->parse_time = parse_time;

        snapshot_time = problem.snapshot_time;
        last_incident = problem.last_incident_end;
        esd_count = problem.extended_dwelltimes.size();
        ert_count = problem.extended_runtimes.size();
        ld_count = problem.late_departures.size();
        etd_count = problem.extended_train_dwelltimes.size();
    }

    std::ostream &operator<<(std::ostream &os, const ProblemStatistics &ps) {
        os << "Problem Statistics \n";
        os << "  Parse Time   : " << ps.parse_time << "\n";
        os << "  Snapshot Time: " << readable(ps.snapshot_time) << " (" << ps.snapshot_time << ")\n";
        os << "  Last Incident: " << readable(ps.last_incident) << " (" << ps.last_incident << ")\n";
        os << "  ESD Count    : " << ps.esd_count << "\n";
        os << "  ERT Count    : " << ps.ert_count << "\n";
        os << "  LD Count     : " << ps.ld_count << "\n";
        os << "  ETD Count    : " << ps.etd_count << "\n";
        return os;
    }
    template<>
    std::string toJson(const ProblemStatistics &ps) {
        std::string str = "{";
        str += "\"parse_time\":" + toJson(ps.parse_time) + ",";
        str += "\"snapshot_time\":" + toJson(ps.snapshot_time) + ",";
        str += "\"last_incident\":" + toJson(ps.last_incident) + ",";
        str += "\"esd_count\":" + toJson(ps.esd_count) + ",";
        str += "\"ert_count\":" + toJson(ps.ert_count) + ",";
        str += "\"ld_count\":" + toJson(ps.ld_count) + ",";
        str += "\"etd_count\":" + toJson(ps.etd_count);
        str += "}";
        return str;
    }
}// namespace fb
