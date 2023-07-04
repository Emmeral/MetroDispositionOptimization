
#include "DataParser.h"
#include "CSVConverter.h"
#include "csv/CSVParser.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>

namespace fb {
    using namespace input;


    InputModel DataParser::parse(std::string directory) {


        InputModel model{};
        std::cout << "Parsing directory" << directory << "\n";

        for (const auto &entry: std::filesystem::directory_iterator(directory)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            std::filesystem::path const &path = entry.path();
            if (path.has_extension() && path.extension() == ".csv") {
                std::ifstream fileStream(path);
                auto const rows = CSVParser::parse(fileStream);
                auto const &filename = path.filename().generic_string();
                constructFromCsv(rows, filename, model);
            }
        }

        // if we don't have explicit nodes in the data use the default nodes from the problem
        if (model.frequency_measure_nodes.empty()) {
            model.frequency_measure_nodes.push_back({"PADTLL", Direction::WB});
            model.frequency_measure_nodes.push_back({"WCHAPXR", Direction::EB});
        }

        return model;
    }


    void DataParser::constructFromCsv(const std::vector<CSVRow> &rows, const std::string &filename,
                                      InputModel &input_model) {


        auto name_no_ending = filename.substr(0, filename.size() - 4);// remove .csv
        auto const &without_header = rows | std::views::drop(1);
        if (name_no_ending == "ALL_DUTY_START_END") {
            input_model.duty_start_end = std::vector(rows.size() - 1, DutyStartEnd());
            std::ranges::transform(without_header, input_model.duty_start_end.begin(), CSVConverter::parseDutyStartEnd);
        } else if (name_no_ending == "BASE_STATION_VALUE") {
            input_model.base_station_values = std::vector(rows.size() - 1, BaseStationValue());
            std::ranges::transform(without_header, input_model.base_station_values.begin(),
                                   CSVConverter::parseBaseStationValue);
        } else if (name_no_ending == "HEADWAY") {
            input_model.headways = std::vector(rows.size() - 1, Headway());
            std::ranges::transform(without_header, input_model.headways.begin(), CSVConverter::parseHeadway);
        } else if (name_no_ending == "LINK") {
            input_model.links = std::vector(rows.size() - 1, Link());
            std::ranges::transform(without_header, input_model.links.begin(), CSVConverter::parseLink);
        } else if (name_no_ending == "MINIMUM_RUN_TIME" || name_no_ending == "MINIMUM_RUN_TIME_rev2") {
            input_model.minimum_runtimes = std::vector(rows.size() - 1, MinimumRuntime());
            std::ranges::transform(without_header, input_model.minimum_runtimes.begin(),
                                   CSVConverter::parseMinimumRuntime);
        } else if (name_no_ending == "NODE") {
            input_model.nodes = std::vector(rows.size() - 1, Node());
            std::ranges::transform(without_header, input_model.nodes.begin(), CSVConverter::parseNode);
        } else if (name_no_ending == "PADTLL_WCHAPXR_TARGET_FREQUENCY") {
            input_model.target_frequencies = std::vector(rows.size() - 1, TargetFrequency());
            std::ranges::transform(without_header, input_model.target_frequencies.begin(),
                                   CSVConverter::parseTargetFrequency);
        } else if (name_no_ending == "ROLLING_STOCK_DUTY") {
            input_model.rolling_stock_duties = std::vector(rows.size() - 1, RollingStockDuty());
            std::ranges::transform(without_header, input_model.rolling_stock_duties.begin(),
                                   CSVConverter::parseRollingStockDuty);
        } else if (name_no_ending == "TRAIN_HEADER") {
            input_model.train_headers = std::vector(rows.size() - 1, TrainHeader());
            std::ranges::transform(without_header, input_model.train_headers.begin(), CSVConverter::parseTrainHeader);
        } else if (name_no_ending == "TRAIN_SCHEDULE") {
            input_model.schedule = std::vector(rows.size() - 1, ScheduleItem());
            std::ranges::transform(without_header, input_model.schedule.begin(), CSVConverter::parseScheduleItem);
        } else if (name_no_ending == "FREQUENCY_MEASURE_NODES") {
            input_model.frequency_measure_nodes = std::vector(rows.size() - 1, FrequencyMeasure());
            std::ranges::transform(without_header, input_model.frequency_measure_nodes.begin(),
                                   CSVConverter::parseFrequencyMeasure);
        }
    }


