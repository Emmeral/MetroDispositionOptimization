
#pragma once

#include "model/definitions.h"
#include "model/enums/Enums.h"
#include <array>
#include <iostream>
#include <limits>
#include <string>
#include <vector>


namespace fb::input {


    typedef std::string id_t;

    struct DutyStartEnd {
        id_t duty_id;
        uint32_t start_time_seconds;
        id_t start_node;
        uint32_t end_time_seconds;
        id_t end_node;
    };

    struct BaseStationValue {
        id_t node_id;
        Direction direction;
        std::string timeband;
        uint32_t start_timeband_seconds;
        uint32_t end_timeband_seconds;
        uint16_t bsv;
    };

    struct Headway {
        id_t link_start_node;
        id_t link_end_node;
        Activity start_activity_train_front;
        Activity end_activity_train_front;
        Activity start_activity_train_behind;
        Activity end_activity_train_behind;
        uint32_t minimum_headway_seconds;
    };

    struct Link {
        id_t start_node;
        id_t end_node;
        Direction direction;
        uint32_t distance_meters;
    };

    struct MinimumRuntime {
        id_t link_start_node;
        id_t link_end_node;
        Activity start_activity;
        Activity end_activity;
        uint32_t minimum_runtime;
    };

    struct Node {
        std::string name;
        id_t code;
        NodeCategory category;

        std::vector<std::string> eb_tracks;
        std::vector<std::string> wb_tracks;

        double latitude;
        double longitude;

        ShortTurn st_eb;
        ShortTurn st_wb;
    };

    struct TargetFrequency {
        uint32_t start_time_seconds;
        uint32_t end_time_seconds;

        std::string start_time_hhmm;
        std::string end_time_hhmm;

        uint32_t threshold_headway_seconds;
    };

    struct RollingStockDuty {
        id_t duty_id;
        uint16_t seq;
        uint16_t rev_seq;

        uint32_t start_time_seconds;
        uint32_t end_time_seconds;

        std::string start_time_hhmmss;
        std::string end_time_hhmmss;

        id_t start_node;
        id_t end_node;

        EventType event_type;

        id_t train_course_id;
    };

    struct TrainHeader {

        id_t train_course_id;
        Direction direction;
        CourseCategory category;
        uint32_t start_seconds;
        uint32_t end_seconds;

        id_t start_node;
        id_t end_node;
    };

    struct ScheduleItem {
        id_t course_id;
        uint16_t seq;
        uint16_t rev_seq;

        id_t node;
        uint32_t arrival_seconds;
        uint32_t departure_seconds;

        std::string arrival_hhmmss;
        std::string departure_hhmmss;

        std::string track;
        Activity activity;
    };

    struct FrequencyMeasure {
        id_t node_id;
        Direction direction;
    };


    struct InputModel {
        std::vector<DutyStartEnd> duty_start_end{};
        std::vector<BaseStationValue> base_station_values{};
        std::vector<Headway> headways{};
        std::vector<Link> links{};
        std::vector<MinimumRuntime> minimum_runtimes{};
        std::vector<Node> nodes{};
        std::vector<TargetFrequency> target_frequencies{};
        std::vector<FrequencyMeasure> frequency_measure_nodes{};
        std::vector<RollingStockDuty> rolling_stock_duties{};
        std::vector<TrainHeader> train_headers{};
        std::vector<ScheduleItem> schedule{};
    };

}// namespace fb::input
