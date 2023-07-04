
#pragma once

#include "model/main/Model.h"
namespace fb {

    struct ExtendedRunTime {
        seconds_t start;
        seconds_t end;
        link_index_t link;
        seconds_t extended_runtime;
    };

    struct LateDeparture {
        course_index_t course;
        seconds_t departure_delay;
    };

    struct ExtendedDwellTime {
        seconds_t start;
        seconds_t end;
        node_index_t node;
        seconds_t extended_dwell;
    };

    struct TrainExtendedDwell {
        course_index_t course;
        node_index_t node;
        seconds_t extended_dwell;
    };


    struct ProblemInstance {
        seconds_t snapshot_time{0};
        seconds_t last_incident_end{0};
        // indexed by course id
        std::vector<std::vector<ScheduleItem>> realized_schedule{};

        std::vector<ExtendedRunTime> extended_runtimes{};
        std::vector<LateDeparture> late_departures{};
        std::vector<ExtendedDwellTime> extended_dwelltimes{};
        std::vector<TrainExtendedDwell> extended_train_dwelltimes{};

        std::map<link_index_t, std::vector<ExtendedRunTime *>> mapped_extended_runtimes{};
        std::map<course_index_t, LateDeparture *> mapped_late_departures{};
        std::map<node_index_t, std::vector<ExtendedDwellTime *>> mapped_extended_dwelltimes{};
        std::map<std::pair<course_index_t, node_index_t>, TrainExtendedDwell *> mapped_extended_train_dwelltimes{};


        ScheduleItem const *realized(course_index_t c, index_t si) const;
        bool hasRealizedDeparture(course_index_t c, index_t si, Model const &model) const;
        bool hasRealizedArrival(course_index_t c, index_t si, Model const &model) const;

        inline bool hasRealizedDeparture(Course const &c, fb::ScheduleItem const &si, Model const &model) const {
            return hasRealizedDeparture(c.index, si.index, model);
        };
        inline bool hasRealizedArrival(Course const &c, fb::ScheduleItem const &si, Model const &model) const {
            return hasRealizedArrival(c.index, si.index, model);
        };
    };


}// namespace fb
