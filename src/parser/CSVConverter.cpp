
#include "CSVConverter.h"
#include "model/input/InputModel.h"
#include "util.h"
#include <sstream>

namespace fb {
    using namespace input;

    DutyStartEnd CSVConverter::parseDutyStartEnd(const CSVRow &row) {

        DutyStartEnd result{};
        result.duty_id = row[0];
        result.start_time_seconds = std::stoi(row[1]);
        result.start_node = row[2];
        result.end_time_seconds = std::stoi(row[3]);
        result.end_node = row[4];

        return result;
    }


    CSVRow CSVConverter::saveDutyStartEnd(const DutyStartEnd &duty) {

        std::vector<std::string> data = {duty.duty_id, std::to_string(duty.start_time_seconds), duty.start_node,
                                         std::to_string(duty.end_time_seconds), duty.end_node};
        return CSVRow(data);
    }

    input::BaseStationValue CSVConverter::parseBaseStationValue(const CSVRow &row) {
        input::BaseStationValue result{};
        result.node_id = row[0];

        auto const it = direction_table.find(row[1]);
        result.direction = it->second;

        result.timeband = row[2];
        result.start_timeband_seconds = std::stoi(row[3]);
        result.end_timeband_seconds = std::stoi(row[4]);
        result.bsv = std::stoi(row[5]);
        return result;
    }

    CSVRow CSVConverter::saveBaseStationValue(const input::BaseStationValue &bss) {
        std::vector<std::string> data = {bss.node_id,
                                         reverseLookup(direction_table, bss.direction),
                                         bss.timeband,
                                         std::to_string(bss.start_timeband_seconds),
                                         std::to_string(bss.end_timeband_seconds),
                                         std::to_string(bss.bsv)};
        return CSVRow(data);
    }

    Headway CSVConverter::parseHeadway(const CSVRow &row) {
        Headway result{};

        result.link_start_node = row[0];
        result.link_end_node = row[1];
        auto it = activity_table.find(row[2]);
        result.start_activity_train_front = it->second;
        it = activity_table.find(row[3]);
        result.end_activity_train_front = it->second;
        it = activity_table.find(row[4]);
        result.start_activity_train_behind = it->second;
        it = activity_table.find(row[5]);
        result.end_activity_train_behind = it->second;

        result.minimum_headway_seconds = std::stoi(row[6]);
        return result;
    }

    CSVRow CSVConverter::saveHeadway(const Headway &hw) {
        std::vector<std::string> data = {hw.link_start_node,
                                         hw.link_end_node,
                                         reverseLookup(activity_table, hw.start_activity_train_front),
                                         reverseLookup(activity_table, hw.end_activity_train_front),
                                         reverseLookup(activity_table, hw.start_activity_train_behind),
                                         reverseLookup(activity_table, hw.end_activity_train_behind),
                                         std::to_string(hw.minimum_headway_seconds)};
        return CSVRow(data);
    }


    input::Link CSVConverter::parseLink(const CSVRow &row) {
        input::Link result{};

        result.start_node = row[0];
        result.end_node = row[1];
        result.direction = direction_table.find(row[2])->second;
        result.distance_meters = std::stoi(row[3]);

        return result;
    }

    CSVRow CSVConverter::saveLink(const input::Link &link) {
        std::vector<std::string> data = {
                link.start_node,
                link.end_node,
                reverseLookup(direction_table, link.direction),
                std::to_string(link.distance_meters),
        };
        return CSVRow(data);
    }

    MinimumRuntime CSVConverter::parseMinimumRuntime(const CSVRow &row) {
        MinimumRuntime result{};

        result.link_start_node = row[0];
        result.link_end_node = row[1];
        result.start_activity = activity_table.find(row[2])->second;
        result.end_activity = activity_table.find(row[3])->second;
        result.minimum_runtime = std::stoi(row[4]);
        return result;
    }

    CSVRow CSVConverter::saveMinumumRuntime(const MinimumRuntime &mrt) {
        std::vector<std::string> data = {
                mrt.link_start_node, mrt.link_end_node, reverseLookup(activity_table, mrt.start_activity),
                reverseLookup(activity_table, mrt.end_activity), std::to_string(mrt.minimum_runtime)};
        return CSVRow(data);
    }

    input::Node CSVConverter::parseNode(const CSVRow &row) {
        input::Node result{};

        result.name = row[0];
        result.code = row[1];
        result.category = node_category_table.find(row[2])->second;

        // tracks is again a comma seperated list of string
        CSVRow eb_tracks{};
        eb_tracks.readRow(row[3]);
        result.eb_tracks = eb_tracks.data();

        CSVRow wb_tracks{};
        wb_tracks.readRow(row[4]);
        result.wb_tracks = wb_tracks.data();

        result.latitude = std::stod(row[5]);
        result.longitude = std::stod(row[6]);

        result.st_eb = short_turn_table.find(row[7])->second;
        result.st_wb = short_turn_table.find(row[8])->second;

        return result;
    }

