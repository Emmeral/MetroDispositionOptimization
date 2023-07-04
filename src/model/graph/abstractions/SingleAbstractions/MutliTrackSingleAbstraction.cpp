
#include "MultiTrackSingleAbstraction.h"
#include "fmt/format.h"

namespace fb::graph {
    MultiTrackSingleAbstraction::MultiTrackSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                                             index_t schedule_item_index)
        : SingleScheduleItemAbstraction(course, schedule_item_index),
          AbstractTrainAtStationAbstraction(index, node,
                                            TaggedWeight::fixed(course.schedule[schedule_item_index].departure -
                                                                course.schedule[schedule_item_index].arrival)) {}

    MultiTrackSingleAbstraction::MultiTrackSingleAbstraction(index_t index, const fb::Node &node, const Course &course,
                                                             const ScheduleItem &schedule_item_index,
                                                             TaggedWeight stop_time)
        : SingleScheduleItemAbstraction(course, schedule_item_index),
          AbstractTrainAtStationAbstraction(index, node, stop_time) {}


    void MultiTrackSingleAbstraction::materialize(AlternativeGraph &graph) {
        offset_ = graph.nodes.size();


        auto station = graph.createNode();
        auto line = graph.createNode();

        createStopPassChoice(graph);

        graph.createFixedEdge(station, line, 0);
        graph.createStopEdge(station, line, stop_pass_choice_, stop_time_);
        graph.createPassEdge(station, line, stop_pass_choice_);

        createTrackChoices(graph, getOriginalInScheduleItem()->original_track);
        node_count_ = graph.nodes.size() - offset_;
    }


    node_index_t MultiTrackSingleAbstraction::getArrivalTimeOutNode() const { return offset_; }
    node_index_t MultiTrackSingleAbstraction::getDepartureTimeOutNode() const { return offset_ + 1; }

    node_index_t MultiTrackSingleAbstraction::getArrivalTimeInNode() const { return offset_; }
    node_index_t MultiTrackSingleAbstraction::getDepartureTimeInNode() const { return offset_ + 1; }


}// namespace fb::graph
