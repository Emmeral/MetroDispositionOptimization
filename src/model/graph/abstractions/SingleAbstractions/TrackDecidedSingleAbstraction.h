
#pragma once


#include "model/graph/abstractions/AbstractTrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/SingleScheduleItemAbstraction.h"
#include "model/graph/abstractions/abstract/SingleTrackAbstraction.h"
#include "model/graph/abstractions/abstract/StopPassChoiceAbstraction.h"


namespace fb::graph {
    class TrackDecidedSingleAbstraction : public AbstractTrainAtStationAbstraction,
                                          public SingleScheduleItemAbstraction,
                                          public SingleTrackAbstraction,
                                          public StopPassChoiceAbstraction,
                                          public NonRealizedAbstraction {

    public:
        TrackDecidedSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                      const ScheduleItem &schedule_item, const TaggedWeight &stop_time);

        void materialize(graph::AlternativeGraph &graph) override;


        node_index_t getArrivalTimeOutNode() const override;
        node_index_t getDepartureTimeOutNode() const override;

        node_index_t getArrivalTimeInNode() const override;
        node_index_t getDepartureTimeInNode() const override;
    };
}// namespace fb::graph