    CSVRow CSVConverter::saveNode(const input::Node &node) {

        std::stringstream eb_tracks;
        eb_tracks << CSVRow(node.eb_tracks);

        std::stringstream wb_tracks;
        wb_tracks << CSVRow(node.wb_tracks);

        std::vector<std::string> data = {node.name,
                                         node.code,
                                         reverseLookup(node_category_table, node.category),
                                         eb_tracks.str(),
                                         wb_tracks.str(),
                                         std::to_string(node.latitude),
                                         std::to_string(node.longitude),
                                         reverseLookup(short_turn_table, node.st_eb),
                                         reverseLookup(short_turn_table, node.st_wb)};
        return CSVRow(data);
    }

    TargetFrequency CSVConverter::parseTargetFrequency(const CSVRow &row) {
        TargetFrequency result{};
        result.start_time_seconds = std::stoi(row[0]);
        result.end_time_seconds = std::stoi(row[1]);
        result.start_time_hhmm = row[2];
        result.end_time_hhmm = row[3];
        result.threshold_headway_seconds = std::stoi(row[4]);

        return result;
    }

    CSVRow CSVConverter::saveTargetFrequency(const TargetFrequency &trgf) {
        std::vector<std::string> data = {std::to_string(trgf.start_time_seconds), std::to_string(trgf.end_time_seconds),
                                         trgf.start_time_hhmm, trgf.end_time_hhmm,
                                         std::to_string(trgf.threshold_headway_seconds)};
        return CSVRow(data);
    }

    RollingStockDuty CSVConverter::parseRollingStockDuty(const CSVRow &row) {
        RollingStockDuty result{};

        result.duty_id = row[0];
        result.seq = std::stoi(row[1]);
        result.rev_seq = std::stoi(row[2]);
        // not using comp idx field -> skipping row[3]
        result.start_time_seconds = std::stoi(row[4]);
        result.start_time_hhmmss = row[5];
        result.end_time_seconds = std::stoi(row[6]);
        result.end_time_hhmmss = row[7];

        result.start_node = row[8];
        result.end_node = row[9];
        result.event_type = rolling_stock_event_type_table.find(row[10])->second;
        result.train_course_id = row[11];

        return result;
    }

    CSVRow CSVConverter::saveRollingStockDuty(const RollingStockDuty &rsd) {
        std::vector<std::string> data = {
                rsd.duty_id,
                std::to_string(rsd.seq),
                std::to_string(rsd.rev_seq),
                std::to_string(rsd.seq - 1),
                std::to_string(rsd.start_time_seconds),
                rsd.start_time_hhmmss,
                std::to_string(rsd.end_time_seconds),
                rsd.end_time_hhmmss,
                rsd.start_node,
                rsd.end_node,
                reverseLookup(rolling_stock_event_type_table, rsd.event_type),
                rsd.train_course_id,
        };
        return CSVRow(data);
    }

    TrainHeader CSVConverter::parseTrainHeader(const CSVRow &row) {
        TrainHeader result{};

        result.train_course_id = row[0];
        result.direction = direction_table.find(row[1])->second;
        result.category = course_category_table.find(row[2])->second;
        result.start_seconds = std::stoi(row[3]);
        result.end_seconds = std::stoi(row[4]);
        result.start_node = row[5];
        result.end_node = row[6];

        return result;
    }

    CSVRow CSVConverter::saveTrainHeader(const TrainHeader &th) {
        std::vector<std::string> data = {th.train_course_id,
                                         reverseLookup(direction_table, th.direction),
                                         reverseLookup(course_category_table, th.category),
                                         std::to_string(th.start_seconds),
                                         std::to_string(th.end_seconds),
                                         th.start_node,
                                         th.end_node};
        return CSVRow(data);
    }

    input::FrequencyMeasure CSVConverter::parseFrequencyMeasure(const CSVRow &row) {

        FrequencyMeasure result{};
        result.node_id = row[0];
        result.direction = direction_table.find(row[1])->second;
        return result;
    }

    input::ScheduleItem CSVConverter::parseScheduleItem(const CSVRow &row) {
        input::ScheduleItem result{};

        result.course_id = row[0];
        result.seq = std::stoi(row[1]);
        result.rev_seq = std::stoi(row[2]);
        result.node = row[3];
        result.arrival_seconds = row[4].empty() ? invalid<seconds_t>() : std::stoi(row[4]);
        result.arrival_hhmmss = row[5];
        result.departure_seconds = row[6].empty() ? invalid<seconds_t>() : std::stoi(row[6]);
        result.departure_hhmmss = row[7];
        result.track = row[8];
        result.activity = activity_table.find(row[9])->second;

        return result;
    }


    CSVRow CSVConverter::saveScheduleItem(const input::ScheduleItem &si) {
        std::vector<std::string> data = {si.course_id,
                                         std::to_string(si.seq),
                                         std::to_string(si.rev_seq),
                                         si.node,
                                         std::to_string(si.arrival_seconds),
                                         si.arrival_hhmmss,
                                         std::to_string(si.departure_seconds),
                                         si.departure_hhmmss,
                                         si.track,
                                         reverseLookup(activity_table, si.activity)};
        return CSVRow(data);
    }



}// namespace fb
