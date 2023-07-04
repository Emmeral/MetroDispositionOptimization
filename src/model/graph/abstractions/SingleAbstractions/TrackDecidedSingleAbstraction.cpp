
#include "TrackDecidedSingleAbstraction.h"

namespace fb::graph {

    TrackDecidedSingleAbstraction::TrackDecidedSingleAbstraction(const index_t index, const fb::Node &node,
                                                                 const Course &course,
                                                                 const ScheduleItem &schedule_item,
                                                                 const TaggedWeight &stop_time)
        : AbstractTrainAtStationAbstraction(index, node, stop_time),
          SingleScheduleItemAbstraction(course, schedule_item), SingleTrackAbstraction(schedule_item.original_track) {}

    void TrackDecidedSingleAbstraction::materialize(AlternativeGraph &graph) {
        offset_ = graph.nodes.size();

        auto station = graph.createNode();
        auto line = graph.createNode();

        createStopPassChoice(graph);

        graph.createFixedEdge(station, line, 0);
        graph.createStopEdge(station, line, stop_pass_choice_, stop_time_);
        graph.createPassEdge(station, line, stop_pass_choice_);

        node_count_ = graph.nodes.size() - offset_;
    }
    node_index_t TrackDecidedSingleAbstraction::getArrivalTimeOutNode() const { return offset_; }
    node_index_t TrackDecidedSingleAbstraction::getDepartureTimeOutNode() const { return offset_ + 1; }

    node_index_t TrackDecidedSingleAbstraction::getArrivalTimeInNode() const { return offset_; }
    node_index_t TrackDecidedSingleAbstraction::getDepartureTimeInNode() const { return offset_ + 1; }


}// namespace fb::graph
