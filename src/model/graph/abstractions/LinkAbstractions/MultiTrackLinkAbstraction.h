
#pragma once

#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include "model/graph/abstractions/abstract/CourseLinkAbstraction.h"
#include "model/graph/abstractions/abstract/MultiTrackAbstraction.h"
#include "model/graph/abstractions/abstract/NonRealizedAbstraction.h"
#include "model/graph/abstractions/abstract/PredefinedActivityAbstraction.h"
namespace fb::graph {

    class MultiTrackLinkAbstraction : public AbstractTrainAtStationAbstraction,
                                      public CourseLinkAbstraction,
                                      public PredefinedActivityAbstraction,
                                      public MultiTrackAbstraction,
                                      public NonRealizedAbstraction {

    public:
        MultiTrackLinkAbstraction(index_t index, const fb::Node &node, const Course &in_course,
                                  const Course &out_course, TaggedWeight change_end_time);
        MultiTrackLinkAbstraction(index_t index, const fb::Node &node, const Course &in_course,
                                  const Course &out_course, seconds_t change_end_time);

        void materialize(AlternativeGraph &graph) override;

        node_index_t getArrivalTimeOutNode() const override { return offset_; }
        node_index_t getDepartureTimeOutNode() const override { return offset_ + 1; }
        node_index_t getArrivalTimeInNode() const override { return offset_; }
        node_index_t getDepartureTimeInNode() const override { return offset_ + 1; }
    };

}// namespace fb::graph
