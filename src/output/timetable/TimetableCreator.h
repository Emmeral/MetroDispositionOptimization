
#pragma once

#include "TimetableAmendment.h"
#include "algo/path/definitions.h"
#include "model/PartialSchedule.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
namespace fb {

    class TimetableCreator {
    public:
        TimetableCreator(const Model &model, const std::vector<std::vector<ScheduleItem>> &realized_schedule);
        TimetableAmendment createAmendedTimetable(graph::Selection const &selection, const graph::GraphWithMeta &gwm);
        TimetableAmendment createAmendedTimetable(graph::Selection const &selection, const graph::GraphWithMeta &gwm,
                                                  std::vector<distance_t> const &dfs);

        TimetableAmendment createAmendedTimetable(PartialSchedule const &partial_schedule,
                                                  bool addAmendmentsFromRealized = false) const;

        PartialSchedule createPartialSchedule(const graph::Selection &selection, const graph::GraphWithMeta &gwm);
        PartialSchedule createPartialSchedule(const graph::Selection &selection, const graph::GraphWithMeta &gwm,
                                              const std::vector<distance_t> &dfs);


    private:
        Model const &model_;
        std::vector<std::vector<ScheduleItem>> const &realized_schedule_;
        std::vector<RSDuty> createAmendedDuties(const std::vector<Course> &amended_courses) const;
    };

}// namespace fb
