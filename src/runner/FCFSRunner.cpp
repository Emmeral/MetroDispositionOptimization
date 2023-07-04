
#include "FCFSRunner.h"
#include "algo/initialselection/FCFSSelectionFinder.h"
#include "exceptions/CycleException.h"
#include "output/timetable/TimetableCreator.h"
#include "penalty/PenaltyComputer.h"
#include "util.h"
#include <chrono>

namespace fb {


    TimetableAmendment FCFSRunner::run(const ProblemInstance &original_problem, SingleRunStatistics &stats) {


        ProblemInstance const *current_problem = &original_problem;
        PartialSchedule partial_sched{0};
        ProblemInstance problem_store{};
        auto i = 1;

        while (true) {

            // build graph
            auto start = std::chrono::steady_clock::now();
            auto gwm = graph_builder_.build(*current_problem);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            stats.graph_statistics.emplace_back(gwm, duration);
            stats.snapshot_times.emplace_back(current_problem->snapshot_time);
            // solve

            PenaltyComputer penalty_computer{gwm, model_, current_problem->snapshot_time};
            FCFSSelectionFinder fcfs_finder{model_.duties};
            try {
                std::cout << "IteratedFCFSRunner iteration: " << i++ << "\n";

                start = std::chrono::steady_clock::now();
                auto result = fcfs_finder.calculateInitialSelectionWithMeta(gwm);
                end = std::chrono::steady_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                stats.time_per_iteration.emplace_back(duration);
                stats.bab_statistics.emplace_back(gwm.graph);

                DetailedPenalty penalty{};
                penalty.delay_penalty =
                        penalty_computer.delay_computer.computeDelayPenalty(result.pc_state->distance_from_start);
                penalty.skipped_stop_penalty = penalty_computer.computeSkippedStopPenalty(result.selection);
                penalty.frequency_penalty = penalty_computer.computeTargetFrequencyPenalty(
                        result.selection, result.pc_state->distance_from_start);
                stats.bab_statistics.back().initial_selection_penalty = penalty;

                // extract schedule up until some point
                TimetableCreator creator{model_, current_problem->realized_schedule};
                partial_sched = creator.createPartialSchedule(result.selection, gwm);


                if (checkDone(partial_sched)) {
                    break;
                }

                partial_sched.pruneToSnapshotTime(partial_sched.end_of_validity);

                // complete schedule with planned schedule, if it is on time and there are no more future problems
                if (problemsInPast(partial_sched, *current_problem) && checkEndsOnTime(partial_sched)) {
                    completeWithPlanned(partial_sched);
                    stats.completed_with_planned = true;
                    break;
                }

                // construct new problem based on new realized schedule
                problem_store = *current_problem;

                problem_store.realized_schedule = std::move(partial_sched.schedule);
                problem_store.snapshot_time = partial_sched.end_of_validity;
                current_problem = &problem_store;
            } catch (CycleException const &e) {
                logCycleException(gwm, e);
                throw e;
            }
        }

        TimetableCreator creator{model_, original_problem.realized_schedule};
        return creator.createAmendedTimetable(partial_sched);
    }
    FCFSRunner::FCFSRunner(const Options &options, const Model &model) : Runner(options, model) {}
}// namespace fb
