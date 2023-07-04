
#include "StaticImplicationBuilder.h"

namespace fb {

    StaticImplicationBuilder::StaticImplicationBuilder(Model const &model, graph::AlternativeGraph &graph,
                                                       const graph::Abstractions &abstractions,
                                                       HeadwayCollection const &headways,
                                                       GraphBuilderOptions const &options)
        : model_(model), graph_(graph), abstractions_(abstractions), headways_(headways), options_(options) {}


    void StaticImplicationBuilder::addStaticImplicationsAtNodeInDirection(node_index_t node_id, Direction dir) {

        auto &dir_mapped = headways_.station_headways[node_id];

        for (auto &[k, station_choice_index]: dir_mapped.ofDir(dir)) {

            graph::TrainAtStationAbstraction const &abs1 = abstractions_[k.first];
            graph::TrainAtStationAbstraction const &abs2 = abstractions_[k.second];
            addImplicationToPriorLine(node_id, station_choice_index, abs1, abs2);

            // implication in other direction only if overtaking is forbidden / not possible
            bool no_overtake = abs1.consideredTrackCount() == 1 && abs2.consideredTrackCount() == 1 &&
                               abs1.consideredTracks()[0] == abs2.consideredTracks()[0];
            if (no_overtake) {
                addImplicationToNextLine(node_id, station_choice_index, abs1, abs2);
            }
        }
    }
    void StaticImplicationBuilder::addImplicationToNextLine(node_index_t node_id,
                                                            graph::choice_index_t station_choice_index,
                                                            const graph::TrainAtStationAbstraction &abs1,
                                                            const graph::TrainAtStationAbstraction &abs2) {
        auto *next1 = abstractions_.next(abs1);
        auto *next2 = abstractions_.next(abs2);
        if (next1 == nullptr || next2 == nullptr) {
            return;
        }
        if (next1->getNode().index != next2->getNode().index) {
            return;
        }

        auto next_node_index = next1->getNode().index;
        auto link = model_.getLinkFromTo(node_id, next_node_index);

        if (options_.build_static_implications_in_same_direction) {
            std::map<abs_pair, graph::choice_index_t> const &map = headways_.line_headways[link->id];
            auto it = map.find({abs1.getIndex(), abs2.getIndex()});
            if (it != map.end()) {
                graph::choice_index_t line_choice_index = it->second;


                for (auto ai: {graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND}) {
                    graph_.choices[station_choice_index].alternative(ai).static_implications.push_back(
                            {line_choice_index, ai});
                    graph_.choices[line_choice_index].alternative(ai).static_implications.push_back(
                            {station_choice_index, ai});
                }
            }
        }

        if (options_.build_trivial_implications) {
            // check for fixed headways
            std::map<abs_pair, graph::AlternativeIndex> const &fixed_map = headways_.fixed_line_headways_[link->id];
            auto fixed_it = fixed_map.find({abs1.getIndex(), abs2.getIndex()});
            if (fixed_it == fixed_map.end()) {
                return;
            }
            graph::AlternativeIndex fixed = fixed_it->second;
            graph_.addTrivialImplication({station_choice_index, fixed});
        }
    }
    void StaticImplicationBuilder::addImplicationToPriorLine(node_index_t node_id,
                                                             graph::choice_index_t station_choice_index,
                                                             const graph::TrainAtStationAbstraction &abs1,
                                                             const graph::TrainAtStationAbstraction &abs2) {
        // should imply prior line headway and vice versa
        auto *prior1 = abstractions_.prior(abs1);
        auto *prior2 = abstractions_.prior(abs2);
        if (prior1 == nullptr || prior2 == nullptr) {
            return;
        }
        if (prior1->getNode().index != prior2->getNode().index) {
            return;
        }


        auto prior_node_index = prior1->getNode().index;

        auto link = model_.getLinkFromTo(prior_node_index, node_id);

        if (options_.build_static_implications_in_same_direction) {
            std::map<abs_pair, graph::choice_index_t> const &map = headways_.line_headways[link->id];
            auto it = map.find({prior1->getIndex(), prior2->getIndex()});

            if (it != map.end()) {
                graph::choice_index_t line_choice_index = it->second;


                for (auto ai: {graph::AlternativeIndex::FIRST, graph::AlternativeIndex::SECOND}) {
                    graph_.choices[station_choice_index].alternative(ai).static_implications.push_back(
                            {line_choice_index, ai});
                    graph_.choices[line_choice_index].alternative(ai).static_implications.push_back(
                            {station_choice_index, ai});
                }
                return;
            }
        }

        if (options_.build_trivial_implications) {
            // check for "fixed" headways
            std::map<abs_pair, graph::AlternativeIndex> const &fixed_map = headways_.fixed_line_headways_[link->id];
            auto fixed_it = fixed_map.find({prior1->getIndex(), prior2->getIndex()});
            if (fixed_it == fixed_map.end()) {
                return;
            }
            graph::AlternativeIndex fixed = fixed_it->second;

            graph_.addTrivialImplication({station_choice_index, fixed});
        }
    }
    void StaticImplicationBuilder::addMixedDirStaticImplications(node_index_t node_id) {
        auto mixed = headways_.station_headways[node_id].mixed;

        for (auto &[k, station_choice_index]: mixed) {

            graph::TrainAtStationAbstraction const &abs1 = abstractions_[k.first];
            graph::TrainAtStationAbstraction const &abs2 = abstractions_[k.second];


            auto const *prior_1 = &abs1;
            auto const *next_2 = &abs2;

            bool has_both_tracks = false;
            bool at_same_node = true;

            while (!has_both_tracks) {

                prior_1 = abstractions_.prior(*prior_1);
                next_2 = abstractions_.next(*next_2);

                if (prior_1 == nullptr || next_2 == nullptr) {
                    at_same_node = false;
                    break;
                }
                auto const &node1 = prior_1->getNode();
                auto const &node2 = next_2->getNode();
                if (node1.index != node2.index) {
                    at_same_node = false;
                    break;
                }


                // check for intersection of available tracks
                auto const &track_at_1 = prior_1->consideredTracks();
                auto const &track_at_2 = next_2->consideredTracks();
                for (auto const &t: track_at_1) {
                    if (std::find(track_at_2.begin(), track_at_2.end(), t) != track_at_2.end()) {
                        has_both_tracks = true;
                        break;
                    }
                }
            }
            if (!at_same_node) {
                continue;
            }


            auto const &p_node = prior_1->getNode();

            std::map<abs_pair, graph::choice_index_t> const &map = headways_.station_headways[p_node.index].mixed;
            auto it = map.find({prior_1->getIndex(), next_2->getIndex()});
            if (options_.build_static_implications_in_alternating_directions && it != map.end()) {

                graph::choice_index_t implied_choice = it->second;

                graph_.choices[station_choice_index]
                        .alternative(graph::AlternativeIndex::FIRST)
                        .static_implications.push_back({implied_choice, graph::AlternativeIndex::FIRST});
                graph_.choices[implied_choice]
                        .alternative(graph::AlternativeIndex::SECOND)
                        .static_implications.push_back({station_choice_index, graph::AlternativeIndex::SECOND});

            } else if (options_.build_trivial_implications) {
                // check for fixed headways
                std::map<abs_pair, graph::AlternativeIndex> const &fixed_map =
                        headways_.fixed_station_headways[p_node.index].mixed;
                auto fixed_it = fixed_map.find({prior_1->getIndex(), next_2->getIndex()});
                if (fixed_it == fixed_map.end()) {
                    continue;
                }
                graph::AlternativeIndex fixed = fixed_it->second;

                if (fixed == graph::AlternativeIndex::SECOND) {
                    graph_.addTrivialImplication({station_choice_index, graph::AlternativeIndex::SECOND});
                }
            }
        }
    }


}// namespace fb
