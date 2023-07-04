#pragma once


#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/main/Model.h"
#include <fmt/core.h>
#include <sstream>
namespace fb {


    struct ScheduleItemAmendment {

        ScheduleItemAmendment() = default;
        ScheduleItemAmendment(course_index_t course, ScheduleItem const &si);


        course_index_t course{invalid<course_index_t>()};
        index_t schedule_item{invalid<index_t>()};
        node_index_t node{invalid<node_index_t>()};

        virtual std::string toTextLine(Model const &model) const = 0;

    protected:
        std::string baseToTextLine(Model const &model, const std::string &name) const;
    };

    struct ReTiming : ScheduleItemAmendment {

        ReTiming() = default;

        ReTiming(course_index_t course, ScheduleItem const &si, seconds_t new_time);

        seconds_t old_time{invalid<seconds_t>()};
        seconds_t new_time{invalid<seconds_t>()};
    };

    struct ArrivalReTiming final : ReTiming {
        ArrivalReTiming() = default;
        ArrivalReTiming(course_index_t course, const ScheduleItem &si, seconds_t new_time);

        std::string toTextLine(const Model &model) const override;
    };

    struct DepartureReTiming final : ReTiming {
        DepartureReTiming() = default;
        DepartureReTiming(course_index_t course, const ScheduleItem &si, seconds_t new_time);

        std::string toTextLine(const Model &model) const override;
    };


    struct RePlatforming final : ScheduleItemAmendment {

        RePlatforming() = default;
        RePlatforming(course_index_t course, ScheduleItem const &si, track_id_t new_track);

        std::string toTextLine(const Model &model) const override;

        track_id_t old_track{invalid<track_id_t>()};
        track_id_t new_track{invalid<track_id_t>()};
    };

    struct SkippedStop final : ScheduleItemAmendment {
        SkippedStop() = default;
        SkippedStop(course_index_t course, const ScheduleItem &si) : ScheduleItemAmendment(course, si) {}

        std::string toTextLine(const Model &model) const override;
    };


    struct ScheduleAmendment {

        std::vector<std::vector<ScheduleItem>> amended_schedule{};
    };


    struct TimetableAmendment {
        std::vector<Course> courses{};
        std::vector<RSDuty> duties{};

        std::vector<ArrivalReTiming> arrival_retimings{};
        std::vector<DepartureReTiming> departure_retimings{};
        std::vector<RePlatforming> replatformings{};
        std::vector<SkippedStop> skipped_stops{};
    };

}// namespace fb
