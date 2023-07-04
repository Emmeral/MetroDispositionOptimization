
#include "ProblemWriter.h"
#include "util.h"
#include <filesystem>
#include <fstream>

namespace fb {


    void ProblemWriter::writeProblem(const ProblemInstance &problem, const std::string &folder) {

        std::filesystem::remove_all(folder);
        std::filesystem::create_directories(folder);
        auto snapshot_time_path = folder + "/SNAPSHOT_TIME.txt";

        std::ofstream snapshot_time_file(snapshot_time_path);
        snapshot_time_file << std::to_string(problem.snapshot_time) << "\n";
        snapshot_time_file.close();

        auto realized_schedule_path = folder + "/REALIZED_SCHEDULE.csv";
        std::ofstream realized_schedule_file(realized_schedule_path);

        CSVRow header_realized_schedule{{"TRAIN_COURSE_ID", "SEQ", "NODE", "ARRIVAL_SECONDS", "ARRIVAL_HHMMSS",
                                         "DEPARTURE_SECONDS", "DEPARTURE_HHMMSS", "TRACK"}};
        realized_schedule_file << header_realized_schedule << "\n";
        for (auto cid = 0; cid < problem.realized_schedule.size(); ++cid) {
            for (auto const &si: problem.realized_schedule[cid]) {
                realized_schedule_file << writeRealizedScheduleItem(si, model_.courses[cid], problem) << "\n";
            }
        }


        if (!problem.extended_runtimes.empty()) {
            std::ofstream extended_runtimes_file(folder + "/EXTENDED_RUN_TIMES.csv");
            CSVRow header_extended_runtimes{{"LINK_START_NODE", "LINK_END_NODE", "START_TIME_SECONDS",
                                             "END_TIME_SECONDS", "START_TIME_HHMMSS", "END_TIME_HHMMSS",
                                             "EXTENDED_RUN_TIME_SECONDS"}};
            extended_runtimes_file << header_extended_runtimes << "\n";
            for (auto const &ert: problem.extended_runtimes) {
                extended_runtimes_file << writeExtendedRuntime(ert) << "\n";
            }
        }

        if (!problem.extended_dwelltimes.empty()) {
            std::ofstream extended_dwell_times_file(folder + "/STATION_EXTENDED_DWELL.csv");
            CSVRow header_extended_dwell_times{{"NODE", "START_TIME_SECONDS", "END_TIME_SECONDS", "START_TIME_HHMMSS",
                                                "END_TIME_HHMMSS", "EXTENDED_RUN_TIME_SECONDS"}};
            extended_dwell_times_file << header_extended_dwell_times << "\n";
            for (auto const &edt: problem.extended_dwelltimes) {
                extended_dwell_times_file << writeExtendedDwellTime(edt) << "\n";
            }
        }

        if (!problem.extended_train_dwelltimes.empty()) {
            std::ofstream course_dwell_times_file(folder + "/TRAIN_EXTENDED_DWELL.csv");
            CSVRow header_course_dwell_times{{"TRAIN_COURSE_ID", "NODE", "EXTENDED_RUN_TIME_SECONDS"}};
            course_dwell_times_file << header_course_dwell_times << "\n";
            for (auto const &cdt: problem.extended_train_dwelltimes) {
                course_dwell_times_file << writeCourseDwellTime(cdt) << "\n";
            }
        }

        if (!problem.late_departures.empty()) {
            std::ofstream late_departures_file(folder + "/LATE_DEPARTURES.csv");
            CSVRow header_late_departures{{"TRAIN_COURSE_ID", "DEPARTURE_DELAY_SECONDS"}};
            late_departures_file << header_late_departures << "\n";
            for (auto const &ld: problem.late_departures) {
                late_departures_file << writeLateDeparture(ld) << "\n";
            }
        }
    }
    ProblemWriter::ProblemWriter(const Model &model) : model_(model) {}
    CSVRow ProblemWriter::writeExtendedRuntime(const ExtendedRunTime &extended_run_time) {
        auto const &link = model_.links[extended_run_time.link];
        std::vector<std::string> data = {model_.node_metadata[link.link_start_node].code,
                                         model_.node_metadata[link.link_end_node].code,
                                         std::to_string(extended_run_time.start),
                                         std::to_string(extended_run_time.end),
                                         readable(extended_run_time.start),
                                         readable(extended_run_time.end),
                                         std::to_string(extended_run_time.extended_runtime)};
        return CSVRow(data);
    }
    CSVRow ProblemWriter::writeExtendedDwellTime(const ExtendedDwellTime &extended_dwell_time) {
        std::vector<std::string> data = {model_.node_metadata[extended_dwell_time.node].code,
                                         std::to_string(extended_dwell_time.start),
                                         std::to_string(extended_dwell_time.end),
                                         readable(extended_dwell_time.start),
                                         readable(extended_dwell_time.end),
                                         std::to_string(extended_dwell_time.extended_dwell)};
        return CSVRow(data);
    }
    CSVRow ProblemWriter::writeCourseDwellTime(const TrainExtendedDwell &course_dwell_time) {
        auto const &course_code = model_.course_code[course_dwell_time.course];
        std::vector<std::string> data = {course_code, model_.node_metadata[course_dwell_time.node].code,
                                         std::to_string(course_dwell_time.extended_dwell)};

        return CSVRow(data);
    }
    CSVRow ProblemWriter::writeLateDeparture(const LateDeparture &late_departure) {
        std::vector<std::string> data = {model_.course_code[late_departure.course],
                                         std::to_string(late_departure.departure_delay)};

        return CSVRow(data);
    }
    CSVRow ProblemWriter::writeRealizedScheduleItem(const ScheduleItem &schedule_item, const Course &course,
                                                    ProblemInstance const &problem) {

        seconds_t raw_arrival = schedule_item.arrival;
        seconds_t raw_departure = schedule_item.departure;
        if (isInvalid(raw_arrival)) {
            auto const *prior_course = model_.priorCourseInDuty(course);
            if (prior_course != nullptr) {
                raw_arrival = problem.realized_schedule[prior_course->index].back().arrival;
            }
        }
        if (isInvalid(raw_departure)) {
            auto const *next_course = model_.nextCourseInDuty(course);
            if (next_course != nullptr && !problem.realized_schedule[next_course->index].empty()) {
                raw_departure = problem.realized_schedule[next_course->index].front().departure;
            }
        }

        std::string arrival = isInvalid(raw_arrival) ? "" : std::to_string(raw_arrival);
        std::string arrival_readable = isInvalid(raw_arrival) ? "" : readable(raw_arrival);
        std::string departure = isInvalid(raw_departure) ? "" : std::to_string(raw_departure);
        std::string departure_readable = isInvalid(raw_departure) ? "" : readable(raw_departure);

        std::vector<std::string> data = {
                model_.course_code[course.index],
                std::to_string(schedule_item.index + 1),
                model_.node_metadata[schedule_item.node].code,
                arrival,
                arrival_readable,
                departure,
                departure_readable,
                model_.original_tracks_mapping[schedule_item.node][schedule_item.original_track],
                reverseLookup(activity_table, schedule_item.original_activity)};
        return CSVRow(data);
    }
}// namespace fb
