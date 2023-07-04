#pragma once

#include "PredefinedActivityAbstraction.h"
#include "SingleTrackAbstraction.h"
#include "model/graph/abstractions/AbstractTrainAtStationAbstraction.h"

namespace fb::graph {


    class FullyDecidedAbstraction : public AbstractTrainAtStationAbstraction,
                                    public SingleTrackAbstraction,
                                    public PredefinedActivityAbstraction {

    public:
        FullyDecidedAbstraction(const index_t index, const fb::Node &node, const TaggedWeight &stop_time,
                                track_id_t track_id, Activity predefined_activity)
            : AbstractTrainAtStationAbstraction(index, node, stop_time), SingleTrackAbstraction(track_id),
              PredefinedActivityAbstraction(predefined_activity) {}

        FullyDecidedAbstraction(const index_t index, const fb::Node &node, const TaggedWeight &stop_time,
                                ScheduleItem const &reference)
            : AbstractTrainAtStationAbstraction(index, node, stop_time),
              SingleTrackAbstraction(reference.original_track),
              PredefinedActivityAbstraction(reference.original_activity) {}

        void materialize(AlternativeGraph &graph) override {
            offset_ = graph.nodes.size();

            auto station = graph.createNode();// 0
            auto line = graph.createNode();   // 1

            auto dwell_time = TaggedWeight::fixed(0);
            if (predefined_activity_ == Activity::STOP) {
                dwell_time = stop_time_;
            }
            graph.createFixedEdge(station, line, dwell_time);

            node_count_ = graph.nodes.size() - offset_;
        }

        node_index_t getArrivalTimeOutNode() const override { return offset_; }
        node_index_t getDepartureTimeOutNode() const override { return offset_ + 1; }
        node_index_t getArrivalTimeInNode() const override { return offset_; }
        node_index_t getDepartureTimeInNode() const override { return offset_ + 1; }
    };
}// namespace fb::graph
