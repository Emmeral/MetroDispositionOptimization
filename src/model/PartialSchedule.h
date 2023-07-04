
#pragma once


#include "model/main/Model.h"
#include <vector>
namespace fb {

    struct PartialSchedule {

        explicit PartialSchedule(size_t course_count) : schedule(course_count, std::vector<ScheduleItem>{}){};

        std::vector<std::vector<ScheduleItem>> schedule{};

        seconds_t end_of_validity{invalid<seconds_t>()};

        seconds_t dutyEnd(RSDuty const &duty, std::vector<Course> const &courses) const;
        seconds_t firstUncompletedEnd(std::vector<RSDuty> const &duties, std::vector<Course> const &courses) const;
        seconds_t lastEnd(std::vector<RSDuty> const &duties, std::vector<Course> const &courses) const;

        void pruneToSnapshotTime(seconds_t time);
    };


}// namespace fb
