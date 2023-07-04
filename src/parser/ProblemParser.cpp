
#include "ProblemParser.h"
#include "parser/csv/CSVParser.h"
#include <filesystem>
#include <fstream>

namespace fb {

    ProblemParser::ProblemParser(const Model &model) : model_(model) {}


    ProblemInstance ProblemParser::parse(std::string const &directory) {
        std::cout << "Parsing directory" << directory << "\n";

        ProblemInstance instance{};
        instance.snapshot_time = invalid<seconds_t>();
        instance.realized_schedule.resize(model_.courses.size());

        int parsed_snapshot_time = invalid<int>();

        for (const auto &entry: std::filesystem::directory_iterator(directory)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            std::filesystem::path const &path = entry.path();
            if (path.has_extension() && path.extension() == ".csv") {
                std::ifstream fileStream(path);
                auto const rows = CSVParser::parse(fileStream);
                auto const &filename = path.filename().generic_string();
                constructFromCsv(rows, filename, instance);
            }

            if (path.filename().generic_string() == "SNAPSHOT_TIME.txt") {
                std::ifstream fileStream(path);
                std::string line;
                std::getline(fileStream, line);
                parsed_snapshot_time = std::stoi(line);
            }
        }


        instance.snapshot_time = 0;
        for (auto &v: instance.realized_schedule) {
            for (auto &si: v) {
                if (isValid(si.departure) && si.departure > instance.snapshot_time) {
                    instance.snapshot_time = si.departure;
                } else if (isValid(si.arrival) && si.arrival > instance.snapshot_time) {
                    instance.snapshot_time = si.arrival;
                }
            }
        }
        if (instance.snapshot_time > parsed_snapshot_time) {
            std::cout << "Supplied Snapshot time is not consistent wit realized schedule. Manually calculated: "
                      << instance.snapshot_time << " but supplied " << parsed_snapshot_time << "\n";
        }

        if (instance.snapshot_time < parsed_snapshot_time) {
            instance.snapshot_time = parsed_snapshot_time;
        }


        instance.last_incident_end = instance.snapshot_time;

        for (auto &ert: instance.extended_runtimes) {
            instance.mapped_extended_runtimes[ert.link].push_back(&ert);
            instance.last_incident_end = std::max(instance.last_incident_end, ert.end);
        }
        for (auto &sed: instance.extended_dwelltimes) {
            instance.mapped_extended_dwelltimes[sed.node].push_back(&sed);
            instance.last_incident_end = std::max(instance.last_incident_end, sed.end);
        }
        for (auto &ted: instance.extended_train_dwelltimes) {
            instance.mapped_extended_train_dwelltimes[{ted.course, ted.node}] = &ted;

            for (auto const &si: model_.courses[ted.course].schedule) {
                if (si.node == ted.node) {
                    auto time = si.arrival + ted.extended_dwell;
                    instance.last_incident_end = std::max(instance.last_incident_end, time);
                    break;
                }
            }
        }
        for (auto &ld: instance.late_departures) {
            instance.mapped_late_departures[ld.course] = &ld;

            auto time = model_.courses[ld.course].planned_start + ld.departure_delay;
            instance.last_incident_end = std::max(instance.last_incident_end, time);
        }

        return std::move(instance);
    }
    void ProblemParser::constructFromCsv(const std::vector<CSVRow> &rows, const std::string &filename,
                                         ProblemInstance &instance) {
        auto name_no_ending = filename.substr(0, filename.size() - 4);// remove .csv
        auto const &without_header = rows | std::views::drop(1);


        if (name_no_ending == "EXTENDED_RUN_TIME" || name_no_ending == "EXTENDED_RUN_TIMES") {
            instance.extended_runtimes.resize(rows.size() - 1);
            std::ranges::transform(without_header, instance.extended_runtimes.begin(),
                                   [&](auto const &r) { return this->parseExtendedRunTime(r); });
        } else if (name_no_ending == "STATION_EXTENDED_DWELL" || name_no_ending == "STATION_EXT_DWELL") {
            instance.extended_dwelltimes.resize(rows.size() - 1);
            std::ranges::transform(without_header, instance.extended_dwelltimes.begin(),
                                   [&](auto const &r) { return this->parseExtendedDwellTime(r); });
        } else if (name_no_ending == "LATE_DEPARTURES") {
            instance.late_departures.resize(rows.size() - 1);
            std::ranges::transform(without_header, instance.late_departures.begin(),
                                   [&](auto const &r) { return this->parseLateDeparture(r); });
        } else if (name_no_ending == "TRAIN_EXTENDED_DWELL" || name_no_ending == "TRAIN_EXT_DWELL") {
            instance.extended_train_dwelltimes.resize(rows.size() - 1);
            std::ranges::transform(without_header, instance.extended_train_dwelltimes.begin(),
                                   [&](auto const &r) { return this->parseExtendedTrainDwell(r); });
        } else if (name_no_ending == "REALIZED_SCHEDULE") {
            for (auto const &row: without_header) {
                parseAndInsertScheduleItem(row, instance);
            }
        }
    }
    ExtendedRunTime ProblemParser::parseExtendedRunTime(const CSVRow &row) {

        ExtendedRunTime ert{};

        auto start_node = row[0];
        auto end_node = row[1];
        auto start_id = model_.code_to_node_index.at(start_node);
        auto end_id = model_.code_to_node_index.at(end_node);
        ert.link = model_.getLinkFromTo(start_id, end_id)->id;

        ert.start = std::stoi(row[2]);
        ert.end = std::stoi(row[3]);
        ert.extended_runtime = std::stoi(row[6]);

        return ert;
    }
    LateDeparture ProblemParser::parseLateDeparture(const CSVRow &row) {
        LateDeparture ldp{};
        auto course = row[0];
        ldp.course = model_.code_to_course_index.at(course);

        ldp.departure_delay = std::stoi(row[1]);
        return ldp;
    }
    ExtendedDwellTime ProblemParser::parseExtendedDwellTime(const CSVRow &row) {

        ExtendedDwellTime edt{};

        edt.node = model_.code_to_node_index.at(row[0]);
        edt.start = std::stoi(row[1]);
        edt.end = std::stoi(row[2]);
        edt.extended_dwell = std::stoi(row[5]);

        return edt;
    }
    TrainExtendedDwell ProblemParser::parseExtendedTrainDwell(const CSVRow &row) {
        TrainExtendedDwell ted{};

        ted.course = model_.code_to_course_index.at(row[0]);
        ted.node = model_.code_to_node_index.at(row[1]);
        ted.extended_dwell = std::stoi(row[2]);

        return ted;
    }
    void ProblemParser::parseAndInsertScheduleItem(const CSVRow &row, ProblemInstance &instance) {

        ScheduleItem si{};

        std::string course = row[0];

        if (!model_.code_to_course_index.contains(course)) {
            // std::cout << "Found realized course not in original schedule: " << course << " Ignoring it!\n";
            return;
        }

        course_index_t course_id = model_.code_to_course_index.at(course);

        auto seq = std::stoi(row[1]);

        ScheduleItem const &in_schedule = model_.courses[course_id].schedule[seq - 1];


        std::string node = row[2];
        node_index_t node_id = model_.code_to_node_index.at(node);

        si.index = seq - 1;
        si.node = node_id;

        si.arrival = row[3].empty() ? invalid<seconds_t>() : std::stoi(row[3]);
        si.departure = row[5].empty() ? invalid<seconds_t>() : std::stoi(row[5]);

        if (isInvalid(si.arrival) && isInvalid(si.departure)) {
            return;// si past snapshot time, the remaining si's are not relevant anymore
        }

        auto track_raw = row[7];
        if (track_raw.empty()) {
            si.original_track = in_schedule.original_track;
        } else {
            si.original_track = model_.getTrackIdAtNode(node_id, track_raw);
        }

        auto raw_activity = row[8];
        if (raw_activity.empty()) {
            si.original_activity = in_schedule.original_activity;
        } else {
            auto it = activity_table.find(raw_activity);
            si.original_activity = it->second;
        }

        std::vector<ScheduleItem> &vec = instance.realized_schedule[course_id];

        if (vec.size() != si.index) {
            std::cout << "WARN: Realized schedule is in wrong order or incomplete: " << course << "\n";
        }
        vec.push_back(si);
    }


}// namespace fb
