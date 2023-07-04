
#include "MainRunner.h"
#include "FCFSRunner.h"
#include "IteratedRunner.h"
#include "NonIteratedRunner.h"
#include "Runner.h"
#include "builder/ModelBuilder.h"
#include "output/timetable/AmendmentWriter.h"
#include "parser/DataParser.h"
#include "parser/ProblemParser.h"
#include "parser/sanitizer/InputSanitizer.h"
#include "parser/sanitizer/ProblemSanitizer.h"
#include "penalty/schedule/AmendmentPenaltyComputer.h"
#include "util.h"
#include "validation/ScheduleValidator.h"
#include <chrono>
#include <filesystem>
#include <iostream>

namespace fb {
    void MainRunner::runAndSave(const Options &options) {

        auto name = options.name;
        if (name.empty()) {
            auto last_slash = options.problem.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                name = options.problem.substr(last_slash + 1);
            } else {
                name = nowAsString("%Y-%m-%d-%H-%M-%S");
            }
        }

        auto base_out_folder = options.solution + "/" + name;

        // clear output folder
        auto counter = 1;
        while (std::filesystem::exists(base_out_folder)) {
            base_out_folder = options.solution + "/" + name + "_" + std::to_string(counter);
            ++counter;
        };

        std::cout << "Using output folder " << base_out_folder << "\n";
        std::filesystem::create_directories(base_out_folder);

        std::ofstream file;
        file.open(base_out_folder + "/Options.json");
        file << toJson(options) << "\n";
        file.close();


        auto [model, model_stats] = parseModelAndLog(options);

        file.open(base_out_folder + "/ModelStatistics.json");
        file << toJson(model_stats) << "\n";
        file.close();


        if (options.batch) {
            std::filesystem::directory_iterator it{options.problem};

            std::vector<std::filesystem::directory_entry> entries{};
            while (it != std::filesystem::directory_iterator{}) {
                if (it->is_directory()) {
                    entries.push_back(*it);
                }
                ++it;
            }

            std::sort(entries.begin(), entries.end(),
                      [](const std::filesystem::directory_entry &a, const std::filesystem::directory_entry &b) {
                          return a.path().filename().string() < b.path().filename().string();
                      });

            auto i = 1;
            for (auto &de: entries) {
                std::cout << "Problem " << i << "/" << entries.size() << "\n";
                runAndSaveSingle(options, model, de.path(), base_out_folder + "/" + de.path().filename().string());
                ++i;
            }
        } else {
            runAndSaveSingle(options, model, options.problem, base_out_folder);
        }
    }
    std::pair<Model, ModelStatistics> MainRunner::parseModelAndLog(const Options &options) {


        std::cout << "\nParsing model\n";
        auto start = std::chrono::high_resolution_clock::now();

        auto parser = DataParser();
        auto input_data = parser.parse(options.model);


        auto sanitizer = InputSanitizer(input_data);

        if (options.sanitize) {
            sanitizer.overrideWrongIndividualData();
            sanitizer.expandNodeTracks();
            sanitizer.correctMinimalRuntimes();
            sanitizer.correctMinimalHeadways();
        }

        auto model = ModelBuilder::buildFromInput(input_data);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);


        ModelStatistics m_stats{model, duration.count()};
        std::cout << m_stats;

        TimetableAmendment amendment{};
        amendment.courses = model.courses;
        amendment.duties = model.duties;
        ProblemInstance instance{};
        instance.snapshot_time = 0;// empty problem
        instance.realized_schedule.resize(model.courses.size());
        ScheduleValidator validator{model, instance};
        auto violations = validator.validate(amendment);
        if (!violations.empty()) {
            std::cout << "Violations in original timetable model:\n";
            for (auto const &v: violations) {
                std::cout << v << "\n";
            }
            std::cout << std::flush;
            throw std::runtime_error("Violations in original timetable model");
        }


        return {std::move(model), std::move(m_stats)};
    }
    void MainRunner::runAndSaveSingle(const Options &options, const Model &model,
                                      const std::filesystem::path &problem_folder, std::string const &out_folder) {

        RunnerResult result = runSingle(options, model, problem_folder.string());

        if (!options.solution.empty()) {
            AmendmentWriter writer{model};
            writer.writeAmendment(result.amendment, out_folder, result.validation_result, &result.penalty);
        }


        std::ofstream file;
        file.open(out_folder + "/Statistics.json");
        file << toJson(result.statistics) << "\n";
        file.close();


        file.open(out_folder + "/Penalty.json");
        file << toJson(result.penalty) << "\n";
        file.close();

        if (!result.validation_result.empty()) {
            std::cout << "INVALID RESULT for " << problem_folder << "\n";
        }
    }
    ProblemInstance MainRunner::parseProblemAndLog(const std::string &problem_folder, const Model &model,
                                                   SingleRunStatistics &stats) {

        std::cout << "\nParsing problem\n";
        auto start = std::chrono::high_resolution_clock::now();

        ProblemParser problem_parser{model};
        auto problem_instance = problem_parser.parse(problem_folder);

        ProblemSanitizer problem_sanitizer{model};
        problem_sanitizer.sanitizeRealizedSchedule(problem_instance);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        ProblemStatistics p_stats{problem_instance, duration.count()};
        std::cout << p_stats;
        stats.problem_statistics = p_stats;

        return std::move(problem_instance);
    }
    MainRunner::RunnerResult MainRunner::runSingle(const Options &options, const Model &model,
                                                   const std::string &problem_folder) {

        SingleRunStatistics statistics{};
        RunnerResult result{};
        auto start = std::chrono::high_resolution_clock::now();
        ProblemInstance problem = parseProblemAndLog(problem_folder, model, statistics);

        TimetableAmendment amendment;

        switch (options.mode) {

            case Options::Mode::Iterated: {
                IteratedRunner runner{options, model};
                amendment = runner.run(problem, statistics);
                break;
            }
            case Options::Mode::Single: {
                NonIteratedRunner runner{options, model};
                amendment = runner.run(problem, statistics);
                break;
            }
            case Options::Mode::FCFS: {

                FCFSRunner runner{options, model};
                amendment = runner.run(problem, statistics);
                break;
            }
            default:
                throw std::runtime_error("Unknown mode");
        }

        auto end = std::chrono::high_resolution_clock::now();
        statistics.total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (options.validate) {
            ScheduleValidator validator{model, problem};
            auto validation_result = validator.validate(amendment);
            result.validation_result = std::move(validation_result);
        }

        AmendmentPenaltyComputer penalty_computer{model, problem};
        auto penalty = penalty_computer.computeAmendmentPenalty(amendment, true);
        result.penalty = penalty;
        result.statistics = statistics;
        result.amendment = std::move(amendment);

        return std::move(result);
    }
}// namespace fb
