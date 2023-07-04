#pragma once

#include "DistanceState.h"
#include "DistanceToEndState.h"
#include "algo/path/definitions.h"
#include "model/graph/definitions.h"
#include "util/LookupTableSet.h"
#include <queue>
#include <set>
#include <vector>
namespace fb {


    struct ChoiceValuePair {
        graph::choice_index_t choice;
        distance_t value;

        int operator<(ChoiceValuePair const &other) const { return value < other.value; }
    };

    struct ChoiceImpactState {
        std::vector<distance_t> choice_distances;
        std::vector<bool> related_by_group;
        std::priority_queue<ChoiceValuePair> choice_queue;
    };


    template<typename State>
        requires std::derived_from<State, ChoiceImpactState> && std::derived_from<State, DistanceState> &&
                 std::derived_from<State, CommonState> &&
                 std::derived_from<State, DistanceToEndState<State::BACKWARDS_DISTANCES_COUNT>>
    class ChoiceImpactStateUpdater : public StateUpdater<State> {

        typedef LookupTableSet<graph::choice_index_t> choice_set_t;

    public:
        ChoiceImpactStateUpdater(graph::Abstractions const &abstractions, Model const &model,
                                 graph::AlternativeGraph const &graph,
                                 std::vector<distance_t> const &empty_selection_distances,
                                 unsigned int skipped_stop_impact_factor = BIGGEST_SFF,
                                 unsigned int relation_impact_factor = 1000)
            : abstractions_(abstractions), model_(model), graph_(graph),
              static_choice_impact_(graph.choices.size(), {0, 0}),
              empty_selection_distances_(empty_selection_distances),
              skipped_stop_impact_factor_(skipped_stop_impact_factor), relation_impact_factor_(relation_impact_factor) {


            for (auto const &c: graph_.choices) {
                auto const &meta = graph_.choice_metadata[c.index];
                if (meta.type == graph::ChoiceType::STOP_PASS) {
                    auto const &sp_meta = meta.data.sp;
                    graph::TrainAtStationAbstraction const &tas = abstractions_[sp_meta.abstraction];
                    auto arr = tas.getOriginalArrival();
                    Course const &course = tas.getInCourse();
                    auto dir = course.direction;
                    auto bsv = model_.base_station_values[sp_meta.node].of(arr, dir);
                    if (course.category == CourseCategory::PASSENGER) {
                        static_choice_impact_[c.index].ofAlt(graph::passAlternative()) = bsv;
                    }
                }
            }
        }

        void initialize(State &state) const override {


            graph::Selection const &selection = state.selection;
            graph::AlternativeGraph const &graph = state.selection.getGraph();

            state.choice_distances.resize(graph.choices.size(), MIN_DISTANCE);
            state.related_by_group.resize(graph.choices.size(), false);

            for (auto const &edge: graph.alternative_edges) {

                graph::FullAlternativeIndex corresponding_alternative = graph.alternative_edges_metadata[edge.index];

                if (selection.decisionMade(corresponding_alternative.choice_index)) {
                    continue;
                }

                int impact = getEdgeImpact(state, edge);

                auto &dis_of_choice = state.choice_distances[corresponding_alternative.choice_index];
                dis_of_choice = std::max(impact, dis_of_choice);
            }

            // add all not made choice to the queue
            for (auto &c: graph.choices) {
                if (!selection.decisionMade(c)) {
                    state.choice_queue.push({c.index, state.choice_distances[c.index]});
                }
            }
        }
        void update(State &state, graph::Selection::DecisionResult decision_result) const override {


            graph::Selection const &selection = state.selection;
            graph::AlternativeGraph const &graph = state.selection.getGraph();

            choice_set_t updated{graph.choices.size()};

            auto &choice_queue = state.choice_queue;
            // first remove the choice from the top that was just selected
            while (!choice_queue.empty() && selection.decisionMade(choice_queue.top().choice)) {
                state.choice_queue.pop();
            }

            collectUpdatesByDecisions(decision_result.causes, updated, state);
            collectUpdatesByEdges(state.unselected_edges_with_updated_source, updated, state);
            collectUpdatesByEdges(state.unselected_edges_with_updated_target, updated, state);


            // insert better impacts -> there will be duplicates in the queue
            for (auto c: updated) {
                state.choice_queue.push({c, state.choice_distances[c]});
            }

            // remove outdated choices lazily
            while (!choice_queue.empty() && selection.decisionMade(state.choice_queue.top().choice)) {
                state.choice_queue.pop();
            }
        }
        void collectUpdatesByEdges(std::vector<graph::a_edge_index_t> const &edges, choice_set_t &updated,
                                   State &state) const {

            graph::Selection const &selection = state.selection;
            graph::AlternativeGraph const &graph = state.selection.getGraph();

            for (auto edge_i: edges) {
                auto const &edge_meta = graph.alternative_edges_metadata[edge_i];
                auto choice = graph.alternative_edges_metadata[edge_i].choice_index;

                if (selection.decisionMade(choice)) {
                    continue;
                }

                auto impact = getEdgeImpact(state, graph.alternative_edges[edge_i]);
                impact += getChoiceImpact(state, edge_meta);


                if (state.choice_distances[choice] < impact) {
                    state.choice_distances[choice] = impact;
                    updated.emplace(choice);
                }
            }
        }

