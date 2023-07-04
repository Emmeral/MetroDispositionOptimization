
#pragma once

#include "StateEvaluator.h"
#include "algo/SkippedStopsInfo.h"
#include "algo/bab/states/DistanceState.h"
#include "algo/bab/states/SkippedStopPenaltyState.h"
#include "algo/bab/states/State.h"
#include "penalty/DelayPenaltyComputer.h"
#include <set>
namespace fb {

    template<typename State>
        requires std::derived_from<State, CommonState> && std::derived_from<State, DistanceState> &&
                 std::derived_from<State, DistanceToEndState<State::BACKWARDS_DISTANCES_COUNT>> &&
                 std::derived_from<State, SkippedStopPenaltyState>
    class NoPeriodStateEvaluator final : public StateEvaluator<State> {
    public:
        NoPeriodStateEvaluator(graph::GraphWithMeta const &graph, PenaltyComputer const &computer,
                               std::vector<BaseStationValue> const &bsvs)
            : penalty_computer_(computer), graph_with_meta_{graph}, bsvs_{bsvs} {};


        penalty_t boundBeforeDecision(const State &state,
                                      graph::FullAlternativeIndex potential_decision) const override {

            graph::AlternativeGraph const &graph = graph_with_meta_.graph;
            auto const &new_edges = graph.alternative(potential_decision).edges;

            int delay_change = penalty_computer_.delay_computer.computeDelayPenaltyChange(
                    graph, new_edges, state.distance_from_start, state.distances_to_end, *state.distance_to_end_bias);


            int skipped_stop_change = getSkippedStopChange(state, potential_decision, graph);

            return state.lower_bound + delay_change + skipped_stop_change;
        }
        penalty_t boundBeforeUpdate(const State &state, graph::FullAlternativeIndex taken_decision,
                                    const graph::Selection::DecisionResult &decision_result) const override {


            auto const &new_edges = decision_result.new_edges;
            int delay_penalty_change = penalty_computer_.delay_computer.computeDelayPenaltyChange(
                    graph_with_meta_.graph, new_edges, state.distance_from_start, state.distances_to_end,
                    *state.distance_to_end_bias);

            graph::AlternativeGraph const &graph = graph_with_meta_.graph;
            int skipped_stop_change = 0;
            for (auto const &cause: decision_result.causes) {
                // TODO: better handling for multple stops changes -> However, does not happen atm.
                skipped_stop_change = std::max(skipped_stop_change, getSkippedStopChange(state, cause, graph));
            }


            return state.lower_bound + delay_penalty_change + skipped_stop_change;
        }
        penalty_t bound(const State &state) const override {

            if (state.cycle) {
                return std::numeric_limits<penalty_t>::max();
            }

            /*
            if (state.selection.isFull()) {
                // only consider period for total selections, otherwise ignore
                return penalty_computer_.computeTotalPenalty(state.selection, state.distance_from_start);
            }
            */

            return penalty_computer_.delay_computer.computeDelayPenalty(state.distance_from_start) +
                   state.skipped_stops_penalty_bound;
        }

    private:
        PenaltyComputer const &penalty_computer_;
        graph::GraphWithMeta const &graph_with_meta_;

        std::vector<BaseStationValue> const &bsvs_;

        int getSkippedStopChange(const State &state, graph::FullAlternativeIndex const &potential_decision,
                                 const graph::AlternativeGraph &graph) const {

            graph::TaggedChoiceMetadata const &choice_meta = graph.choice_metadata[potential_decision.choice_index];

            if (choice_meta.type != graph::ChoiceType::STOP_PASS) {
                return 0;
            }

            graph::StopPassMetadata const &stop_pass_metadata = choice_meta.data.sp;
            auto const &abs = graph_with_meta_.abstractions[stop_pass_metadata.abstraction];

            if (abs.getInCourse().category == CourseCategory::EMPTY) {
                return 0;
            }

            bsv_t bsv = bsvs_[stop_pass_metadata.node].of(abs.getOriginalArrival(), abs.getInCourse().direction);

            SkippedStopInfo const &stop_info = state.skipped_stops[stop_pass_metadata.course];

            return stop_info.boundAfterDecision(bsv, potential_decision.alternative_index) - stop_info.bound;
        }
    };

}// namespace fb
