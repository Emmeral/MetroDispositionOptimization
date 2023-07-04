
#include "BranchAndBoundRunner.h"
#include "algo/OnlineTopologicalSorter.h"
#include "algo/bab/states/DistanceState.h"
#include "algo/bab/states/TopologicalState.h"
#include "algo/bab/statestore/LIFOStateStore.h"
#include "algo/path/BackwardsPathCalculator.h"
#include "algo/path/LongestPathCalculator.h"
#include "penalty/PenaltyComputer.h"

#include "algo/CycleFinder.h"
#include "algo/bab/BranchAndBoundAlgorithm.h"
#include "algo/bab/StateUpdater.h"
#include "algo/bab/choiceselector/BiggestDistanceToEndChoiceSelector.h"
#include "algo/bab/choiceselector/SequentialChoiceSelector.h"
#include "algo/bab/evaluator/NoPeriodStateEvaluator.h"
#include "algo/bab/states/DistanceToEndState.h"
#include "algo/bab/statestore/LIFOStateStore.h"
#include "algo/bab/statestore/PenaltyOrderedStateStore.h"
#include "exceptions/CycleException.h"
#include "output/dot/graph/GraphDotWriter.h"


#include "algo/initialselection/FCFSSelectionFinder.h"
#include "algo/initialselection/NoopInitialSelectionFinder.h"
#include "algo/initialselection/OriginalInitialSelectionFinder.h"
#include "algo/initialselection/SimpleFCFSSelectionFinder.h"

#include "output/dot/graph/SubgraphWriter.h"
#include <chrono>

namespace fb {


    BranchAndBoundRunner::Result BranchAndBoundRunner::runBranchAndBound() {


        auto const &graph = gwm_.graph;
        OnlineTopologicalSorter sorter{graph};
        LongestPathCalculator pathCalculator{graph, gwm_.start_node, gwm_.measure_nodes.getNodes()};


        Result result(graph);

        graph::Selection empty_sel{graph};

        OnlineTopologicalSorter::State sorter_state = sorter.calculateInitialTopologicalOrder(empty_sel);
        LongestPathCalculator::State empty_sel_dist = pathCalculator.calculateInitialPaths(empty_sel, sorter_state);
        PenaltyComputer penalty_computer{gwm_, model_, problem_.snapshot_time};

        if (sorter_state.cycle) {
            throw CycleException("Found cycle in graph with empty selection", std::move(empty_sel));
        }

        DetailedPenalty no_dec_pen =
                logPenalty("without decisions", empty_sel, empty_sel_dist.distance_from_start, penalty_computer);
        DetailedPenalty init_sel_pen;


        std::cout << "Calculating initial selection" << std::endl;
        auto initial = retrieveSelectionOfOriginalDecisions(graph);
        std::cout << "Done calculating initial selection" << std::endl;

        if (initial.selection.isFull()) {
            if (!initial.order_state.has_value()) {
                initial.order_state = sorter.calculateInitialTopologicalOrder(initial.selection);
            }
            if (!initial.pc_state.has_value()) {
                initial.pc_state = pathCalculator.calculateInitialPaths(initial.selection, initial.order_state.value());
            }

            if (initial.order_state->cycle) {
                throw CycleException("Found cycle in graph with initial selection", std::move(initial.selection));
            }

            init_sel_pen = logPenalty("initial selection", initial.selection, initial.pc_state->distance_from_start,
                                      penalty_computer);
        }

        std::vector<distance_t> bias{};
        if (options_.distance_to_end_normalization) {
            for (auto const &mn: gwm_.measure_nodes.getNodes()) {
                bias.push_back(-empty_sel_dist.distance_from_start[mn]);
            }
        }
        BackwardsPathCalculator backwards_path_calculator{graph, gwm_.start_node, gwm_.measure_nodes.getNodes(), bias};


        auto start = std::chrono::high_resolution_clock::now();
        if (options_.state_config == "Default") {
            constexpr auto DISTANCES_TO_END = 1;

            typedef State<TopologicalState, DistanceState, DistanceToEndState<DISTANCES_TO_END>, ChoiceImpactState,
                          SkippedStopPenaltyState>
                    State;
            typedef BiggestDistanceToEndChoiceSelector<State> ChoiceSelector;
            typedef NoPeriodStateEvaluator<State> StateEvaluator;
            typedef CompositeUpdater<State, TopologicalStateUpdater<State>, DistanceStateUpdater<State>,
                                     DistanceToEndStateUpdater<State>, ChoiceImpactStateUpdater<State>,
                                     SkippedStopPenaltyStateUpdater<State>>
                    StateUpdater;

            ChoiceSelector choice_selector{};
            StateEvaluator state_evaluator{gwm_, penalty_computer, model_.base_station_values};
            StateUpdater updater{TopologicalStateUpdater<State>{sorter}, DistanceStateUpdater<State>{pathCalculator},
                                 DistanceToEndStateUpdater<State>{backwards_path_calculator},
                                 ChoiceImpactStateUpdater<State>{
                                         gwm_.abstractions, model_, gwm_.graph, empty_sel_dist.distance_from_start,
                                         options_.skipped_stop_impact_factor, options_.relation_impact_factor},
                                 SkippedStopPenaltyStateUpdater<State>(model_.base_station_values, model_.courses,
                                                                       gwm_.abstractions)};

            result = dispatchRun<State>(std::move(initial.selection), updater, state_evaluator, choice_selector);
        }


        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        result.solve_time_ms = duration.count();

        result.stats.no_selection_penalty = no_dec_pen;
        result.stats.initial_selection_penalty = init_sel_pen;


        if (result.result_selection.isFull()) {
            auto best_dfs = pathCalculator.calculateInitialPaths(result.result_selection).distance_from_start;
            auto best_pen = logPenalty("best solution found", result.result_selection, best_dfs, penalty_computer);
            result.stats.best_selection_penalty = best_pen;
        } else {
            std::cout << "No solution found in time" << std::endl;
        }

        return std::move(result);
    }
    InitialSelectionFinder::Result
    BranchAndBoundRunner::retrieveSelectionOfOriginalDecisions(const graph::AlternativeGraph &graph) const {


        switch (options_.initial_selection) {
            case BranchAndBoundOptions::InitialSelection::ORIGINAL: {
                OriginalInitialSelectionFinder finder{};
                return finder.calculateInitialSelectionWithMeta(gwm_);
            }
            case BranchAndBoundOptions::InitialSelection::FCFS: {
                FCFSSelectionFinder fcfs_finder{model_.duties};
                return fcfs_finder.calculateInitialSelectionWithMeta(gwm_);
            }
            case BranchAndBoundOptions::InitialSelection::SIMPLE_FCFS: {
                SimpleFCFSSelectionFinder fcfs_finder{model_.duties};
                return fcfs_finder.calculateInitialSelectionWithMeta(gwm_);
            }
            case BranchAndBoundOptions::InitialSelection::NOOP:
            default: {
                NoopInitialSelectionFinder noop_finder{};
                return noop_finder.calculateInitialSelectionWithMeta(gwm_);
            }
        }
    }