        void collectUpdatesByDecisions(std::vector<graph::FullAlternativeIndex> const &decisions, choice_set_t &updated,
                                       State &state) const {

            graph::AlternativeGraph const &graph = state.selection.getGraph();

            for (auto dec: decisions) {
                graph::Choice const &choice = graph.choices[dec.choice_index];
                for (graph::choice_index_t const &rel: choice.relations) {
                    if (!state.related_by_group[rel]) {
                        state.choice_distances[rel] += relation_impact_factor_;
                        state.related_by_group[rel] = true;
                        updated.emplace(rel);
                    }
                }
            }
        }

        int getEdgeImpact(const State &state, const graph::Edge &edge) const {

            graph::AlternativeGraph const &graph = state.selection.getGraph();

            int dfs = state.distance_from_start[edge.from];
            if (dfs == MIN_DISTANCE) {
                return MIN_DISTANCE;// there is no path from start to end
            }

            std::array<DistDestPair, State::BACKWARDS_DISTANCES_COUNT> const &dist_to_ends =
                    state.distances_to_end[edge.to];

            auto sum = 0;
            auto count = 0;
            for (auto i = 0; i < State::BACKWARDS_DISTANCES_COUNT; ++i) {
                if (!dist_to_ends[i].isValid()) {
                    break;
                }
                count++;
                sum += dist_to_ends[i].dist;
            }
            if (count == 0) {
                return MIN_DISTANCE;// there is no path from start to end
            }

            sum += count * (dfs + graph.getEdgeWeight(edge, dfs));
            sum = (sum * DELAY_COST_PER_MINUTE) / 60;

            return sum;
        }

        int getChoiceImpact(const State &state, const graph::FullAlternativeIndex fai) const {
            auto const choice = fai.choice_index;
            auto sum = static_choice_impact_[choice].ofAlt(fai.alternative_index) * skipped_stop_impact_factor_;

            sum += state.related_by_group[choice] * relation_impact_factor_;

            return sum;
        }

    private:
        graph::Abstractions const &abstractions_;
        graph::AlternativeGraph const &graph_;
        std::vector<distance_t> const &empty_selection_distances_;
        Model const &model_;

        unsigned int skipped_stop_impact_factor_;
        unsigned int relation_impact_factor_;

        struct PenaltyByAlt {
            penalty_t first;
            penalty_t second;

            inline penalty_t &ofAlt(graph::AlternativeIndex alt) {
                if (alt == graph::AlternativeIndex::FIRST) {
                    return first;
                } else {
                    return second;
                }
            }
            inline penalty_t const &ofAlt(graph::AlternativeIndex alt) const {
                if (alt == graph::AlternativeIndex::FIRST) {
                    return first;
                } else {
                    return second;
                }
            }
        };

        std::vector<PenaltyByAlt> static_choice_impact_;
    };

}// namespace fb
