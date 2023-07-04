
#include "NonIteratedRunner.h"
#include "BranchAndBoundRunner.h"
#include "algo/initialselection/FCFSSelectionFinder.h"
#include "output/timetable/TimetableCreator.h"

namespace fb {
    NonIteratedRunner::NonIteratedRunner(const Options &options, const Model &model) : Runner(options, model) {}


    TimetableAmendment NonIteratedRunner::run(const ProblemInstance &original_problem, SingleRunStatistics &stats) {

        TimetableCreator base_creator{model_, original_problem.realized_schedule};

        // build graph
        auto start = std::chrono::steady_clock::now();
        auto gwm = graph_builder_.build(original_problem);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        stats.graph_statistics.emplace_back(gwm, duration);
        stats.snapshot_times.emplace_back(original_problem.snapshot_time);
        // solve
        BranchAndBoundRunner runner{options_.bab, model_, gwm, original_problem};

        auto bab_result = runner.runBranchAndBound();

        stats.time_per_iteration.emplace_back(bab_result.solve_time_ms);
        stats.bab_statistics.emplace_back(std::move(bab_result.stats));

        // extract schedule up until some point
        PartialSchedule partial_sched = base_creator.createPartialSchedule(bab_result.result_selection, gwm);


        if (checkDone(partial_sched)) {
            return base_creator.createAmendedTimetable(partial_sched);
        }

        auto first_End = partial_sched.firstUncompletedEnd(model_.duties, model_.courses);
        assert(isValid(first_End));
        partial_sched.pruneToSnapshotTime(first_End);

        stats.snapshot_times.emplace_back(first_End);
        // complete schedule with planned schedule, if it is on time and there are no more future problems
        if (problemsInPast(partial_sched, original_problem) && checkEndsOnTime(partial_sched)) {
            completeWithPlanned(partial_sched);
            stats.completed_with_planned = true;
            return base_creator.createAmendedTimetable(partial_sched);
        }

        // if there are still problems and the schedule is not on time use FCFS to solve remaining schedule
        ProblemInstance remaining_problem = original_problem;
        remaining_problem.realized_schedule = std::move(partial_sched.schedule);
        remaining_problem.snapshot_time = first_End;


        // build graph of remaining problem independent of initial extend
        GraphBuilderOptions adapted_options = options_.graph;
        adapted_options.extend = std::numeric_limits<int>::max();
        GraphBuilder remaining_graph_builder{model_, adapted_options};

        start = std::chrono::steady_clock::now();
        auto remaining_graph = remaining_graph_builder.build(remaining_problem);
        end = std::chrono::steady_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        stats.graph_statistics.emplace_back(remaining_graph, duration);

        FCFSSelectionFinder fcfs_finder{model_.duties};
        auto result = fcfs_finder.calculateInitialSelectionWithMeta(remaining_graph);


        TimetableCreator remaining_creator{model_, remaining_problem.realized_schedule};
        return remaining_creator.createAmendedTimetable(result.selection, remaining_graph,
                                                        result.pc_state->distance_from_start);
    }
}// namespace fb