    void DataParser::save(const InputModel &model, const std::string &directory) {


        std::filesystem::create_directory(directory);

        auto header = CSVRow({"DUTY_ID", "START_TIME_SECONDS", "START_NODE", "END_TIME_SECONDS", "END_NODE"});
        saveSingleFile(directory + "/" + "ALL_DUTY_START_END.csv", header, model.duty_start_end,
                       CSVConverter::saveDutyStartEnd);

        header = CSVRow({"NODE", "DIRECTION", "TIMEBAND", "START_TIMEBAND_SECONDS", "END_TIMEBAND_SECONDS", "BSV"});
        saveSingleFile(directory + "/" + "BASE_STATION_VALUE.csv", header, model.base_station_values,
                       CSVConverter::saveBaseStationValue);


        header = CSVRow({"LINK_START_NODE", "LINK_END_NODE", "START_ACTIVITY_TRAIN_FRONT", "END_ACTIVITY_TRAIN_FRONT",
                         "START_ACTIVITY_TRAIN_BEHIND", "END_ACTIVITY_TRAIN_BEHIND", "MINIMUM_HEADWAY_SECONDS"});
        saveSingleFile(directory + "/" + "HEADWAY.csv", header, model.headways, CSVConverter::saveHeadway);


        header = CSVRow({"START_NODE", "END_NODE", "DIRECTION", "DISTANCE_METERS"});
        saveSingleFile(directory + "/" + "LINK.csv", header, model.links, CSVConverter::saveLink);

        header = CSVRow(
                {"LINK_START_NODE", "LINK_END_NODE", "START_ACTIVITY", "END_ACTIVITY", "MINIMUM_RUN_TIME_SECONDS"});
        saveSingleFile(directory + "/" + "MINIMUM_RUN_TIME.csv", header, model.minimum_runtimes,
                       CSVConverter::saveMinumumRuntime);

        header = CSVRow(
                {"NAME", "CODE", "NODE_CATEGORY", "EB_TRACKS", "WB_TRACKS", "LATITUDE", "LONGITUDE", "ST_EB	ST_WB"});
        saveSingleFile(directory + "/" + "NODE.csv", header, model.nodes, CSVConverter::saveNode);


        header = CSVRow({"START_TIME_SECONDS	END_TIME_SECONDS", "START_TIME_HHMM", "END_TIME_HHMM",
                         "THRESHOLD_HEADWAY_SECONDS"});
        saveSingleFile(directory + "/" + "PADTLL_WCHAPXR_TARGET_FREQUENCY.csv", header, model.target_frequencies,
                       CSVConverter::saveTargetFrequency);


        header = CSVRow({"DUTY_ID", "SEQ", "REV_SEQ", "Comp idx", "START_TIME_SECONDS", "START_TIME_HHMMSS",
                         "END_TIME_SECONDS", "END_TIME_HHMMSS", "START_NODE	END_NODE", "EVENT_TYPE",
                         "TRAIN_COURSE_ID"});
        saveSingleFile(directory + "/" + "ROLLING_STOCK_DUTY.csv", header, model.rolling_stock_duties,
                       CSVConverter::saveRollingStockDuty);


        header = CSVRow({"TRAIN_COURSE_ID", "SEQ", "REV_SEQ", "NODE", "ARRIVAL_SECONDS", "ARRIVAL_HHMMSS",
                         "DEPARTURE_SECONDS", "DEPARTURE_HHMMSS", "Track", "Activity"});
        saveSingleFile(directory + "/" + "TRAIN_SCHEDULE.csv", header, model.schedule, CSVConverter::saveScheduleItem);


        header = CSVRow(
                {"TRAIN_COURSE_ID", "DIRECTION", "CATEGORY", "START_SECONDS", "END_SECONDS", "START_NODE", "END_NODE"});
        saveSingleFile(directory + "/" + "TRAIN_HEADER.csv", header, model.train_headers,
                       CSVConverter::saveTrainHeader);
    }


}// namespace fb
