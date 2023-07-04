
#include "TrackHeadwayBuilder.h"
#include "model/graph/abstractions/TrainAtStationAbstraction.h"
#include <deque>

namespace fb {
    HeadwayBuildResult TrackHeadwayBuilder::build(const graph::TrainAtStationAbstraction &abs1,
                                                  const graph::TrainAtStationAbstraction &abs2) {


        bool from_first = true;
        bool from_second = true;

        auto const first_realized_arr = abs1.getRealizedArrival();
        auto const second_realized_arr = abs2.getRealizedArrival();

        // nothing to do if both arrivals have already happened -> e.g. tracks are blocked
        if (isValid(first_realized_arr) && isValid(second_realized_arr)) {
            return HeadwayBuildResult{};
        }

        // if one side is already realized we don't have incoming headway edges
        if (isValid(first_realized_arr)) {
            from_second = false;
        } else if (isValid(second_realized_arr)) {
            from_first = false;
        }

        seconds_t first_arrival = no_constrain_distances_[abs1.getArrivalTimeOutNode()];
        seconds_t second_arrival = no_constrain_distances_[abs2.getArrivalTimeOutNode()];

        seconds_t first_departure = no_constrain_distances_[abs1.getDepartureTimeOutNode()];
        seconds_t second_departure = no_constrain_distances_[abs2.getDepartureTimeOutNode()];

        auto diff = 0;
        if (first_arrival < second_arrival) {
            diff = ((long) second_arrival) - first_arrival;
        } else {
            diff = ((long) first_arrival) - second_arrival;
        }

        if (diff > options_.headway_choice_max_difference) {
            if (first_arrival > second_arrival && !isValid(first_realized_arr)) {
                from_first = false;
            } else if (!isValid(second_realized_arr)) {
                from_second = false;
            }
        }
        assert(from_first || from_second);


        auto choice_index = invalid<graph::choice_index_t>();


        auto const &tracks = abs1.getNode().tracks.all_tracks;

        bool overlap = false;
        for (track_id_t id = 0; id < tracks.size(); ++id) {
            if (abs1.considersTrack(id) && abs2.considersTrack(id)) {
                overlap = true;
                // only create choice if we found an intersection and have edges in both directions
                if (isInvalid(choice_index) && from_first && from_second) {
                    choice_index = createChoice(abs1, abs2, diff);
                }
                graph::FullAlternativeIndex first{choice_index, graph::AlternativeIndex::FIRST};
                graph::FullAlternativeIndex second{choice_index, graph::AlternativeIndex::SECOND};

                if (from_first) {
                    createHeadwayEdges(abs1, abs2, id, first);
                }
                if (from_second) {
                    createHeadwayEdges(abs2, abs1, id, second);
                }
            }
        }

        if (!overlap) {
            return HeadwayBuildResult{};
        }

        graph::FullAlternativeIndex first{choice_index, graph::AlternativeIndex::FIRST};
        graph::FullAlternativeIndex second{choice_index, graph::AlternativeIndex::SECOND};
        // add an edge, that decided which course is first at the node
        if (isValid(choice_index)) {
            graph_.createAlternativeEdge(abs1.getArrivalTimeOutNode(), abs2.getArrivalTimeInNode(), first, 0);
            graph_.createAlternativeEdge(abs2.getArrivalTimeOutNode(), abs1.getArrivalTimeInNode(), second, 0);
        } else if (!(from_first && from_second)) {
            if (from_first) {
                graph_.createFixedEdge(abs1.getArrivalTimeOutNode(), abs2.getArrivalTimeInNode(), 0);
            } else if (from_second) {
                graph_.createFixedEdge(abs2.getArrivalTimeOutNode(), abs1.getArrivalTimeInNode(), 0);
            }
        }


        if (isValid(choice_index)) {
            return HeadwayBuildResult(choice_index);
        } else if (from_first) {
            return HeadwayBuildResult(graph::AlternativeIndex::FIRST);
        } else {
            return HeadwayBuildResult(graph::AlternativeIndex::SECOND);
        }
    }
    graph::choice_index_t TrackHeadwayBuilder::createChoice(const graph::TrainAtStationAbstraction &abs1,
                                                            const graph::TrainAtStationAbstraction &abs2, int diff) {


        seconds_t first_arrival = abs1.getOriginalArrival();
        seconds_t second_arrival = abs2.getOriginalArrival();

        graph::AlternativeIndex original;
        auto const *prior_1 = abstractions_.prior(abs1);
        auto const *prior_2 = abstractions_.prior(abs2);

        if (prior_1 != nullptr && prior_2 != nullptr && prior_1->getNode().index == prior_2->getNode().index) {
            // use the departure at the last node to verify the original order
            // because sometimes the schedule contains some weird overtaking at lines
            original = static_cast<graph::AlternativeIndex>(prior_2->getOriginalDeparture() <
                                                            prior_1->getOriginalDeparture());
        } else {
            original = static_cast<graph::AlternativeIndex>(second_arrival < first_arrival);
        }
        auto const node_id = abs1.getNode().index;

        auto choice_index = graph_.createChoice<graph::ChoiceType::TRACK_HEADWAY>(
                original, {node_id, abs1.getIndex(), abs2.getIndex()});


        bool isLink = abs1.isLink() || abs2.isLink();
        if (diff < options_.track_relation_max_difference && (!options_.track_relation_only_for_link || isLink)) {
            // relate track choice to the track headway choice
            if (abs1.hasTrackChoice()) {
                for (auto const tc: *abs1.getTrackChoices()) {
                    graph_.choices[tc].relations.emplace_back(choice_index);
                    graph_.choices[choice_index].relations.emplace_back(tc);
                }
            }
            if (abs2.hasTrackChoice()) {
                for (auto const tc: *abs2.getTrackChoices()) {
                    graph_.choices[tc].relations.emplace_back(choice_index);
                    graph_.choices[choice_index].relations.emplace_back(tc);
                }
            }
        }


        return choice_index;
    }
    void TrackHeadwayBuilder::createHeadwayEdges(const graph::TrainAtStationAbstraction &abs1,
                                                 const graph::TrainAtStationAbstraction &abs2, track_id_t id,
                                                 const graph::FullAlternativeIndex &alternative) {

        auto const &cached_1 = getOrCreateHeadwayNodes(abs1);
        auto const &cached_2 = getOrCreateHeadwayNodes(abs2);

        auto out = cached_1.out_nodes_.at(id);
        auto in = cached_2.in_nodes_.at(id);

        assert(isValid(out));
        assert(isValid(in));

        if (isValid(alternative.choice_index)) {
            graph_.createAlternativeEdge(out, in, alternative, 30);
        } else {
            graph_.createFixedEdge(out, in, 30);
        }
    }

    const TrackHeadwayBuilder::CachedHeadwayNodes &
    TrackHeadwayBuilder::getOrCreateHeadwayNodes(const graph::TrainAtStationAbstraction &tas) {

        auto &cached = cached_nodes_[tas.getIndex()];

        if (!cached.exists) {
            cached.in_nodes_ = node_builder_.createHeadwayNodes<TrackHeadwayNodeBuilder::Type::IN>(tas);
            cached.out_nodes_ = node_builder_.createHeadwayNodes<TrackHeadwayNodeBuilder::Type::OUT>(tas);
            cached.exists = true;
        }

        return cached;
    }


}// namespace fb
