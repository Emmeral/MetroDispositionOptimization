
#pragma once


#include "StateUpdater.h"
#include "algo/bab/choiceselector/ChoiceSeletor.h"
#include "algo/bab/evaluator/StateEvaluator.h"
#include "algo/bab/statestore/StateStore.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
#include "model/main/Model.h"
#include "penalty/PenaltyComputer.h"
#include "statistics/bab/BranchAndBoundStatistics.h"

#include <cmath>
namespace fb {


    template<typename State, typename StateEvaluator, typename ChoiceSelector, typename StateStore,
             typename StateUpdater>
        requires std::derived_from<State, CommonState> &&
                 std::derived_from<StateEvaluator, fb::StateEvaluator<State>> &&
                 std::derived_from<StateStore, fb::StateStore<State>> &&
                 std::derived_from<ChoiceSelector, fb::ChoiceSelector<State>> &&
                 std::derived_from<StateUpdater, fb::StateUpdater<State>>
    class BranchAndBoundAlgorithm {
    public:
        BranchAndBoundAlgorithm(graph::GraphWithMeta const &graph_with_meta, graph::Selection &&upper_bound_selection,
                                StateStore &state_store, ChoiceSelector &choice_selector,
                                StateEvaluator &state_evaluator, StateUpdater &state_updater)
            : graph_with_meta_(graph_with_meta), upper_bound_{std::move(upper_bound_selection)},
              state_store_(&state_store), choice_selector_(&choice_selector), state_evaluator_(&state_evaluator),
              state_updater_(&state_updater), statistics(graph_with_meta.graph) {


            state_store_->useStatistics(statistics);
            if (upper_bound_.selection.isFull()) {
                state_updater_->initialize(upper_bound_);
                upper_bound_value_ = state_evaluator_->bound(upper_bound_);
            } else {
                upper_bound_value_ = std::numeric_limits<decltype(upper_bound_value_)>::max();
            }
            upper_bound_.lower_bound = upper_bound_value_;

            state_store_->registerNewUpperBound(upper_bound_);

            // create initial state by applying trivial implications
            graph::Selection initial_selection{graph_with_meta_.graph};
            graph::Selection::DecisionResult dr{};
            for (auto tc: graph_with_meta_.graph.trivial_implications) {
                dr += initial_selection.makeDecision(tc);
            }

            State initial_state{std::move(initial_selection)};
            state_updater_->initialize(initial_state);

            state_store_->registerState(std::move(initial_state));// initial selection for search
        }


        State &solve() {
            while (doStep())
                ;

            return getBestSolutionFound();
        }
        State &getBestSolutionFound() { return upper_bound_; }
        penalty_t getBestSolutionBound() { return upper_bound_value_; }

        bool doStep() {


            if (state_store_->empty()) {
                return false;
            }
            ++statistics.iterations;

            State &state = state_store_->peekState();
            graph::choice_index_t choice_index = choice_selector_->getBranchChoice(state);

            addToStatistics(state, choice_index);

            auto pen_a1 = state_evaluator_->boundBeforeDecision(state, {choice_index, graph::AlternativeIndex::FIRST});
            auto pen_a2 = state_evaluator_->boundBeforeDecision(state, {choice_index, graph::AlternativeIndex::SECOND});


            if (pen_a1 < upper_bound_value_ && pen_a2 < upper_bound_value_) {
                createTwoDescendants(state, choice_index);
            } else if (pen_a1 < upper_bound_value_ || pen_a2 < upper_bound_value_) {
                createSingleDescendant(state, choice_index, pen_a1, pen_a2);
            } else {
                // perform no decision and delete state from store
                statistics.pruned_before_decision += 2;
                handleDeadEndWithStats(choice_index, state);
            }


            statistics.state_store_max_size = std::max(state_store_->size(), statistics.state_store_max_size);

            return true;
        }


        BABStatistics statistics;

