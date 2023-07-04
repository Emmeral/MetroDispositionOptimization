
#include "IteratedRunner.h"
#include "BranchAndBoundRunner.h"
#include "builder/graph/GraphBuilder.h"
#include "exceptions/CycleException.h"
#include "output/timetable/TimetableCreator.h"
#include "statistics/SingleRunStatistics.h"
#include "util.h"

namespace fb {
    IteratedRunner::IteratedRunner(const Options &options, const Model &model) : Runner(options, model) {}


    TimetableAmendment IteratedRunner::run(const ProblemInstance &original_problem, SingleRunStatistics &stats) {

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
            BranchAndBoundRunner runner{options_.bab, model_, gwm, *current_problem};

            try {
                std::cout << "IteratedRunner iteration: " << i++ << "\n";
                auto bab_result = runner.runBranchAndBound();

                stats.time_per_iteration.emplace_back(bab_result.solve_time_ms);
                stats.bab_statistics.emplace_back(std::move(bab_result.stats));

                // extract schedule up until some point
                TimetableCreator creator{model_, current_problem->realized_schedule};
                partial_sched = creator.createPartialSchedule(bab_result.result_selection, gwm);


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
}// namespace fb
