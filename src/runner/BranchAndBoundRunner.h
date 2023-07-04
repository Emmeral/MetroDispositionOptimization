
#pragma once

#include "algo/bab/BranchAndBoundAlgorithm.h"
#include "algo/bab/statestore/BestOfLastInsertedStateStore.h"
#include "algo/bab/statestore/LIFOStateStore.h"
#include "algo/bab/statestore/PenaltyOrderedStateStore.h"
#include "algo/initialselection/InitialSelectionFinder.h"
#include "model/graph/GraphWithMeta.h"
#include "model/graph/Selection.h"
#include "options/BranchAndBoundOptions.h"
#include "penalty/DetailedPenalty.h"
#include "penalty/PenaltyComputer.h"
#include "penalty/definitions.h"
#include "statistics/bab/BranchAndBoundStatistics.h"
#include <chrono>
namespace fb {


    class BranchAndBoundRunner {

    public:
        explicit BranchAndBoundRunner(const BranchAndBoundOptions &options, Model const &model,
                                      graph::GraphWithMeta const &gwm, ProblemInstance const &problem);


        struct Result {
            Result(graph::AlternativeGraph const &graph);
            Result(BABStatistics &&stats, graph::Selection &&selection);

            BABStatistics stats;
            penalty_t result_penalty{0};
            graph::Selection result_selection;
            unsigned long solve_time_ms{0};
        };


        fb::BranchAndBoundRunner::Result runBranchAndBound();


    private:
        Model const &model_;
        ProblemInstance const &problem_;
        graph::GraphWithMeta const &gwm_;
        BranchAndBoundOptions const &options_;

        DetailedPenalty logPenalty(const std::string &name, const graph::Selection &sel,
                                   const std::vector<distance_t> &dfs, const PenaltyComputer &penalty_computer) const;
        InitialSelectionFinder::Result retrieveSelectionOfOriginalDecisions(const graph::AlternativeGraph &graph) const;


        template<typename State, typename StateUpdater, typename StateEvaluator, typename ChoiceSelector>
        Result dispatchRun(graph::Selection &&original_sel, StateUpdater &state_updater,
                           StateEvaluator &state_evaluator, ChoiceSelector &choice_selector) {

            switch (options_.store) {
                case BranchAndBoundOptions::StoreOption::LIFO: {

                    LIFOStateStore<State> lifo_state_store{};
                    return dispatchRun<State>(std::move(original_sel), state_updater, state_evaluator, choice_selector,
                                              lifo_state_store);
                }
                case BranchAndBoundOptions::StoreOption::BOL: {
                    BestOfLastInsertedStateStore<State, StateUpdater> bol_state_store{
                            options_.bol_threshold, options_.store_max_load, state_updater};
                    return dispatchRun<State>(std::move(original_sel), state_updater, state_evaluator, choice_selector,
                                              bol_state_store);
                }
                case BranchAndBoundOptions::StoreOption::PO: {
                    PenaltyOrderedStateStore<State> po_state_store{};
                    return dispatchRun<State>(std::move(original_sel), state_updater, state_evaluator, choice_selector,
                                              po_state_store);
                }
                default:
                    throw std::invalid_argument("Unsupported Store");
            }
        }

        template<typename State, typename StateUpdater, typename StateEvaluator, typename ChoiceSelector,
                 typename StateStore>
        Result dispatchRun(graph::Selection &&original_sel, StateUpdater &state_updater,
                           StateEvaluator &state_evaluator, ChoiceSelector &choice_selector, StateStore &state_store) {
            auto bab = BranchAndBoundAlgorithm{
                    gwm_, std::move(original_sel), state_store, choice_selector, state_evaluator, state_updater};
            return run(bab);
        }

        template<typename BABAlgo>
        Result run(BABAlgo &bab) {

            auto start = std::chrono::high_resolution_clock::now();
            auto bound_ms = options_.timeout;

            auto ms_since_start = 0l;
            long last_log_time = 0;
            double speed = 10.0;
            unsigned int last_log_iteration = 0;
            unsigned int next_log_iteration = 5;

            bool could_do_step = true;
            while (could_do_step && ms_since_start < bound_ms) {
                could_do_step = bab.doStep();

                unsigned int iteration = bab.statistics.iterations;
                if (iteration >= next_log_iteration) {
                    ms_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(
                                             std::chrono::high_resolution_clock::now() - start)
                                             .count();

                    double current_speed =
                            ((iteration - last_log_iteration)) / ((double) (ms_since_start - last_log_time) / 1000.0);
                    speed = std::min(speed * 0.9 + current_speed * 0.1, 1000.0);
                    last_log_time = ms_since_start;
                    last_log_iteration = iteration;
                    next_log_iteration = iteration + std::max(0, (int) speed);
                    std::cout << "\rIteration " << iteration << " Time: " << ms_since_start / 1000
                              << "s Speed: " << current_speed << "/s"
                              << " Bound:" << bab.getBestSolutionBound() << "    ";
                    std::cout.flush();
                }
            }
            auto best_solution = bab.getBestSolutionFound();

            std::cout << "\n";
            if (could_do_step) {
                bab.statistics.aborted = true;
            }


            return Result{std::move(bab.statistics), std::move(best_solution.selection)};
        }
    };


}// namespace fb
