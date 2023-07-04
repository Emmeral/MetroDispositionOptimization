
#include "MultiTrackLinkAbstraction.h"

namespace fb::graph {

    MultiTrackLinkAbstraction::MultiTrackLinkAbstraction(const index_t index, const fb::Node &node,
                                                         const Course &in_course, const Course &out_course,
                                                         seconds_t change_end_time)
        : MultiTrackLinkAbstraction(index, node, in_course, out_course, TaggedWeight::fixed(change_end_time)) {}

    MultiTrackLinkAbstraction::MultiTrackLinkAbstraction(const index_t index, const fb::Node &node,
                                                         const Course &in_course, const Course &out_course,
                                                         TaggedWeight change_end_time)
        : AbstractTrainAtStationAbstraction(index, node, change_end_time), CourseLinkAbstraction(in_course, out_course),
          PredefinedActivityAbstraction(Activity::STOP) {}
    void MultiTrackLinkAbstraction::materialize(AlternativeGraph &graph) {
        offset_ = graph.nodes.size();

        auto station = graph.createNode();
        auto line = graph.createNode();

        graph.createFixedEdge(station, line, stop_time_);


        createTrackChoices(graph, getOriginalInScheduleItem()->original_track);

        node_count_ = graph.nodes.size() - offset_;
    }


}// namespace fb::graph