    DetailedPenalty BranchAndBoundRunner::logPenalty(const std::string &name, const graph::Selection &sel,
                                                     const std::vector<distance_t> &dfs,
                                                     const PenaltyComputer &penalty_computer) const {

        DetailedPenalty penalty{};
        penalty.delay_penalty = penalty_computer.delay_computer.computeDelayPenalty(dfs);
        penalty.skipped_stop_penalty = penalty_computer.computeSkippedStopPenalty(sel);
        penalty.frequency_penalty = penalty_computer.computeTargetFrequencyPenalty(sel, dfs);
        std::cout << "Penalty " << name << " :"
                  << "\n";
        std::cout << "  Delay Penalty: " << penalty.delay_penalty << "\n";
        std::cout << "  Skipped Stop Penalty: " << penalty.skipped_stop_penalty << "\n";
        std::cout << "  Frequency Penalty: " << penalty.frequency_penalty << "\n";
        std::cout << "  Total Penalty: " << penalty.total() << "\n";

        return penalty;
    }

    BranchAndBoundRunner::BranchAndBoundRunner(const BranchAndBoundOptions &options, Model const &model,
                                               graph::GraphWithMeta const &gwm, ProblemInstance const &problem)
        : options_(options), gwm_(gwm), model_(model), problem_(problem) {}
    BranchAndBoundRunner::Result::Result(graph::AlternativeGraph const &graph)
        : stats{graph}, result_selection{graph} {}
    BranchAndBoundRunner::Result::Result(BABStatistics &&stats, graph::Selection &&selection)
        : stats{stats}, result_selection{selection} {}
}// namespace fb
