
#pragma once

#include "model/main/Model.h"
#include "model/problems/ProblemInstance.h"
#include "options/Options.h"
#include "output/timetable/TimetableAmendment.h"
#include "penalty/DetailedPenalty.h"
#include "statistics/SingleRunStatistics.h"
#include "statistics/bab/BranchAndBoundStatistics.h"
#include "statistics/graph/GraphStatistics.h"
#include <filesystem>
namespace fb {

    class MainRunner {
    public:
        struct RunnerResult {
            TimetableAmendment amendment;
            DetailedPenalty penalty;
            std::vector<std::string> validation_result{};
            SingleRunStatistics statistics;
        };


        static void runAndSave(const Options &options);

        static void runAndSaveSingle(const Options &options, const Model &model,
                                     const std::filesystem::path &problem_folder, std::string const &out_folder);
        static RunnerResult runSingle(const Options &options, const Model &model, const std::string &problem_folder);

    private:
        static std::pair<Model, ModelStatistics> parseModelAndLog(const Options &options);
        static ProblemInstance parseProblemAndLog(const std::string &problem_folder, const Model &model,
                                                  SingleRunStatistics &stats);
    };

}// namespace fb
