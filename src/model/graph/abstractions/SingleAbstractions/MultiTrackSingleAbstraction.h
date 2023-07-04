
#pragma once

#include "model/graph/AlternativeGraph.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/MultiTrackAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/SingleScheduleItemAbstraction.h"
#include "model/graph/abstractions/abstract/StopPassChoiceAbstraction.h"
namespace fb::graph {

    /**
     * Class describing a train passing through a node on a single course. The course does neither start not end here.
     */
    class MultiTrackSingleAbstraction : public AbstractTrainAtStationAbstraction,
                                        public SingleScheduleItemAbstraction,
                                        public MultiTrackAbstraction,
                                        public StopPassChoiceAbstraction,
                                        public NonRealizedAbstraction {

    public:
        /**
         * @param index the index of this abstraction
         * @param node the node that is abstracted
         * @param course the course of the schedule item
         * @param schedule_item_index the index of the item representing entry and exit
         * @param graph the graph use for creating the nodes
         */
        MultiTrackSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                    index_t schedule_item_index);

        MultiTrackSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                    ScheduleItem const &schedule_item_index, TaggedWeight stop_time);

        void materialize(graph::AlternativeGraph &graph) override;


        node_index_t getArrivalTimeOutNode() const final;
        node_index_t getDepartureTimeOutNode() const final;

        node_index_t getArrivalTimeInNode() const final;
        node_index_t getDepartureTimeInNode() const final;
    };

}// namespace fb::graph
