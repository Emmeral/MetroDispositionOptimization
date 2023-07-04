
#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/MultiTrackAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/PredefinedActivityAbstraction.h"
#include "model/graph/abstractions/abstract/SingleScheduleItemAbstraction.h"

namespace fb::graph {

    class PredefinedActivityMultiTrackAbstraction : public AbstractTrainAtStationAbstraction,
                                                    public SingleScheduleItemAbstraction,
                                                    public MultiTrackAbstraction,
                                                    public PredefinedActivityAbstraction,
                                                    public NonRealizedAbstraction {

    public:
        PredefinedActivityMultiTrackAbstraction(const index_t index, const fb::Node &node, const Course &course,
                                                const ScheduleItem &schedule_item,
                                                const TaggedWeight &stop_time = TaggedWeight::fixed(0))
            : AbstractTrainAtStationAbstraction(index, node, stop_time),
              SingleScheduleItemAbstraction(course, schedule_item),
              PredefinedActivityAbstraction(schedule_item.original_activity) {}

        void materialize(AlternativeGraph &graph) override;

        node_index_t getArrivalTimeOutNode() const final { return offset_; }
        node_index_t getDepartureTimeOutNode() const final { return offset_ + 1; }
        node_index_t getArrivalTimeInNode() const final { return offset_; }
        node_index_t getDepartureTimeInNode() const final { return offset_ + 1; }
    };


}// namespace fb::graph
