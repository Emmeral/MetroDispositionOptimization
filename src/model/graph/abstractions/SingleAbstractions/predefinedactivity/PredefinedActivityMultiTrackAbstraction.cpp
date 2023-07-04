
#include "PredefinedActivityMultiTrackAbstraction.h"

namespace fb::graph {


    void PredefinedActivityMultiTrackAbstraction::materialize(AlternativeGraph &graph) {

        offset_ = graph.nodes.size();

        auto station = graph.createNode();// 0
        auto line = graph.createNode();   // 1

        auto dwell_time = TaggedWeight::fixed(0);
        if (predefined_activity_ == Activity::STOP) {
            dwell_time = stop_time_;
        }
        graph.createFixedEdge(station, line, dwell_time);

        createTrackChoices(graph, getOriginalInScheduleItem()->original_track);

        node_count_ = graph.nodes.size() - offset_;
    }

}// namespace fb::graph
