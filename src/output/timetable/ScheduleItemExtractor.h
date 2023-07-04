
#pragma once

#include "TimetableAmendment.h"
#include "algo/path/definitions.h"
#include "model/graph/Selection.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include <vector>
namespace fb {

    class ScheduleItemExtractor {
    public:
        ScheduleItemExtractor(const graph::Selection &selection, const std::vector<distance_t> &dfs);

        ScheduleItem extractItemWithAmendment(const graph::TrainAtStationAbstraction &tas, Course const &course,
                                              TimetableAmendment &amendment);

        ScheduleItem extractItem(const graph::TrainAtStationAbstraction &tas, Course const &course);

    private:
        track_id_t getUsedTrack(graph::TrainAtStationAbstraction const &tas, const graph::Selection &selection);
        Activity getActivity(graph::TrainAtStationAbstraction const &tas, const graph::Selection &selection);


        graph::Selection const &selection_;
        const std::vector<distance_t> &dfs_;
    };

}// namespace fb