    private:
        void createSingleDescendant(State &state, unsigned int choice_index, penalty_t pen_a1,
                                    penalty_t pen_a2) {// perform single decision
            bool updated = false;
            ++statistics.pruned_before_decision;
            if (pen_a1 < upper_bound_value_) {
                graph::FullAlternativeIndex alt = {choice_index, graph::AlternativeIndex::FIRST};
                updated = performDecision(state, alt);
            } else if (pen_a2 < upper_bound_value_) {
                graph::FullAlternativeIndex alt = {choice_index, graph::AlternativeIndex::SECOND};
                updated = performDecision(state, alt);
            }
            if (updated) {
                state_store_->reRegisterHead();
            } else {
                handleDeadEndWithStats(choice_index, state);
            }
        }
        void createTwoDescendants(State &state, unsigned int choice_index) {

            ++statistics.both_descendant_valid_before_update;
            State copy = state;// first copy state so that the other can be updated

            auto original = graph_with_meta_.graph.choices[choice_index].original;
            auto other = graph::otherAlternative(original);

            bool updated = performDecision(state, {choice_index, original});
            bool updated_copy = performDecision(copy, {choice_index, other});


            if (!updated && !updated_copy) {
                handleDeadEndWithStats(choice_index, state);
                return;
            }

            if (updated) {
                state_store_->reRegisterHead();
            } else {
                state_store_->popState();
            }

            if (updated_copy) {

                // TODO: this is specific for DSF, must be generalized: First explores nodes with lower bound
                if (updated && copy.lower_bound < state_store_->peekState().lower_bound) {
                    std::swap(copy, state_store_->peekState());
                    state_store_->reRegisterHead();
                }

                state_store_->registerState(std::move(copy));
            }
            if (updated && updated_copy) {
                ++statistics.both_descendant_valid_after_update;
            }
        }

        bool performDecision(State &state, graph::FullAlternativeIndex decision) {
            auto decision_result = state.selection.makeDecision(decision);
            return handleDecisionResult(state, decision, decision_result);
        }

        bool handleDecisionResult(State &state, const graph::FullAlternativeIndex &decision,
                                  graph::Selection::DecisionResult const &decision_result) {
            penalty_t pen = state_evaluator_->boundBeforeUpdate(state, decision, decision_result);

            if (pen >= upper_bound_value_) {
                ++statistics.pruned_before_update;
                return false;// can prune
            }
            // update the distances
            state_updater_->update(state, decision_result);

            pen = state_evaluator_->bound(state);
            if (pen >= upper_bound_value_) {
                ++statistics.pruned_after_update;
                return false;
            }
            if (state.selection.isFull() && pen < upper_bound_value_) {
                // got new bound
                upper_bound_value_ = pen;
                upper_bound_ = state;
                state_store_->registerNewUpperBound(upper_bound_);
                ++statistics.upper_bound_updated;
                statistics.iteration_of_last_upper_bound_update = statistics.iterations;
            }

            if (state.selection.isFull()) {
                // have no usage for fully decided states
                ++statistics.complete_selections_considered;
                return false;
            }

            state.lower_bound = pen;
            state.last_decision_made = decision;
            return true;
        }


        void addToStatistics(State const &state, graph::choice_index_t choice) {
            auto type = graph_with_meta_.graph.choice_metadata[choice].type;
            statistics.choices_considered.of(type)++;
            statistics.max_choices_statistics.markConsidered(choice, type);

            auto const &sel = state.selection;
            auto const remaining_decisions = sel.remainingDecisions();
            statistics.closest_to_complete = std::min(statistics.closest_to_complete, remaining_decisions);
        }
        void handleDeadEndWithStats(graph::choice_index_t last_choice, State const &last_state) {
            auto depth = last_state.selection.madeDecisions();
            if (!state_store_->empty()) {// can be empty if the new upper bound already pruned all choices
                state_store_->popState();
            }
            ++statistics.dead_end_reached;
            auto const &choice_meta = graph_with_meta_.graph.choice_metadata[last_choice];
            statistics.last_choice_before_dead_end.of(choice_meta.type)++;

            if (!state_store_->empty()) {
                auto const &next_state = state_store_->peekState();
                statistics.dead_end_jumps += (depth - next_state.selection.madeDecisions());
                graph::choice_index_t jump_choice = next_state.last_decision_made.choice_index;
                auto type = graph_with_meta_.graph.choice_metadata[jump_choice].type;
                statistics.dead_end_jump_destination.of(type)++;
            }
        }


        graph::GraphWithMeta const &graph_with_meta_;

        State upper_bound_;
        penalty_t upper_bound_value_;


        StateStore *const state_store_;
        ChoiceSelector *const choice_selector_;
        StateEvaluator *const state_evaluator_;
        StateUpdater *const state_updater_;
    };


    // template argument deduction guide https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
    template<typename StateEvaluator, typename ChoiceSelector, typename StateStore, typename StateUpdater>
    BranchAndBoundAlgorithm(graph::GraphWithMeta const &graph_with_meta, const graph::Selection &upper_bound_selection,
                            StateStore &state_store, ChoiceSelector &choice_selector, StateEvaluator &state_evaluator,
                            StateUpdater &state_updater)
            -> BranchAndBoundAlgorithm<typename StateUpdater::state_type, StateEvaluator, ChoiceSelector, StateStore,
                                       StateUpdater>;

}// namespace fb
